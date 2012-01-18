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
			file.write(chunk, position, function()
			{
				position += chunk.length
				response.resume()
			})
		})
		
		response.on("end", function()
		{
			file.close()
			callback()
		})
	})
	
	request.end()
}

CouchStorage.prototype.create = function(asset)
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
		assert(Math.floor(response.statusCode / 100) == 2)
	})
	
	request.end(JSON.stringify(asset))
}

CouchStorage.prototype.lock = function(id, application)
{
	// first, fetch the latest version of the asset
	var options = {
		method: "GET",
		host: this.host,
		port: this.port,
		path: "/" + this.database + "/" + id,
		agent: false
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
			
			// check that the asset is not already locked
			if (asset.lock === undefined)
			{
				// send the locked version
				asset.lock = {
					user: "MrPlop",
					application: application
				}
				
				var sendOptions = {
					method: "PUT",
					host: this.host,
					port: this.port,
					path: "/" + this.database + "/" + id,
					agent: false,
					headers: {
						"Content-Type": "application/json"
					}
				}
				
				console.log(asset)
				var sendRequest = http.request(sendOptions)
				//sendRequest.end(JSON.stringify(asset), "utf8")
			}
		})
	})
	
	request.end()
}
