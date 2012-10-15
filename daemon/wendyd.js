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
var cradle = require("cradle")

// read config file
fs.readFile(__dirname + "/config.json", function(err, data)
{
	if (err) throw err
	
	var config = JSON.parse(data)
	
	// initialize subsystems
	
	// couch db connection (metadata)
	var metadb = new (cradle.Connection)(config.meta.host, config.meta.port).database(config.meta.database)
	
	// data storage
	var storage = new (require("./storage.js").Storage)(config.storage.host, config.storage.port)
	
	// local file cache
	var cache = new (require("./cache.js").Cache)(config.cache.root)
	
	// management engine
	var engine = new (require("./engine.js").Engine)(metadb, storage, cache)
	
	// local web service
	var service = new (require("./webservice.js").WebService)(engine)
})
