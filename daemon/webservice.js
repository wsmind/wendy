/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2013 Remi Papillie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 *****************************************************************************/

var http = require("http")
var querystring = require("querystring")

// HTTP API declaration
// This structure is used to validate requests before
// attempting any further processing. It does not perform
// advanced checks, such as parameter type validation or
// regex matching.
// All query parameters are considered optional, none
// are enforced as mandatory.
var api = {
	"data": {
		"GET": ["version"],
		"PUT": []
	},
	"list": {
		"GET": []
	},
	"share": {
		"POST": []
	},
	"version": {
		"GET":  []
	},
	"tools": {
		"GET": []
	}
}

function WebService(engine)
{
	this.engine = engine
	
	var server = http.createServer()
	
	var self = this
	server.on("request", function(request, response)
	{
		var url = require("url").parse(request.url, true)
		
		var pathParts = url.pathname.split("/")
		
		// basic API check
		if (!self._validate(pathParts[1], request.method, url.query))
		{
			response.writeHead(418, "I'm a teapot")
			response.end()
			return
		}
		
		// forge method name
		var methodName = "_" + pathParts[1] + "_" + request.method
		
		var resource = "/" + pathParts.slice(2).join("/")
		
		// call it
		self[methodName](resource, url.query, request, response)
	})
	
	server.listen(46288, function()
	{
		console.log("Server listening on port 46288")
	})
}
exports.WebService = WebService

WebService.prototype._validate = function(category, method, parameters)
{
	var allowedMethods = api[category]
	if (allowedMethods === undefined)
		return false // unknown call semantics
	
	var allowedParameters = allowedMethods[method]
	if (allowedParameters === undefined)
		return false // method not allowed
	
	for (var parameterName in parameters)
	{
		if (allowedParameters.indexOf(parameterName) == -1)
			return false // parameter not allowed
	}
	
	// all tests passed
	return true
}

WebService.prototype._data_GET = function(resource, parameters, request, response)
{
	// try to read asset
	this.engine.read(resource, function(err, stream)
	{
		if (err)
		{
			response.writeHead(400, err.message)
			response.end()
		}
		else
		{
			response.writeHead(200, "OK")
			stream.pipe(response)
		}
	})
}

WebService.prototype._data_PUT = function(resource, parameters, request, response)
{
	// try to write asset
	this.engine.save(resource, request, function(err)
	{
		if (err)
		{
			response.writeHead(400, err.message)
			response.end()
		}
		else
		{
			response.writeHead(200, "OK")
			response.end()
		}
	})
}

WebService.prototype._list_GET = function(resource, parameters, request, response)
{
	// enumerate assets with the given filter
	this.engine.list(resource, function(err, pathList)
	{
		if (err)
		{
			response.writeHead(400, err.message)
			response.end()
		}
		else
		{
			response.writeHead(200, "OK")
			response.write(JSON.stringify(pathList))
			response.end()
		}
	})
}

WebService.prototype._share_POST = function(resource, parameters, request, response)
{
	var self = this
	var body = ""
	
	request.on("data", function(chunk)
	{
		body += chunk
	})
	
	request.on("end", function()
	{
		var postData = querystring.decode(body)
		
		// check that the description field is present
		if (postData["description"] === undefined)
		{
			response.writeHead(400, "Required parameter: 'description'")
			response.end()
		}
		else
		{
			// try to share everything local
			self.engine.share(postData["description"], function(err, version)
			{
				if (err) throw err
				
				console.log("version " + version + " successfully shared")
				
				response.writeHead(200, "OK", {"Content-Type": "application/json"})
				response.write(JSON.stringify({version: version}))
				response.end()
			})
		}
	})
}

WebService.prototype._version_GET = function(resource, parameters, request, response)
{
	// try to read version
	this.engine.readVersion(resource.slice(1), function(err, version)
	{
		if (err)
		{
			response.writeHead(400, err.message)
			response.end()
		}
		else
		{
			response.writeHead(200, "OK", {
				"Content-Type": "application/json"
			})
			response.end(JSON.stringify(version))
		}
	})
}

WebService.prototype._tools_GET = function(resource, parameters, request, response)
{
	// rewrite the special /tools URL
	this._data_GET("/.tools" + resource, parameters, request, response)
}

/*Service.prototype.handleClient = function(client)
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
				
				openedFiles[fd].close(function(err)
				{
					if (err) throw err
					
					delete openedFiles[fd]
					self.safeWrite(client, "CLOSED " + fd + "\n")
				})
				
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
	var lastRevisionId = Math.max.apply(Math, Object.keys(asset.revisions))
	var lastRevision = asset.revisions[lastRevisionId]
	
	var rights = ""
	
	// check reading right (e.g if a complete (maybe old) revision is available)
	for (var i in asset.revisions)
	{
		if (asset.revisions[i].state == "cached")
		{
			rights += "r"
			break
		}
	}
	
	// check writing right (last revision downloaded + lock)
	if (asset.lock && (lastRevision.state == "cached"))
	{
		if (asset.lock.user == "MrPlop")
			rights += "w"
	}
	
	this.safeWrite(client, "ASSET " + id + "\n")
	if (asset.lock)
	{
		if (asset.lock.user == "MrPlop")
			this.safeWrite(client, "lockUser <you>\n")
		else
			this.safeWrite(client, "lockUser " + asset.lock.user + "\n")
		this.safeWrite(client, "lockApp " + asset.lock.application + "\n")
	}
	if (rights != "")
		this.safeWrite(client, "rights " + rights + "\n")
	this.safeWrite(client, "revision " + lastRevisionId + "\n")
	this.safeWrite(client, "author " + lastRevision.author + "\n")
	this.safeWrite(client, "date " + lastRevision.date + "\n")
	if (lastRevision.path) this.safeWrite(client, "path " + lastRevision.path + "\n")
	if (lastRevision.type) this.safeWrite(client, "type " + lastRevision.type + "\n")
	if (lastRevision.length) this.safeWrite(client, "length " + lastRevision.length + "\n")
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
}*/
