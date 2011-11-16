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
	this.engine.on("changed", function(asset)
	{
		if (asset.deleted)
		{
			client.write("REMOVED " + asset.id + "\n")
		}
		else
		{
			client.write("UPDATED " + asset.id + "\n")
			client.write("END\n")
		}
	})
	
	// initial dump
	this.engine.dump(function(asset)
	{
		console.log("dumping " + asset)
		client.write("UPDATED " + asset.id + "\n")
		client.write("END\n")
	})
}