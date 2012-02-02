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

var fs = require("fs")
var path = require("path")
var assert = require("assert")

/**
 * \brief Cache constructor
 * \param path cache root on the filesystem (must be a directory)
 */
function Cache(root)
{
	this.root = root
	
	// create root cache directory (+tmp) if it doesn't exist
	this._recursiveMkdir(this.root)
	this._recursiveMkdir(path.join(this.root, "tmp"))
	
	// TODO: flush tmp directory
}
exports.Cache = Cache

// callback will be called with (id, blobs)
Cache.prototype.dump = function(callback)
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
}

// mode must be "r" or "w"
Cache.prototype.open = function(id, blob, mode, callback)
{
	assert((mode == "r") || (mode == "w"))
	
	var assetFolder = this.root
	
	// always write to temporary folder
	if (mode == "w")
		assetFolder = path.join(assetFolder, "tmp")
	
	var self = this
	var assetPath = path.join(assetFolder, id + "-" + blob)
	fs.open(assetPath, mode, 0666, function(err, fd)
	{
		// TODO: handle error
		if (err) throw err
		
		// path where to move the asset upon close
		var targetPath = null
		if (mode == "w")
		{
			targetPath = path.join(self.root, id + "-" + blob)
			
			// if the target file already exists, delete it
			if (path.existsSync(targetPath))
				fs.unlinkSync(targetPath)
		}
		
		var file = new AssetFile(fd, assetPath, targetPath)
		callback(file)
	})
}

// callback(err, object)
Cache.prototype.readLocalMetadata = function(callback)
{
	var metadataPath = path.join(this.root, "local.json")
	path.exists(metadataPath, function(exists)
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
		
		if (!path.existsSync(currentPath))
			fs.mkdirSync(currentPath, 0777)
	}
}

function AssetFile(fd, path, targetPath)
{
	this.fd = fd
	this.path = path
	this.targetPath = targetPath
}

AssetFile.prototype.stat = function(callback)
{
	fs.fstat(this.fd, function(err, stats)
	{
		if (err) throw err
		
		callback(err, {
			size: stats.size
		})
	})
}

AssetFile.prototype.read = function(buffer, position, callback)
{
	fs.read(this.fd, buffer, 0, buffer.length, position, function(err, bytesRead, buffer)
	{
		// TODO: handle error
		if (err) throw err
		
		callback(err, bytesRead, buffer)
	})
}

AssetFile.prototype.write = function(buffer, position, callback)
{
	fs.write(this.fd, buffer, 0, buffer.length, position, function(err, written, buffer)
	{
		// TODO: handle error
		if (err) throw err
		
		callback(err, written, buffer)
	})
}

AssetFile.prototype.close = function()
{
	fs.closeSync(this.fd)
	
	// move the asset to another location
	if (this.targetPath)
		fs.renameSync(this.path, this.targetPath)
}
