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

// read config file
fs.readFile(__dirname + "/config.json", function(err, data)
{
	if (err) throw err
	
	var config = JSON.parse(data)
	
	// initialize daemon
	var storage = new (require("./storage.js").CouchStorage)(config.storage.host, config.storage.port, config.storage.database)
	var cache = new (require("./cache.js").Cache)(config.cache.root)
	var engine = new (require("./engine.js").Engine)(storage, cache)
	var service = new (require("./service.js").Service)(engine)
})
