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

utils.test("Storage", function()
{
/*	var storage = new (require("../bin/storage.js").Storage)(config.storage.host, config.storage.port)
	
	return {
		"uploading": {
			topic: function() { storage.upload("51a52687284e55046bb7040f9732efff", "plop2.png", this.callback) },
			"should not fail": function(err) { assert.isTrue(!err) }
		}
	}*/
	
	it("should work", function()
	{
		console.log(config.storage.port)
		//assert(false)
	})
})

/*storage.upload("51a52687284e55046bb7040f9732efff", "plop2.png", function(err)
{
	if (err) throw err
	
	console.log("upload complete!")
})

storage.download("29314ecd127f3dbadde3a7172ad1baac", "plop.mpg", function(err)
{
	if (err)
		console.log("error: " + err.message)
	else
		console.log("download complete!")
})*/
