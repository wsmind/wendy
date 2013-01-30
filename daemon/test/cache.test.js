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
var utils = require("./utils.js")

var TEMP_CACHE_DIRECTORY = __dirname + "/fixtures/cache"

var cache = null

describe("cache", function()
{
	before(function(done)
	{
		utils.createCacheFolder(TEMP_CACHE_DIRECTORY, done)
	})
	
	it("initializes", function()
	{
		cache = new (require("../bin/cache.js").Cache)(TEMP_CACHE_DIRECTORY)
	})
	
	describe("metadata", function()
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
	})
	
	describe("content", function()
	{
		it("can be added as temporary", function(done)
		{
			var filename = cache.createTemporaryFilename()
			utils.createTemporary(filename, 1000, function(hash)
			{
				done()
			})
		})
		
		it("can be removed while still temporary", function(done)
		{
			var filename = cache.createTemporaryFilename()
			utils.createTemporary(filename, 1000, function(hash)
			{
				cache.deleteTemporary(filename, done)
			})
		})
		
		it("can be upgraded to cache", function(done)
		{
			var filename = cache.createTemporaryFilename()
			utils.createTemporary(filename, 1000, function(realHash)
			{
				cache.upgradeTemporary(filename, "cache", function(err, hash, size)
				{
					assert(!err)
					assert(hash === realHash)
					assert(size === 1000)
					
					cache.find(hash, function(location, filePath)
					{
						assert(location === "cache")
						assert(filePath !== null)
						done()
					})
				})
			})
		})
		
		it("can be upgraded to wip", function(done)
		{
			var filename = cache.createTemporaryFilename()
			utils.createTemporary(filename, 1000, function(realHash)
			{
				cache.upgradeTemporary(filename, "wip", function(err, hash, size)
				{
					assert(!err)
					assert(hash === realHash)
					assert(size === 1000)
					
					cache.find(hash, function(location, filePath)
					{
						assert(location === "wip")
						assert(filePath !== null)
						done()
					})
				})
			})
		})
		
		it("can be moved between cache to wip", function(done)
		{
			var filename = cache.createTemporaryFilename()
			utils.createTemporary(filename, 1000, function(realHash)
			{
				cache.upgradeTemporary(filename, "cache", function(err, hash, size)
				{
					assert(!err)
					assert(hash === realHash)
					assert(size === 1000)
					
					cache.move(hash, "cache", "wip", function(err)
					{
						assert(!err)
						cache.find(hash, function(location, filePath)
						{
							assert(location === "wip")
							assert(filePath !== null)
							done()
						})
					})
				})
			})
		})
		
		it("cannot be moved the wrong way", function(done)
		{
			var filename = cache.createTemporaryFilename()
			utils.createTemporary(filename, 1000, function(realHash)
			{
				cache.upgradeTemporary(filename, "wip", function(err, hash, size)
				{
					assert(!err)
					assert(hash === realHash)
					assert(size === 1000)
					
					cache.move(hash, "cache", "wip", function(err)
					{
						assert(err)
						cache.find(hash, function(location, filePath)
						{
							// should have stayed in wip
							assert(location === "wip")
							assert(filePath !== null)
							done()
						})
					})
				})
			})
		})
		
		it("is not found when non-existent", function(done)
		{
			cache.find("plop", function(location, filePath)
			{
				assert(location === null)
				assert(filePath === null)
				done()
			})
		})
	})
	
	after(function(done)
	{
		utils.destroyCacheFolder(done)
	})
})
