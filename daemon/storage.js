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
}

CouchStorage.prototype.watchChanges = function(callback)
{
	var options = {
		method: "GET",
		host: this.host,
		port: this.port,
		// TODO: remove deleted docs from changes stream
		path: "/" + this.database + "/_changes?feed=continuous&include_docs=true&since=0"
	}
	
	var request = http.request(options, function(response)
	{
		response.on("data", function(chunk)
		{
			var data = JSON.parse(chunk)
			
			// TODO: remove when ignored at request level
			if (data.deleted === undefined)
			{
				var asset = {
					lock: data.doc.lock,
					revisions: data.doc.revisions
				}
				
				callback(data.doc._id, asset)
			}
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
		response.on("data", function(chunk)
		{
			response.pause()
			file.write(chunk, function()
			{
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

exports.CouchStorage = CouchStorage
