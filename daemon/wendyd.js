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
