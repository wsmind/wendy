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
var fs = require("fs")
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

var engine = null
var firstHash = null
var secondHash = null

describe("engine", function()
{
	before(function(done)
	{
		utils.createDB(DB_HOST, DB_PORT, DB_NAME, function()
		{
			utils.createCacheFolder(TEMP_CACHE_DIRECTORY, function()
			{
				utils.startDataServer(TEMP_DATA_STORAGE_DIRECTORY, TEMP_DATA_STORAGE_PORT, function()
				{
					metadb = new (cradle.Connection)(DB_HOST, DB_PORT).database(DB_NAME)
					storage = new (require("../bin/storage.js").Storage)("localhost", TEMP_DATA_STORAGE_PORT, MAX_PARALLEL_DOWNLOADS, MAX_PARALLEL_UPLOADS)
					cache = new (require("../bin/cache.js").Cache)(TEMP_CACHE_DIRECTORY)
					done()
				})
			})
		})
	})
	
	it("initializes", function()
	{
		engine = new (require("../bin/engine.js").Engine)(metadb, storage, cache)
	})
	
	it("starts", function(done)
	{
		engine.start(done)
	})
	
	it("stops", function(done)
	{
		engine.stop(done)
	})
	
	it("restarts", function(done)
	{
		engine.start(done)
	})
	
	it("lists nothing", function(done)
	{
		engine.list("**", function(err, list)
		{
			assert(!err)
			assert.deepEqual(list, {})
			
			done()
		})
	})
	
	it("saves a new asset", function(done)
	{
		var tempFilename = __dirname + "/fixtures/plop";
		utils.createTemporary(tempFilename, 100, function(hash)
		{
			firstHash = hash
			
			var stream = fs.createReadStream(tempFilename)
			engine.save("/plop", stream, done)
		})
	})
	
	it("references the new asset in the local version", function(done)
	{
		engine.readVersion("local", function(err, version)
		{
			assert(!err)
			assert("/plop" in version.assets)
			assert(version.assets["/plop"].hash == firstHash)
			assert(version.assets["/plop"].size == 100)
			
			done()
		})
	})
	
	it("lists the new asset", function(done)
	{
		engine.list("**", function(err, list)
		{
			assert(!err)
			assert.deepEqual(list, {"/plop": {hash: firstHash, size: 100}})
			
			done()
		})
	})
	
	it("reads back the new asset", function(done)
	{
		var tempFilename = __dirname + "/fixtures/plop.read";
		var fileStream = fs.createWriteStream(tempFilename)
		engine.read("/plop", function(err, stream)
		{
			assert(!err)
			
			stream.pipe(fileStream)
			
			fileStream.on("close", function()
			{
				// check the resulting file
				utils.computeHash(tempFilename, function(hash)
				{
					assert(hash == firstHash)
					done()
				})
			})
		})
	})
	
	it("overwrites the same asset", function(done)
	{
		var tempFilename = __dirname + "/fixtures/plop2";
		utils.createTemporary(tempFilename, 100, function(hash)
		{
			secondHash = hash
			
			var stream = fs.createReadStream(tempFilename)
			engine.save("/plop", stream, done)
		})
	})
	
	it("updates hash in local version", function(done)
	{
		engine.readVersion("local", function(err, version)
		{
			assert(!err)
			assert("/plop" in version.assets)
			assert(version.assets["/plop"].hash == secondHash)
			
			done()
		})
	})
	
	it("reads back the overwritten version", function(done)
	{
		var tempFilename = __dirname + "/fixtures/plop2.read";
		var fileStream = fs.createWriteStream(tempFilename)
		engine.read("/plop", function(err, stream)
		{
			assert(!err)
			
			stream.pipe(fileStream)
			
			fileStream.on("close", function()
			{
				// check the resulting file
				utils.computeHash(tempFilename, function(hash)
				{
					assert(hash == secondHash)
					done()
				})
			})
		})
	})
	
	it("shares the pending changes", function(done)
	{
		engine.share("always comment your changes before sharing", function(err, sharedVersion)
		{
			assert(!err)
			
			// check the resulting versions
			engine.readVersion("local", function(err, localVersion)
			{
				assert(!err)
				assert.deepEqual(localVersion.assets, {})
				
				engine.readVersion(sharedVersion, function(err, version)
				{
					assert(!err)
					assert.deepEqual(version, {
						description: "always comment your changes before sharing",
						author: "Mr Plop",
						assets: {
							"/plop": {
								hash: secondHash,
								size: 100
							}
						}
					})
					
					done()
				})
			})
		})
	})
	
	it("lists the asset from shared version", function(done)
	{
		engine.list("**", function(err, list)
		{
			assert(!err)
			assert.deepEqual(list, {"/plop": {hash: secondHash, size: 100}})
			
			done()
		})
	})
	
	it("stops", function(done)
	{
		engine.stop(done)
	})
	
	/*it("restarts", function(done)
	{
		engine.start(done)
	})
	
	it("lists the asset after restart", function(done)
	{
		engine.list("**", function(err, list)
		{
			assert(!err)
			assert.deepEqual(list, {"/plop": {hash: secondHash, size: 100}})
			
			done()
		})
	})*/
	
	after(function(done)
	{
		utils.destroyDB(function()
		{
			utils.destroyCacheFolder(function()
			{
				utils.stopDataServer(done)
			})
		})
	})
})
