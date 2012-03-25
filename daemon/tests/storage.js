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

/*storage.create(function(id)
{
	console.log("created asset with id " + id)
	
	fs.open("downstream.pdf", "r", 0666, function(err, fd)
	{
		var file = {
			stat: function(callback)
			{
				fs.fstat(fd, callback)
			},
			read: function(buffer, position, callback)
			{
				fs.read(fd, buffer, 0, buffer.length, position, callback)
			},
			close: function(callback)
			{
				fs.close(fd, callback)
			}
		}
		
		storage.upload(id, "42.pdf", file, function()
		{
			console.log("downstream.pdf uploaded!")
		})
	})
})*/

storage.watchChanges(function(id, asset)
{
	/*for (var i in asset.revisions)
	{
		if (!asset.revisions[i].blob)
			continue
		
		console.log("downloading rev " + i)
		
		fs.open("cache/" + id + "-" + i + "-" + asset.revisions[i].path, "w", 0666, function(err, fd)
		{
			var file = {
				write: function(buffer, position, callback)
				{
					fs.write(fd, buffer, 0, buffer.length, position, function(err, written, buffer)
					{
						callback(err, written, buffer)
					})
				},
				close: function(callback)
				{
					fs.close(fd, callback)
				}
			}
			
			storage.download(id, asset.revisions[i].blob, file, function()
			{
				console.log(asset.revisions[i].path + " downloaded!")
			})
		})
	}*/
	
	console.log("unlocking " + id)
	
	storage.lock(id, "fakeApplication")
	//storage.unlock(id)
	
	/*var rev = asset.revisions["1"]
	
	if (rev && rev.blob)
	{
		fs.open("cache/" + id + "-" + rev.blob, "r", 0666, function(err, fd)
		{
			var file = {
				stat: function(callback)
				{
					fs.fstat(fd, function(err, stats)
					{
						if (err) throw err
						
						callback(err, {
							size: stats.size
						})
					})
				},
				read: function(buffer, position, callback)
				{
					fs.read(fd, buffer, 0, buffer.length, position, function(err, bytesRead, buffer)
					{
						callback(err, bytesRead, buffer)
					})
				},
				close: function(callback)
				{
					fs.close(fd, callback)
				}
			}
			
			storage.upload(id, rev.blob, file, function()
			{
				console.log(rev.path + " uploaded (again)!")
			})
		})
	}*/
})
