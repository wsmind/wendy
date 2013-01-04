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
var cp = require("child_process")
var fs = require("fs")
var rimraf = require("rimraf")
var utils = require("./utils.js")

var TEMP_DATA_STORAGE_DIRECTORY = __dirname + "/fixtures/temp-data-storage"
var TEMP_DATA_STORAGE_PORT = 1234

var storage = null
var serverProcess = null

before(function(done)
{
	fs.mkdir(TEMP_DATA_STORAGE_DIRECTORY, function(err)
	{
		serverProcess = cp.fork(__dirname + "/../../data-server/server.js", [TEMP_DATA_STORAGE_PORT, TEMP_DATA_STORAGE_DIRECTORY], {
			silent: true
		})
		
		serverProcess.on("exit", function(code, signal)
		{
			if (!code)
				throw new Error("Data server crashed!")
		})
		
		utils.createTemporary(__dirname + "/fixtures/small", 100, function(hash)
		{
			console.log("small hash: " + hash)
			done()
		})
	})
})

describe("storage", function()
{
	it("initializes", function()
	{
		storage = new (require("../bin/storage.js").Storage)("localhost", TEMP_DATA_STORAGE_PORT)
	})
	
	it("uploads correctly", function(done)
	{
		storage.upload("small", __dirname + "/fixtures/small", done)
	})
	
	it("downloads correctly", function(done)
	{
		storage.download("small", __dirname + "/fixtures/small.downloaded", done)
		// TODO: check md5 hash
	})
	
	it("fails to upload non-existent file", function(done)
	{
		storage.upload("non_existent_file_name", "non_existent_file_name", function(err)
		{
			assert(err)
			done()
		})
	})
	
	it("fails to download non-existent file", function(done)
	{
		storage.download("non_existent_file_name", "non_existent_file_name", function(err)
		{
			assert(err)
			done()
		})
	})
	
	it("fails to erase a local directory", function(done)
	{
		storage.download("small", __dirname + "/fixtures", function(err)
		{
			assert(err)
			done()
		})
	})
})

after(function(done)
{
	serverProcess.kill()
	
	// remove test cache folder
	rimraf(TEMP_DATA_STORAGE_DIRECTORY, function(err)
	{
		assert(!err)
		done()
	})
})
