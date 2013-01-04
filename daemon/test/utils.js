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

var fs = require("fs")
var crypto = require("crypto")
var rimraf = require("rimraf")

// callback(hash)
// throws on error
exports.createTemporary = function(filename, size, callback)
{
	fs.open(filename, "w", function(err, fd)
	{
		if (err) throw err
		
		var md5sum = crypto.createHash("md5")
		
		// write some random stuff (and compute hash at the same time)
		for (var i = 0; i < size; i++)
		{
			var randomByte = new Buffer([Math.floor(Math.random() * 256)])
			fs.writeSync(fd, randomByte, 0, 1, i)
			md5sum.update(randomByte)
		}
		
		callback(md5sum.digest("hex"))
	})
}

/*exports.test = function(name, callback)
{
	before(function(done)
	{
		// read config file
		fs.readFile(__dirname + "/fixtures/test-config.json", function(err, data)
		{
			if (err) throw err
			
			config = JSON.parse(data)
			
			done()
		})
	})
	
	describe(name, callback)
}*/

/*console.log(">> Test DB: http://" + config.meta.host + ":" + config.meta.port + "/" + config.meta.database)
console.log(">> Test storage: http://" + config.storage.host + ":" + config.storage.port)
console.log(">> Test cache root: " + config.cache.root)

// clean test environment
destroyEnvironment(config, function()
{
	// recreate test environment
	createEnvironment(config, function()
	{
		// start test
		var child = child_process.fork(path.join(testFolder, testFile))
		child.send(config)
		
		// wait for termination
		child.on("exit", function(code, signal)
		{
			// output result
			console.log(testFile + " exited with code " + code)
			
			// start next test
			runNextTest(tests)
		})
	})
})*/

function createEnvironment(config, callback)
{
	// create test DB
	var options = {
		method: "PUT",
		host: config.meta.host,
		port: config.meta.port,
		path: "/" + config.meta.database,
	}
	
	var request = http.request(options, function(response)
	{
		// the test database should not exist when we try to create it
		assert(response.statusCode == 201)
		
		// create test cache folder
		fs.mkdir(config.cache.root, 0777, function(err)
		{
			assert(!err)
			callback()
		})
	})
	
	request.end()
}

function destroyEnvironment(config, callback)
{
	// destroy test DB
	var options = {
		method: "DELETE",
		host: config.meta.host,
		port: config.meta.port,
		path: "/" + config.meta.database,
	}
	
	var request = http.request(options, function(response)
	{
		// remove test cache folder
		rimraf(config.cache.root, function(err)
		{
			assert(!err)
			callback()
		})
	})
	
	request.end()
}
