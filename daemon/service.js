/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

var net = require("net")

function Service(engine)
{
	this.engine = engine
	
	var server = net.createServer()
	
	var self = this
	server.on("connection", function(client)
	{
		console.log("Client connected!")
		self.handleClient(client)
	})
	
	server.listen(46288, function()
	{
		console.log("Server listening on port 46288")
	})
}
exports.Service = Service

Service.prototype.handleClient = function(client)
{
	var self = this
	
	console.log("client connected")
	
	var assetCallback = function(id, asset)
	{
		self.sendAsset(client, id, asset)
	}
	
	// engine notifications
	this.engine.on("changed", assetCallback)
	
	// disconnection
	client.on("close", function()
	{
		console.log("client disconnected")
		
		// TODO: close all remaining AssetFiles
		
		self.engine.removeListener("changed", assetCallback)
	})
	
	// client actions
	var reader = new StreamReader(client)
	var openedFiles = {} // fd -> cache.AssetFile
	this.processAction(client, reader, openedFiles)
	
	// initial dump
	this.engine.dump(assetCallback)
}

Service.prototype.processAction = function(client, reader, openedFiles)
{
	var self = this
	reader.readLine(function(line)
	{
		var spacePosition = line.indexOf(" ")
		var command = line.slice(0, spacePosition)
		var parameters = line.slice(spacePosition + 1)
		
		switch (command)
		{
			case "CREATE":
			{
				var path = parameters
				self.engine.create(path)
				break
			}
			
			case "LOCK":
			{
				var parts = parameters.split(" ")
				var id = parts[0]
				var application = parts[1]
				console.log("LOCK!! -> " + id + ", " + application)
				
				self.engine.lock(id, application);
				
				break
			}
			
			case "UNLOCK":
			{
				var id = parameters
				console.log("UNLOCK!! -> " + id)
				
				self.engine.unlock(id);
				
				break
			}
			
			case "OPEN":
			{
				var parts = parameters.split(" ")
				var id = parts[0]
				var mode = parts[1]
				console.log("OPEN!! -> " + id + ", " + mode)
				
				var engineMode = null
				if (mode == "READING") engineMode = "r"
				else if (mode == "WRITING") engineMode = "w"
				
				self.engine.open(id, engineMode, function(err, file)
				{
					if (err) throw err
					
					// find a free file descriptor number
					var fd = 0
					while (openedFiles[fd] != undefined)
						fd++;
					
					openedFiles[fd] = file
					
					self.safeWrite(client, "OPENED " + id + " " + mode + " " + fd + "\n")
				})
				
				break;
			}
			
			case "READ":
			{
				var parts = parameters.split(" ")
				var fd = parseInt(parts[0])
				var offset = parseInt(parts[1])
				var size = parseInt(parts[2])
				
				console.log("READ!! -> " + fd + ", " + offset + ", " + size)
				
				if (openedFiles[fd] === undefined)
				{
					console.error("File " + fd + " was read but not opened!")
					break;
				}
				
				openedFiles[fd].read(new Buffer(size), offset, function(err, bytesRead, buffer)
				{
					if (err) throw err
					
					// TODO: handle bytesRead < size
					
					self.sendChunk(client, fd, offset, buffer)
				})
				
				break;
			}
			
			case "CLOSE":
			{
				var fd = parameters
				
				console.log("CLOSE!! -> " + fd)
				
				if (openedFiles[fd] === undefined)
				{
					console.error("File " + fd + " was closed but not opened!")
					break;
				}
				
				openedFiles[fd].close()
				delete openedFiles[fd]
				
				self.safeWrite(client, "CLOSED " + fd + "\n")
				
				break
			}
		}
		
		// process next action
		self.processAction(client, reader, openedFiles)
	})
}

Service.prototype.sendAsset = function(client, id, asset)
{
	console.log("sending " + id)
	
	// find last revision number
	var lastRevision = Math.max.apply(Math, Object.keys(asset.revisions))
	var revision = asset.revisions[lastRevision]
	
	this.safeWrite(client, "ASSET " + id + "\n")
	if (asset.lock)
	{
		this.safeWrite(client, "lockUser " + asset.lock.user + "\n")
		this.safeWrite(client, "lockApp " + asset.lock.application + "\n")
	}
	this.safeWrite(client, "revision " + lastRevision + "\n")
	this.safeWrite(client, "author " + revision.author + "\n")
	this.safeWrite(client, "date " + revision.date + "\n")
	if (revision.path) this.safeWrite(client, "path " + revision.path + "\n")
	if (revision.type) this.safeWrite(client, "type " + revision.type + "\n")
	if (revision.length) this.safeWrite(client, "length " + revision.length + "\n")
	this.safeWrite(client, "END\n")
}

Service.prototype.sendChunk = function(client, fd, offset, chunk)
{
	this.safeWrite(client, "CHUNK " + fd + " " + offset + " " + chunk.length + "\n")
	this.safeWrite(client, chunk)
}

Service.prototype.safeWrite = function(client, data)
{
	try
	{
		client.write(data)
	}
	catch (err)
	{
		// can happen if the client disconnects *while* writing a bunch of data
		console.log("ERRORRRRR!!!\n")
	}
}

// helper for splitting the stream into lines and binary chunks
function StreamReader(stream)
{
	this.stream = stream
	this.readingBuffer = new Buffer(0)
	this.currentObject = null // might be "line" or "chunk"
	this.chunkSize = 0
	this.callback = null
	
	var self = this
	this.stream.on("data", function(buffer)
	{
		self.readingBuffer += buffer
		self._checkContent()
	})
}
exports.StreamReader = StreamReader // exported for testing purposes

StreamReader.prototype.readLine = function(callback)
{
	this.currentObject = "line"
	this.callback = callback
	this._checkContent()
}

StreamReader.prototype.readChunk = function(size, callback)
{
	this.currentObject = "chunk"
	this.chunkSize = size
	this.callback = callback
	this._checkContent()
}

StreamReader.prototype._checkContent = function()
{
	switch (this.currentObject)
	{
		case "line":
		{
			// scan for a newline in the accumulated buffer
			for (var i = 0; i < this.readingBuffer.length; i++)
			{
				if (this.readingBuffer[i] == '\n')
				{
					// extract line
					var line = this.readingBuffer.slice(0, i).toString("utf8")
					
					// shrink reading buffer keeping only unread content
					this.readingBuffer = this.readingBuffer.slice(i + 1)
					
					// save callback
					var callback = this.callback
					
					// reset state
					this.currentObject = null
					this.callback = null
					
					// signal the line
					callback(line)
					
					return
				}
			}
			
			break;
		}
		case "chunk":
		{
			// check if enough data were read
			if (this.readingBuffer.length >= this.chunkSize)
			{
				// extract chunk
				var chunk = this.readingBuffer.slice(0, this.chunkSize)
				
				// shrink reading buffer keeping only unused content
				this.readingBuffer = this.readingBuffer.slice(this.chunkSize)
				
				// save callback
				var callback = this.callback
				
				// reset state
				this.currentObject = null
				this.chunkSize = 0
				this.callback = null
				
				// signal the chunk
				callback(chunk)
				
				return
			}
			
			break;
		}
	}
}
