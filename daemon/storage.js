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

var fs = require("fs")
var http = require("http")

function Storage(host, port, database)
{
	this.host = host
	this.port = port
}
exports.Storage = Storage

Storage.prototype.download = function(hash, filename, callback)
{
	var options = {
		method: "GET",
		host: this.host,
		port: this.port,
		path: "/" + hash
	}
	
	function signalResult(err)
	{
		if (callback)
			callback(err)
		callback = null
	}
	
	var request = http.request(options, function(response)
	{
		if (response.statusCode != 200)
		{
			signalResult(new Error("Wrong answer from server: " + response.statusCode))
			return;
		}
		
		response.on("error", function(err)
		{
			signalResult(err)
		})
		
		response.on("close", function()
		{
			// if the stream is still readable, the connection was abnormaly closed
			if (response.readable)
				signalResult(new Error("Connection lost"))
		})
		
		var fileStream = fs.createWriteStream(filename)
		
		fileStream.on("error", function(err)
		{
			// error during transfer
			signalResult(err)
		})
		
		fileStream.on("close", function()
		{
			// successful termination (if nothing else failed before)
			signalResult()
		})
		
		response.pipe(fileStream)
	})
	
	// handle errors during connection
	request.on("error", function(err)
	{
		// error during transfer
		signalResult(err)
	})
	
	request.end()
}

Storage.prototype.upload = function(hash, filename, callback)
{
	var options = {
		method: "PUT",
		host: this.host,
		port: this.port,
		path: "/" + hash
	}
	
	var request = http.request(options, function(response)
	{
		if (response.statusCode != 200)
		{
			callback(new Error("Wrong answer from server: " + response.statusCode))
			return;
		}
		
		// successful upload
		callback()
	})
	
	// stream asset file
	var fileStream = fs.createReadStream(filename)
	fileStream.pipe(request)
	
	request.on("error", function(err)
	{
		// upload failed
		callback(err)
	})
}
