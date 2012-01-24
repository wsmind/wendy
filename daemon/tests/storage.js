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

var storage = new (require("../storage.js").CouchStorage)("localhost", 5984, "plop")
var fs = require("fs")

/*storage.create({
	revisions: {
		"0": {
			author: "Mr Blob",
			date: 42,
		}
	}
})*/

storage.watchChanges(function(id, asset)
{
	/*for (var i in asset.revisions)
	{
		if (!asset.revisions[i].blob)
			continue
		
		console.log("downloading rev " + i)
		
		fs.open("cache/" + id + "-" + i, "w", 0666, function(err, fd)
		{
			var file = {
				write: function(buffer, callback)
				{
					fs.write(fd, buffer, 0, buffer.length, null, function(err, written, buffer)
					{
						callback(err, written, buffer)
					})
				},
				close: function()
				{
				}
			}
			
			storage.download(id, asset.revisions[i].blob, file, function()
			{
				console.log(asset.revisions[i].path + " downloaded!")
			})
		})
	}*/
	
	console.log("locking " + id)
	
	storage.lock(id, "fakeApplication")
})
