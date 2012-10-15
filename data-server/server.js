/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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
