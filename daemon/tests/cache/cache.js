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

var cache = new (require("../../cache.js").Cache)("d:/wendy-cache")
var fs = require("fs")

var filename = cache.createTemporaryFilename()

console.log("tmp filename: " + filename)
fs.open(filename, "w", 0666, function(err, fd)
{
	if (err) throw err
	
	fs.write(fd, new Buffer("plop"), 0, 4, 0, function(err, written, buffer)
	{
		if (err) throw err
		
		fs.close(fd, function(err)
		{
			if (err) throw err
			console.log("file written!")
			
			cache.upgradeTemporary(filename, "cache", function(err, hash)
			{
				if (err) throw err
				console.log("cached as " + hash)
			})
			
			/*cache.deleteTemporary(filename, function(err)
			{
				if (err) throw err
			})*/
		})
	})
})

cache.find("plop", function(location)
{
	console.log("plop found in: " + location)
})

/*process.once("message", function(config)
{
	var cache = new (require("../../cache.js").Cache)(config.cache.root)
	
	cache.open("plop", 42, "w", function(file)
	{
		file.write(new Buffer("yeahha"), 0, function(err, written, buffer)
		{
			if (err) throw err
			
			file.close(function(err)
			{
				if (err) throw err
				
				// read back what was written
				cache.open("plop", 42, "r", function(file2)
				{
					var buf = new Buffer(500)
					file2.read(buf, 0, function(err, bytesRead, buffer)
					{
						if (err) throw err
						
						console.log("read " + bytesRead + " bytes:")
						console.log(buf.slice(0, bytesRead).toString("utf8"))
						
						file2.close(function(err)
						{
							if (err) throw err
						})
					})
				})
			})
		})
	})
	
	cache.open(new Buffer("plop"), 47, "w", function(file)
	{
		file.write(new Buffer("yopyop"), 0, function(err, written, buffer)
		{
			file.close(function(err)
			{
			})
		})
	})
	
	cache.open(new Buffer("noplop"), 12, "w", function(file)
	{
		file.write(new Buffer("wendy rules :p"), 0, function(err, written, buffer)
		{
			file.close(function(err)
			{
			})
		})
	})
	
	cache.dump(function(blobs)
	{
		for (var id in blobs)
			console.log("blobs for " + id + ": " + blobs[id])
	})
	
	cache.readLocalMetadata(function(err, metadata)
	{
		console.log("current local metadata: " + JSON.stringify(metadata))
		
		metadata = {
			"12": {author: "swingy", date: 100, tag: "you're it"},
			"25": {path: "this-is-25.txt"}
		}
		cache.writeLocalMetadata(metadata, function(err)
		{
			if (err) throw err
			
			cache.readLocalMetadata(function(err, metadata)
			{
				console.log("new metadata: " + JSON.stringify(metadata))
			})
		})
	})
})*/
