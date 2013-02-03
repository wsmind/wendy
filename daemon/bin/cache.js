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
var path = require("path")
var crypto = require("crypto")
var minimatch = require("minimatch")
var assert = require("assert")

var ProcessingQueue = require("../bin/processingqueue").ProcessingQueue

/**
 * \brief Cache constructor
 * \param path cache root on the filesystem (must be a directory)
 */
function Cache(root)
{
	this.root = root
	this.ioQueue = new ProcessingQueue(1)
}
exports.Cache = Cache

// callback(err)
Cache.prototype.initialize = function(callback)
{
	// create cache directories if they don't exist
	this._recursiveMkdir(path.join(this.root, "cache"))
	this._recursiveMkdir(path.join(this.root, "temp"))
	
	// TODO: flush temp directory
	
	// load local metadata
	this._readLocalMetadata(callback)
}

// callback(filePath, size)
// will receive (null, null) if not found
Cache.prototype.findBlob = function(hash, callback)
{
	if (hash in this.metadata.blobs)
	{
		var blob = this.metadata.blobs[hash]
		callback(path.join(this.root, "cache", hash), blob.size)
	}
	else
	{
		callback(null, null)
	}
}

// callback(asset)
// asset = {version1: hash1, version2: hash2, ...}
// callback will receive null if not found
Cache.prototype.findAsset = function(name, callback)
{
	if (name in this.metadata.assets)
	{
		var asset = this.metadata.assets[name]
		callback(asset)
	}
	else
	{
		callback(null)
	}
}

Cache.prototype.listAssets = function(pattern, callback)
{
	callback(null, minimatch.match(Object.keys(this.metadata.assets), pattern, {dot: true}))
}

Cache.prototype.listVersionAssets = function(version)
{
	var result = {}
	for (var name in this.metadata.assets)
	{
		if (this.metadata.assets[name][version] !== undefined)
		{
			var hash = this.metadata.assets[name][version]
			var blob = this.metadata.blobs[hash]
			result[name] = {hash: hash, size: blob.size}
		}
	}
	
	return result
}

Cache.prototype.createTemporaryFilename = function()
{
	return path.join(this.root, "temp", Math.floor(Math.random() * 1000000000000000).toString())
}

// callback(err, hash, size)
Cache.prototype.upgradeTemporary = function(tempFilename, name, version, callback)
{
	var self = this
	
	// compute hash (will become the file name)
	this._computeHash(tempFilename, function(err, hash)
	{
		if (err)
		{
			callback(err)
			return
		}
		
		// find file size
		fs.stat(tempFilename, function(err, stats)
		{
			if (err)
			{
				callback(err)
				return
			}
			
			// move the file out of temp/
			fs.rename(tempFilename, path.join(self.root, "cache", hash), function(err)
			{
				if (err)
				{
					callback(err)
					return
				}
				
				// update blob metadata
				if (self.metadata.blobs[hash] === undefined)
					self.metadata.blobs[hash] = {size: stats.size, refs: 0}
				
				self.metadata.blobs[hash].refs++;
				
				// update asset metadata
				if (self.metadata.assets[name] === undefined)
					self.metadata.assets[name] = {}
				
				self.metadata.assets[name][version] = hash
				
				// write new metadata
				self._writeLocalMetadata(function(err)
				{
					callback(err, hash, stats.size)
				})
			})
		})
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

// declare a new asset version using an existing hash
// the hash MUST exist in the cache
// callback(err)
Cache.prototype.declareVersion = function(name, version, hash, callback)
{
	// update blob metadata
	if (this.metadata.blobs[hash] === undefined)
	{
		callback(new Error("Cannot bind new asset metadata to non-existent hash"))
		return
	}
	this.metadata.blobs[hash].refs++;
	
	// update asset metadata
	if (this.metadata.assets[name] === undefined)
		this.metadata.assets[name] = {}
	
	this.metadata.assets[name][version] = hash
	
	// write new metadata
	this._writeLocalMetadata(callback)
}

// remove an asset version from the cache
// if the referenced blob is not used by any other asset version, it will be destroyed
// callback(err)
Cache.prototype.deleteVersion = function(name, version, callback)
{
	// check asset name
	if (this.metadata.assets[name] === undefined)
	{
		callback(new Error("Asset '" + name + "' not found in cache"))
		return
	}
	
	var asset = this.metadata.assets[name]
	
	// check version
	if (asset[version] === undefined)
	{
		callback(new Error("Version " + version + "of asset '" + name + "' not found in cache"))
		return
	}
	
	var hash = asset[version]
	delete(asset[version])
	
	// if no more versions for this asset, delete it
	if (Object.keys(asset).length == 0)
		delete(this.metadata.assets[name])
	
	// update blob reference count
	this.metadata.blobs[hash].refs--;
	
	// if this blob is not used any more, destroy its metadata
	if (this.metadata.blobs[hash].refs == 0)
		delete(this.metadata.blobs[hash])
	
	var self = this
	this._writeLocalMetadata(function(err)
	{
		if (err)
		{
			callback(err)
			return
		}
		
		// destroy blob if necessary
		if (self.metadata.blobs[hash] === undefined)
		{
			fs.unlink(path.join(self.root, "cache", hash), callback)
		}
		else
		{
			callback(null)
		}
	})
}

// rename a version (useful when a batch of assets from the same version
// finished transferring)
// callback(err)
Cache.prototype.renameVersion = function(currentVersionName, newVersionName, callback)
{
	for (var name in this.metadata.assets)
	{
		var asset = this.metadata.assets[name]
		if (currentVersionName in asset)
		{
			// transfer version info to new name
			asset[newVersionName] = asset[currentVersionName]
			delete(asset[currentVersionName])
		}
	}
	
	this._writeLocalMetadata(callback)
}

// callback(err, object)
Cache.prototype._readLocalMetadata = function(callback)
{
	// queue accesses to the metadata file to avoid read or write conflicts
	var self = this
	this.ioQueue.process(callback, function(callback)
	{
		var metadataPath = path.join(self.root, "local.json")
		fs.exists(metadataPath, function(exists)
		{
			if (exists)
			{
				fs.readFile(metadataPath, function(err, data)
				{
					if (err)
					{
						callback(err)
						return
					}
					
					try
					{
						self.metadata = JSON.parse(data)
						callback(null)
					}
					catch (err)
					{
						callback(err)
					}
				})
			}
			else
			{
				// no metadata, create a default object
				self.metadata = {blobs: {}, assets: {}}
				callback(null)
			}
		})
	})
}

// callback(err)
Cache.prototype._writeLocalMetadata = function(callback)
{
	// queue accesses to the metadata file to avoid read or write conflicts
	var self = this
	this.ioQueue.process(callback, function(callback)
	{
		var metadataPath = path.join(self.root, "local.json")
		fs.writeFile(metadataPath, JSON.stringify(self.metadata), "utf8", callback)
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
	stream.on("error", callback)
	
	// hashing done
	stream.on("close", function()
	{
		callback(null, md5sum.digest("hex"))
	})
}