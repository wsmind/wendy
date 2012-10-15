/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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

var fs = require("fs")
var path = require("path")
var crypto = require("crypto")
var assert = require("assert")

/**
 * \brief Cache constructor
 * \param path cache root on the filesystem (must be a directory)
 */
function Cache(root)
{
	this.root = root
	
	// create cache directories if they don't exist
	this._recursiveMkdir(path.join(this.root, "cache"))
	this._recursiveMkdir(path.join(this.root, "temp"))
	this._recursiveMkdir(path.join(this.root, "wip"))
	
	// TODO: flush temp directory
}
exports.Cache = Cache

// callback will be called with (id, blobs)
/*Cache.prototype.dump = function(callback)
{
	fs.readdir(this.root, function(err, files)
	{
		if (err)
		{
			console.error("Failed to list files in cache directory (" + this.root + ")")
		}
		else
		{
			// build cross-reference id -> blob list
			var blobs = {}
			for (var i = 0; i < files.length; i++)
			{
				// skip special files
				if ((files[i] == "tmp") || (files[i] == "local.json"))
					continue
				
				var parts = files[i].split("-")
				var id = parts[0]
				var blob = parts[1]
				
				if (id in blobs)
					blobs[id].push(blob)
				else
					blobs[id] = [blob]
			}
			
			callback(blobs)
		}
	})
}*/

// callback(location, filePath) location will be either "cache", "wip", or null (if not found)
Cache.prototype.find = function(hash, callback)
{
	var self = this
	
	// search in the cache first
	var cachePath = path.join(self.root, "cache", hash)
	fs.exists(cachePath, function(exists)
	{
		if (exists)
		{
			callback("cache", cachePath)
		}
		else
		{
			// not found in cache, try wip
			var wipPath = path.join(self.root, "wip", hash)
			fs.exists(wipPath, function(exists)
			{
				if (exists)
				{
					callback("wip", wipPath)
				}
				else
				{
					// not found
					callback(null, null)
				}
			})
		}
	})
}

Cache.prototype.createTemporaryFilename = function()
{
	return path.join(this.root, "temp", Math.floor(Math.random() * 1000000000000000).toString())
}

// destination must be "cache" or "wip"
// callback(err, hash)
Cache.prototype.upgradeTemporary = function(tempFilename, destination, callback)
{
	assert((destination == "cache") || (destination == "wip"))
	
	var self = this
	
	// compute hash (will become the file name)
	this._computeHash(tempFilename, function(err, hash)
	{
		if (err)
		{
			callback(err)
			return
		}
		
		// move the file out of temp/
		fs.rename(tempFilename, path.join(self.root, destination, hash), function(err)
		{
			if (err)
				callback(err, hash)
			else
				callback(null, hash)
		})
	})
}

// source and destination must be "cache" or "wip"
// callback(err)
Cache.prototype.move = function(hash, source, destination, callback)
{
	assert((source == "cache") || (source == "wip"))
	assert((destination == "cache") || (destination == "wip"))
	
	var self = this
	var sourcePath = path.join(self.root, source, hash)
	var destinationPath = path.join(self.root, destination, hash)
	
	fs.exists(sourcePath, function(exists)
	{
		if (!exists)
		{
			callback(new Error("Source file does not exist: '" + sourcePath + "'"))
			return
		}
		
		// move the file
		fs.rename(sourcePath, destinationPath, callback)
	})
}

// callback(err)
Cache.prototype.deleteTemporary = function(tempFilename, callback)
{
	fs.exists(tempFilename, function(exists)
	{
		// do not try to remove temporary file if it was not created
		if (exists)
			fs.unlink(tempFilename, callback)
		else
			callback()
	})
}

// callback(err, object)
Cache.prototype.readLocalMetadata = function(callback)
{
	var metadataPath = path.join(this.root, "local.json")
	fs.exists(metadataPath, function(exists)
	{
		if (exists)
		{
			fs.readFile(metadataPath, function(err, data)
			{
				if (err)
				{
					callback(err, null)
				}
				else
				{
					var metadata = JSON.parse(data)
					callback(null, metadata)
				}
			})
		}
		else
		{
			// no metadata, return an empty object
			callback(null, {})
		}
	})
}

// callback(err)
Cache.prototype.writeLocalMetadata = function(metadata, callback)
{
	var metadataPath = path.join(this.root, "local.json")
	fs.writeFile(metadataPath, JSON.stringify(metadata), function(err)
	{
		callback(err)
	})
}

Cache.prototype._recursiveMkdir = function(directory)
{
	var fragments = path.normalize(directory).replace(/\\/g, "/").split("/")
	
	var currentPath = ""
	for (var i = 0; i < fragments.length; i++)
	{
		currentPath += fragments[i] + "/"
		
		if (!fs.existsSync(currentPath))
			fs.mkdirSync(currentPath, 0777)
	}
}

// callback(err, hash)
Cache.prototype._computeHash = function(filename, callback)
{
	var md5sum = crypto.createHash("md5")
	
	// stream hashing
	var stream = fs.createReadStream(filename)
	stream.on("data", function(chunk)
	{
		md5sum.update(chunk)
	})
	
	// handle errors
	stream.on("error", function(err)
	{
		callback(err)
	})
	
	// hashing done
	stream.on("close", function()
	{
		callback(null, md5sum.digest("hex"))
	})
}