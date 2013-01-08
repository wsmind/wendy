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

function WebService(engine, port)
{
	this.engine = engine
	
	var server = http.createServer()
	
	var self = this
	server.on("request", function(request, response)
	{
		var url = require("url").parse(request.url, true)
		
		var pathParts = url.pathname.split("/")
		pathParts.shift() // remove first (blank) part
		
		var category = pathParts.shift()
		var resource = "/" + pathParts.join("/")
		
		// basic API check
		if (!self._validate(category, request.method, url.query))
		{
			response.writeHead(418, "I'm a teapot")
			response.end()
			return
		}
		
		// find the appropriate handler
		var handler = self._api[category][request.method].handler
		
		// call it with the right _this_
		handler.call(self, resource, url.query, request, response)
	})
	
	server.listen(port, function()
	{
		console.log("Server listening on port " + port)
	})
}
exports.WebService = WebService

WebService.prototype._validate = function(category, method, parameters)
{
	var allowedMethods = this._api[category]
	if (allowedMethods === undefined)
		return false // unknown call semantics
	
	var method = allowedMethods[method]
	if (method === undefined)
		return false // method not allowed
	
	for (var parameterName in parameters)
	{
		if (!(parameterName in method.parameters))
			return false // parameter not allowed
	}
	
	// all tests passed
	return true
}

// HTTP API definition
// This structure is used to validate requests before
// attempting any further processing and generate documentation,
// as well as handling requests as appropriate.
// No advanced checks are performed, such as parameter
// type validation or regex matching.
// All query parameters are considered optional, none
// are enforced as mandatory.
WebService.prototype._api = {
	"data": {
		"GET": {
			doc: "Retrieve asset raw content",
			parameters: {
				"/path": "path to asset",
				"version": "id of a specific version to fetch. The latest version available will be retrieved if not specified."
			},
			handler: function(resource, parameters, request, response)
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
		},
		"PUT": {
			doc: "Save new content for asset",
			parameters: {
				"/path": "path to asset"
			},
			handler: function(resource, parameters, request, response)
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
		}
	},
	"list": {
		"GET": {
			doc: "List asset paths",
			parameters: {
				"/pattern": "glob-style expression to select a subset of asset paths"
			},
			handler: function(resource, parameters, request, response)
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
		}
	},
	"share": {
		"POST": {
			doc: "Publish local changes",
			parameters: {},
			handler: function(resource, parameters, request, response)
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
		}
	},
	"version": {
		"GET": {
			doc: "Fetch metadata about a specific version",
			parameters: {
				"/id": "the version id (js time value), or the string \"local\" for local (unshared) changes"
			},
			handler: function(resource, parameters, request, response)
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
		}
	},
	"tools": {
		"GET": {
			doc: "Access web tools",
			parameters: {
				"/name": "name of the tool to run"
			},
			handler: function(resource, parameters, request, response)
			{
				// rewrite the special /tools URL
				this._api.data.GET.handler.call(this, "/.tools" + resource, parameters, request, response)
			}
		}
	},
	"_api": {
		"GET": {
			doc: "This API generated documentation",
			parameters: {},
			handler: function(resource, parameters, request, response)
			{
				response.writeHead(200, "OK", {
					"Content-Type": "text/html"
				})
				
				// basic documentation generation
				response.write("<!DOCTYPE html>\n")
				response.write("<html>\n")
				response.write("<body style=\"font-family: sans-serif\">\n")
				response.write("<h1>Wendy webservice API</h1>\n")
				
				for (var categoryName in this._api)
				{
					var category = this._api[categoryName]
					for (var methodName in category)
					{
						var method = category[methodName]
						response.write("<h2>" + methodName + " /" + categoryName + "</h2>\n")
						response.write("<p>" + method.doc + "</p>\n")
						response.write("<dl>\n")
						for (var parameterName in method.parameters)
						{
							response.write("<dt style=\"font-weight: bold\">" + parameterName + "</dt>\n")
							response.write("<dd>" + method.parameters[parameterName] + "</dd>\n")
						}
						response.write("<dl>\n")
					}
				}
				
				response.write("</body>\n")
				response.write("</html>\n")
				response.end()
			}
		}
	}
}
