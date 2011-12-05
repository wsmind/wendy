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
	var connected = true
	
	console.log("client connected")
	
	var assetCallback = function(id, asset)
	{
		if (connected)
			self.sendAsset(client, id, asset)
	}
	
	// engine notifications
	this.engine.on("changed", assetCallback)
	
	// disconnection
	client.on("close", function()
	{
		console.log("client disconnected")
		
		connected = false
		self.engine.removeListener("changed", assetCallback)
	})
	
	// client actions
	// TODO
	
	// initial dump
	this.engine.dump(assetCallback)
}

Service.prototype.sendAsset = function(client, id, asset)
{
	console.log("sending " + id)
	
	// find last revision number
	var lastRevision = Math.max.apply(Math, Object.keys(asset.revisions))
	var revision = asset.revisions[lastRevision]
	
	if (revision.blob == null)
	{
		client.write("REMOVED " + id + "\n")
	}
	else
	{
		client.write("UPDATED " + id + "\n")
		client.write("path " + revision.path + "\n")
		client.write("author " + revision.author + "\n")
		client.write("END\n")
	}
}
