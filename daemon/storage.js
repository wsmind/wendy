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
