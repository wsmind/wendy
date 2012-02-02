/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

var cache = new (require("../cache.js").Cache)("cache")

cache.open("plop", 42, "w", function(file)
{
	file.write(new Buffer("yeahha"), 0, function(err, written, buffer)
	{
		file.close()
		
		// read back what was written
		cache.open("plop", 42, "r", function(file2)
		{
			var buf = new Buffer(500)
			file2.read(buf, 0, function(err, bytesRead, buffer)
			{
				console.log("read " + bytesRead + " bytes:")
				console.log(buf.slice(0, bytesRead).toString("utf8"))
				
				file2.close()
			})
		})
	})
})

cache.open(new Buffer("plop"), 47, "w", function(file)
{
	file.write(new Buffer("yopyop"), 0, function(err, written, buffer)
	{
		file.close()
	})
})

cache.open(new Buffer("noplop"), 12, "w", function(file)
{
	file.write(new Buffer("wendy rules :p"), 0, function(err, written, buffer)
	{
		file.close()
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
