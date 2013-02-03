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
var fs = require("fs")
var utils = require("./utils.js")

var TEMP_CACHE_DIRECTORY = __dirname + "/fixtures/cache"

var cache = null
var plop42Hash = null
var duckHash = null

describe("cache", function()
{
	before(function(done)
	{
		utils.createCacheFolder(TEMP_CACHE_DIRECTORY, done)
	})
	
	it("initializes", function(done)
	{
		cache = new (require("../bin/cache.js").Cache)(TEMP_CACHE_DIRECTORY)
		cache.initialize(done)
	})
	
	/*describe("metadata", function()
	{
		var testMetadata = {
			"12": {author: "swingy", date: 100, tag: "you're it"},
			"25": {path: "this-is-25.txt"}
		}
		
		it("are initially empty", function(done)
		{
			cache.readLocalMetadata(function(err, object)
			{
				assert(!err)
				assert.deepEqual(object, {cache: {}, wip: {}})
				done()
			})
		})
		
		it("saves successfully", function(done)
		{
			cache.writeLocalMetadata(testMetadata, done)
		})
		
		it("reloads successfully", function(done)
		{
			cache.readLocalMetadata(function(err, object)
			{
				assert(!err)
				assert.deepEqual(object, testMetadata)
				done()
			})
		})
	})*/
	
	it("adds content as temporary", function(done)
	{
		var filename = cache.createTemporaryFilename()
		utils.createTemporary(filename, 1000, function(hash)
		{
			fs.unlink(filename, done)
		})
	})
	
	it("removes content while still temporary", function(done)
	{
		var filename = cache.createTemporaryFilename()
		utils.createTemporary(filename, 1000, function(hash)
		{
			cache.deleteTemporary(filename, done)
		})
	})
	
	it("removes content even if not created", function(done)
	{
		var filename = cache.createTemporaryFilename()
		cache.deleteTemporary(filename, done)
	})
	
	it("upgrades content to cache", function(done)
	{
		var filename = cache.createTemporaryFilename()
		utils.createTemporary(filename, 1000, function(realHash)
		{
			plop42Hash = realHash
			cache.upgradeTemporary(filename, "/plop", "42", function(err, hash, size)
			{
				assert(!err)
				assert(hash === realHash)
				assert(size === 1000)
				
				cache.findBlob(hash, function(filePath, size)
				{
					assert(filePath !== null)
					assert(size === 1000)
					
					cache.findAsset("/plop", function(asset)
					{
						assert.deepEqual(asset, {"42": realHash})
						done()
					})
				})
			})
		})
	})
	
	it("accepts the same hash multiple times", function(done)
	{
		var filename1 = cache.createTemporaryFilename()
		var filename2 = cache.createTemporaryFilename()
		utils.createTemporary(filename1, 1000, function(realHash)
		{
			duckHash = realHash
			
			// duplicate the temp file
			var readStream = fs.createReadStream(filename1)
			var writeStream = fs.createWriteStream(filename2)
			readStream.pipe(writeStream)
			
			writeStream.on("close", function()
			{
				// upgrade the first temp file
				cache.upgradeTemporary(filename1, "/first-duck", "1234", function(err, hash, size)
				{
					assert(!err)
					assert(hash === realHash)
					assert(size === 1000)
					
					// upgrade the second temp file (with the same hash)
					cache.upgradeTemporary(filename2, "/second-duck", "5678", function(err, hash, size)
					{
						assert(!err)
						assert(hash === realHash)
						assert(size === 1000)
						
						// check the resulting state
						cache.findBlob(hash, function(filePath, size)
						{
							assert(filePath !== null)
							assert(size === 1000)
							
							cache.findAsset("/first-duck", function(asset1)
							{
								assert.deepEqual(asset1, {"1234": realHash})
								
								cache.findAsset("/second-duck", function(asset2)
								{
									assert.deepEqual(asset2, {"5678": realHash})
									done()
								})
							})
						})
					})
				})
			})
		})
	})
	
	it("accepts new metadata without new content", function(done)
	{
		// bind /first-duck@45 file content to /plop@42
		cache.declareVersion("/first-duck", "45", plop42Hash, done)
	})
	
	it("refuses to bind new metadata to non-existent content", function(done)
	{
		// bind /first-duck@46 to non-existent hash
		cache.declareVersion("/first-duck", "46", "thehashdoesnotexist", function(err)
		{
			assert(err) // should fail
			
			// version should not have been created
			cache.findAsset("/first-duck", function(asset)
			{
				assert(asset["46"] === undefined)
				done()
			})
		})
	})
	
	it("destroys metadata only (blob still used)", function(done)
	{
		// the blob is still used as /second-duck@5678
		cache.deleteVersion("/first-duck", "1234", function(err)
		{
			assert(!err)
			
			// check that /second-duck@5678 is still there
			cache.findBlob(duckHash, function(filePath, size)
			{
				assert(filePath !== null)
				assert(size === 1000)
				
				cache.findAsset("/second-duck", function(asset)
				{
					assert.deepEqual(asset, {"5678": duckHash})
					
					// check that /first-duck@1234 was removed
					cache.findAsset("/first-duck", function(asset)
					{
						assert(asset[1234] === undefined)
						done()
					})
				})
			})
		})
	})
	
	it("destroys blob if no more used", function(done)
	{
		// /second-duck@5678 has a blob for itself
		cache.deleteVersion("/second-duck", "5678", function(err)
		{
			assert(!err)
			
			// check that everything is destroyed
			cache.findBlob(duckHash, function(filePath, size)
			{
				assert(filePath === null)
				assert(size === null)
				
				cache.findAsset("/second-duck", function(asset)
				{
					assert(asset === null)
					done()
				})
			})
		})
	})
	
	it("does not find content when non-existent (invalid hash)", function(done)
	{
		cache.findBlob("thisisnotahash", function(filePath, size)
		{
			assert(filePath === null)
			assert(size === null)
			done()
		})
	})
	
	it("does not find content when non-existent (invalid path)", function(done)
	{
		cache.findAsset("/invalidpath", function(asset)
		{
			assert(asset === null)
			done()
		})
	})
	
	it("does not find content when non-existent (invalid version)", function(done)
	{
		cache.findAsset("/plop", function(asset)
		{
			assert(!("43-is-not-the-answer" in asset))
			done()
		})
	})
	
	it("restarts", function(done)
	{
		cache = new (require("../bin/cache.js").Cache)(TEMP_CACHE_DIRECTORY)
		cache.initialize(done)
	})
	
	it("finds previously cached assets", function(done)
	{
		cache.findBlob(plop42Hash, function(filePath, size)
		{
			assert(filePath !== null)
			assert(size === 1000)
			
			cache.findAsset("/plop", function(asset)
			{
				assert.deepEqual(asset, {"42": plop42Hash})
				done()
			})
		})
	})
	
	after(function(done)
	{
		utils.destroyCacheFolder(done)
	})
})
