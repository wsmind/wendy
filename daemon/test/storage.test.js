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

var TEMP_DATA_STORAGE_DIRECTORY = __dirname + "/fixtures/data-storage"
var TEMP_DATA_STORAGE_PORT = 1234
var MAX_PARALLEL_DOWNLOADS = 4
var MAX_PARALLEL_UPLOADS = 4

var storage = null

describe("storage", function()
{
	before(function(done)
	{
		utils.startDataServer(TEMP_DATA_STORAGE_DIRECTORY, TEMP_DATA_STORAGE_PORT, function()
		{
			utils.createTemporary(__dirname + "/fixtures/small", 100, function(hash)
			{
				done()
			})
		})
	})
	
	it("initializes", function()
	{
		storage = new (require("../bin/storage.js").Storage)("localhost", TEMP_DATA_STORAGE_PORT, MAX_PARALLEL_DOWNLOADS, MAX_PARALLEL_UPLOADS)
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
	
	after(function(done)
	{
		utils.stopDataServer(done)
	})
})
