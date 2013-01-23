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

var assert = require("assert")
var cradle = require("cradle")
var utils = require("./utils.js")

var metadb = null
var DB_HOST = "localhost"
var DB_PORT = 5984
var DB_NAME = "wendy-test"

var storage = null
var TEMP_DATA_STORAGE_DIRECTORY = __dirname + "/fixtures/data-storage"
var TEMP_DATA_STORAGE_PORT = 1234
var MAX_PARALLEL_DOWNLOADS = 4
var MAX_PARALLEL_UPLOADS = 4

var cache = null
var TEMP_CACHE_DIRECTORY = __dirname + "/fixtures/cache"

describe("engine", function()
{
	before(function(done)
	{
		utils.startDataServer(TEMP_DATA_STORAGE_DIRECTORY, TEMP_DATA_STORAGE_PORT, function()
		{
			utils.createCacheFolder(TEMP_CACHE_DIRECTORY, function()
			{
				metadb = new (cradle.Connection)(DB_HOST, DB_PORT).database(DB_NAME)
				storage = new (require("../bin/storage.js").Storage)("localhost", TEMP_DATA_STORAGE_PORT, MAX_PARALLEL_DOWNLOADS, MAX_PARALLEL_UPLOADS)
				cache = new (require("../bin/cache.js").Cache)(TEMP_CACHE_DIRECTORY)
				done()
			})
		})
	})
	
	it("initializes", function()
	{
		//engine = new (require("../bin/engine.js").Engine)(metadb, storage, cache)
	})
	
	after(function(done)
	{
		utils.destroyCacheFolder(function()
		{
			utils.stopDataServer(done)
		})
	})
})
