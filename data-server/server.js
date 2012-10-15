/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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

/**
 * This script creates a basic mapping between a file list
 * and HTTP.
 *
 * Methods supported:
 *   GET /path
 *   PUT /path
 *   DELETE /path
 *
 * GET and DELETE operations on a non-existent path will fail with 404.
 */

if (process.argv.length < 4)
{
	console.log("Usage: node server.js <port> <folder>")
	console.log("Note: The script needs write acces to the given folder")
	process.exit(0)
}

var port = process.argv[2]
var root = process.argv[3]

var http = require("http")
var fs = require("fs")
var url = require("url")
var path = require("path")

var server = http.createServer(function(request, response)
{
	var resourcePath = url.parse(request.url).pathname
	var filePath = path.join(root, resourcePath)
	
	console.log(request.method + " " + resourcePath)
	
	switch (request.method)
	{
		case "GET":
		{
			// check for file existence
			fs.exists(filePath, function(exists)
			{
				if (exists)
				{
					var fileStream = fs.createReadStream(filePath)
					fileStream.pipe(response)
				}
				else
				{
					response.writeHead(404, "Not Found")
					response.end()
				}
			})
			break;
		}
		
		case "PUT":
		{
			var fileStream = fs.createWriteStream(filePath)
			
			fileStream.on("close", function()
			{
				response.writeHead(200, "OK")
				response.end()
			})
			
			fileStream.on("error", function()
			{
				response.writeHead(500, "Internal Server Error")
				response.end()
			})
			
			request.pipe(fileStream)
			
			break;
		}
		
		case "DELETE":
		{
			fs.exists(filePath, function(exists)
			{
				if (exists)
				{
					fs.unlink(filePath, function(err)
					{
						if (err)
						{
							console.log("DELETE failed on '" + filePath + "': " + err)
							response.writeHead(500, "Internal server error")
							response.end()
						}
						else
						{
							response.writeHead(200, "OK")
							response.end()
						}
					})
				}
				else
				{
					response.writeHead(404, "Not Found")
					response.end()
				}
			})
			break;
		}
		
		default:
		{
			response.writeHead(405, "Method not allowed")
			response.end()
		}
	}
})

server.listen(port)
