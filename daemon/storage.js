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

var assert = require("assert")
var fs = require("fs")
var http = require("http")

function CouchStorage(host, port, database)
{
	this.host = host
	this.port = port
	this.database = database
	this.sequence = 0 // passed to 'since' parameter of _changes requests
}
exports.CouchStorage = CouchStorage

CouchStorage.prototype.watchChanges = function(callback)
{
	var options = {
		method: "GET",
		host: this.host,
		port: this.port,
		// TODO: remove deleted docs from changes stream w/ couch filters
		path: "/" + this.database + "/_changes?feed=continuous&include_docs=true&since=" + this.sequence
	}
	
	var self = this
	var request = http.request(options, function(response)
	{
		response.on("data", function(chunk)
		{
			var data
			try
			{
				data = JSON.parse(chunk)
			}
			catch (e)
			{
				// ignore invalid chunks (such as a single new line)
				return
			}
			
			// last_seq is sent by couch before closing the connection
			// we must keep the sequence number to avoid getting more changes
			// than necessary from the next connection.
			if (data.last_seq !== undefined)
			{
				self.sequence = data.last_seq
			}
			else
			{
				// TODO: remove when ignored at request level
				if (data.deleted === undefined)
				{
					var asset = {
						lock: data.doc.lock,
						revisions: data.doc.revisions
					}
					
					// append additional information from blobs in revisions
					for (var i in asset.revisions)
					{
						var revision = asset.revisions[i]
						
						if (revision.blob)
						{
							var attachement = data.doc._attachments[revision.blob]
							
							revision.type = attachement.content_type
							revision.length = attachement.length
						}
					}
					
					callback(data.doc._id, asset)
				}
			}
		})
		
		response.on("end", function()
		{
			// restart change watching
			self.watchChanges(callback)
		})
	})
	
	request.end()
}

CouchStorage.prototype.download = function(id, blob, file, callback)
{
	var options = {
		method: "GET",
		host: this.host,
		port: this.port,
		path: "/" + this.database + "/" + id + "/" + blob
	}
	
	var request = http.request(options, function(response)
	{
		assert(response.statusCode == 200)
		
		// stream data to asset file
		var position = 0
		response.on("data", function(chunk)
		{
			response.pause()
			file.write(chunk, position, function(err, written, buffer)
			{
				if (err) throw err
				if (written < chunk.length) throw new Error("TODO: handle incomplete writes")
				
				position += chunk.length
				response.resume()
			})
		})
		
		response.on("end", function()
		{
			file.close(function(err)
			{
				if (err) throw err
				callback()
			})
		})
	})
	
	request.end()
}

CouchStorage.prototype.upload = function(id, blob, file, callback)
{
	var self = this
	
	// read current revision
	self._readMetadata(id, function(asset)
	{
		// read file size
		file.stat(function(err, stats)
		{
			if (err) throw err
			
			var options = {
				method: "PUT",
				host: self.host,
				port: self.port,
				path: "/" + self.database + "/" + id + "/" + blob + "?rev=" + asset._rev,
				headers: {
					"Content-Length": stats.size
				}
			}
			
			var request = http.request(options, function(response)
			{
				assert(Math.floor(response.statusCode / 100) == 2)
				callback()
			})
			
			// stream asset file to couch
			var position = 0
			var chunk = new Buffer(16 * 1024) // TODO: hardcoding is bad
			function uploadNextChunk()
			{
				file.read(chunk, position, function(err, bytesRead, buffer)
				{
					if (err) throw err
					
					// send this chunk
					position += bytesRead
					var async = !request.write(buffer.slice(0, bytesRead))
					
					// send next chunk (if any)
					if (position < stats.size)
					{
						if (async)
							request.once("drain", uploadNextChunk)
						else
							uploadNextChunk()
					}
					else
					{
						file.close(function(err)
						{
							if (err) throw err
							request.end()
						})
					}
				})
			}
			
			request.on("error", function(exception)
			{
				throw exception
			})
			
			// start chunked upload
			uploadNextChunk()
		})
	})
}

// create an empty asset
// callback(id)
CouchStorage.prototype.create = function(callback)
{
	var options = {
		method: "POST",
		host: this.host,
		port: this.port,
		path: "/" + this.database,
		headers: {
			"Content-Type": "application/json"
		}
	}
	
	var request = http.request(options, function(response)
	{
		//assert(Math.floor(response.statusCode / 100) == 2)
		
		var doc = ""
		response.setEncoding("utf8")
		
		response.on("data", function(chunk)
		{
			doc += chunk
		})
		
		response.on("end", function()
		{
			var asset = JSON.parse(doc)
			callback(asset.id)
		})
		
		response.on("error", function(err)
		{
			throw err
		})
	})
	
	request.on("error", function(err)
	{
		throw err
	})
	
	request.end("{}")
}

CouchStorage.prototype.lock = function(id, application)
{
	// first, fetch the latest version of the asset
	var self = this
	self._readMetadata(id, function(asset)
	{
		// check that the asset is not already locked
		if (asset.lock === undefined)
		{
			// send the locked version
			asset.lock = {
				user: "MrPlop",
				application: application
			}
			
			var options = {
				method: "PUT",
				host: self.host,
				port: self.port,
				path: "/" + self.database + "/" + id,
				headers: {
					"Content-Type": "application/json"
				}
			}
			
			var sendRequest = http.request(options)
			sendRequest.end(JSON.stringify(asset), "utf8")
		}
	})
}

CouchStorage.prototype.unlock = function(id)
{
	// first, fetch the latest version of the asset
	var self = this
	self._readMetadata(id, function(asset)
	{
		// check that the asset is not already unlocked
		if (asset.lock !== undefined)
		{
			// send the unlocked version
			delete asset.lock
			
			var options = {
				method: "PUT",
				host: self.host,
				port: self.port,
				path: "/" + self.database + "/" + id,
				headers: {
					"Content-Type": "application/json"
				}
			}
			
			var sendRequest = http.request(options)
			sendRequest.end(JSON.stringify(asset), "utf8")
		}
	})
}

// callback(metadataObject)
CouchStorage.prototype._readMetadata = function(id, callback)
{
	var options = {
		method: "GET",
		host: this.host,
		port: this.port,
		path: "/" + this.database + "/" + id
	}
	
	var request = http.request(options, function(response)
	{
		assert(Math.floor(response.statusCode / 100) == 2)
		
		var doc = ""
		response.setEncoding("utf8")
		
		response.on("data", function(chunk)
		{
			doc += chunk
		})
		
		response.on("end", function()
		{
			var asset = JSON.parse(doc)
			callback(asset)
		})
		
		response.on("error", function(err)
		{
			throw err
		})
	})
	
	request.end()
}
