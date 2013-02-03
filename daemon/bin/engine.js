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
var util = require("util")
var events = require("events")
var fs = require("fs")

function Engine(metadb, storage, cache)
{
	events.EventEmitter.call(this)
	
	this.metadb = metadb
	this.storage = storage
	this.cache = cache
	
	// path -> {version -> {hash, size}}
	this.requiredVersions = {}
}
util.inherits(Engine, events.EventEmitter)
exports.Engine = Engine

// callback(err)
Engine.prototype.start = function(callback)
{
	// first, read information about assets stored locally
	var self = this
	self.cache.initialize(function(err)
	{
		if (err)
		{
			callback(err)
			return
		}
		
		// start polling latest state continuously
		self._startPollingChanges(callback)
	})
}

// callback(err)
Engine.prototype.stop = function(callback)
{
	// stop the change feed
	this._stopPollingChanges()
	callback()
}

// callback(err, stream)
Engine.prototype.read = function(path, callback)
{
	var self = this
	self.cache.findAsset(path, function(asset)
	{
		var hash = null
		if ("local" in asset)
		{
			// there is a local version
			hash = asset["local"]
		}
		else
		{
			// find latest cached version
			var latestVersion = Math.max.apply(Math, Object.keys(asset))
			hash = asset[latestVersion]
		}
		
		self.cache.findBlob(hash, function(filePath, size)
		{
			var stream = fs.createReadStream(filePath)
			callback(null, stream)
		})
	})
}

// callback(err)
Engine.prototype.save = function(path, stream, callback)
{
	var self = this
	
	// create temp file for the new revision
	var tempFilename = this.cache.createTemporaryFilename()
	var writeStream = fs.createWriteStream(tempFilename)
	
	// error management helper
	function signalResult(err)
	{
		if (callback)
			callback(err)
		callback = null
	}
	
	stream.on("error", function(err)
	{
		signalResult(err)
	})
	
	stream.on("close", function()
	{
		// if the stream is still readable, the connection was abnormaly closed
		if (stream.readable)
			signalResult(new Error("Connection lost"))
	})
	
	writeStream.on("error", function(err)
	{
		// error while writing
		signalResult(err)
	})
	
	writeStream.on("close", function()
	{
		// temporary file written successfully; now try to make it persistent
		self.cache.upgradeTemporary(tempFilename, path, "local", function(err, hash, size)
		{
			// check the file was moved correctly
			if (err)
			{
				// fallback: try to remove the temporary
				self.cache.deleteTemporary(tempFilename, function(deleteErr)
				{
					// double error; just log
					if (deleteErr)
						console.log("cannot remove temporary file '" + tempFilename + "': " + deleteErr.message)
				})
				
				// signal the move error
				signalResult(err)
				return
			}
			
			// success!
			signalResult()
		})
	})
	
	// stream the file
	stream.pipe(writeStream)
}

// enumerate assets available locally
// the filter is in glob format
// callback(err, pathList)
Engine.prototype.list = function(filter, callback)
{
	var result = {}
	var self = this
	this.cache.listAssets(filter, function(err, assets)
	{
		if (err)
		{
			callback(err)
			return
		}
		
		for (var i = 0; i < assets.length; i++)
		{
			self.cache.findAsset(assets[i], function(asset)
			{
				var hash = null
				if ("local" in asset)
				{
					// there is a local version
					hash = asset["local"]
				}
				else
				{
					// find latest cached version
					var latestVersion = Math.max.apply(Math, Object.keys(asset))
					hash = asset[latestVersion]
				}
				
				self.cache.findBlob(hash, function(filePath, size)
				{
					result[assets[i]] = {hash: hash, size: size}
				})
			})
		}
		
		callback(null, result)
	})
}

// commit everything to servers
// callback(err, version) // version is a JS time value
Engine.prototype.share = function(description, callback)
{
	var version = (new Date()).getTime().toString()
	var self = this
	
	var localAssets = this.cache.listVersionAssets("local")
	
	// end of upload callback
	var uploadCount = Object.keys(localAssets).length
	var successCount = 0
	var failureCount = 0
	function uploadFinished(err)
	{
		if (err)
			failureCount++
		else
			successCount++
		
		// check for end of all uploads
		if (failureCount + successCount == uploadCount)
		{
			if (failureCount > 0)
			{
				// at least one upload failed
				callback(new Error("Sharing the version failed: some uploads did not end successfully"))
			}
			else
			{
				// all data uploads are done, now upload metadata
				var versionDocument = {
					description: description,
					author: "Mr Plop",
					assets: localAssets
				}
				self.metadb.save(version, versionDocument, function(err, result)
				{
					if (err)
					{
						callback(err, version)
						return
					}
					
					// update local cache version information
					self.cache.renameVersion("local", version, function(err)
					{
						callback(err, version)
					})
				})
			}
		}
	}
	
	// upload all locally modified assets
	for (var name in localAssets)
	{
		var asset = localAssets[name]
		self.cache.findBlob(asset.hash, function(filePath, size)
		{
			// start actual upload
			self.storage.upload(asset.hash, filePath, uploadFinished)
		})
	}
}

// callback(err, version)
Engine.prototype.readVersion = function(version, callback)
{
	if (version == "local")
	{
		callback(null, {
			author: "CurrentUser",
			description: "The description does not exist",
			assets: this.cache.listVersionAssets("local")
		})
	}
	else
	{
		this.metadb.get(version, function(err, document)
		{
			if (err)
			{
				callback(err)
			}
			else
			{
				callback(null, {
					author: document.author,
					description: document.description,
					assets: document.assets,
				})
			}
		})
	}
}

/*Engine.prototype.lock = function(id, application)
{
	if (this.assets[id] == undefined)
		return // this id does not exist
	
	if (this.assets[id].lock != undefined)
		return // already locked
	
	this.storage.lock(id, application)
}

Engine.prototype.unlock = function(id)
{
	if (this.assets[id] == undefined)
		return // this id does not exist
	
	if (this.assets[id].lock == undefined)
		return // not locked
	
		if (this.assets[id].lock.user != "MrPlop")
		return // locked by someone else
	
	this.storage.unlock(id)
}*/

// Continuously check latest changes from meta db
Engine.prototype._startPollingChanges = function(callback)
{
	var self = this
	this.metadb.view("wendy/latest", {group: true, update_seq: true}, function(err, result)
	{
		if (err)
		{
			callback(new Error("Cannot read view wendy/latest in metadb: " + JSON.stringify(err)))
			return
		}
		
		result.forEach(function(path, latestVersion)
		{
			console.log(path)
			//self.assets[path] = latestVersion
			if (!self.requiredVersions[path])
				self.requiredVersions[path] = {}
			
			var version = Object.keys(latestVersion)[0]
			self.requiredVersions[path][version] = latestVersion[version]
			self._checkAssetLocalContent(path)
		})
		
		self.changeFeed = self.metadb.changes({since: result.json.update_seq, include_docs: true})
		self.changeFeed.addListener("change", function(change)
		{
			self._onChange(change)
		})
		
		callback()
	})
}

Engine.prototype._stopPollingChanges = function()
{
	this.changeFeed.removeAllListeners("change")
	this.changeFeed.stop()
}

Engine.prototype._onChange = function(change)
{
	var version = change._id
	var assets = change.doc.assets
	
	for (var path in assets)
	{
		console.log("updated " + path)
		this.requiredVersions[path][version] = assets[path]
		this._checkAssetLocalContent(path)
	}
}

// Compare asset local state to the required state
Engine.prototype._checkAssetLocalContent = function(path)
{
	var self = this
	
	if (!self.requiredVersions[path])
		return // if nothing is required, we are up to date
	
	// check that we have all required versions
	var required = self.requiredVersions[path]
	for (var version in required)
	{
		console.log("required: " + path + " at " + version)
		var hash = required[version].hash
		
		// if hash is null, the asset was deleted
		if (hash)
		{
			// look for the version in the local cache
			self.cache.find(hash, function(location)
			{
				if (location != "cache")
				{
					// the latest version was not found, start download
					var tempFilename = self.cache.createTemporaryFilename()
					self.storage.download(hash, tempFilename, function(err)
					{
						if (err)
						{
							// remove temporary file
							self.cache.deleteTemporary(tempFilename, function(deleteErr)
							{
								// double error; just print
								if (deleteErr)
									console.log("unable to remove temporary file " + tempFilename + " (because of failed download): " + deleteErr.message)
							})
							return
						}
						
						// download finished, save in cache
						self.cache.upgradeTemporary(tempFilename, "cache", function(err)
						{
							if (err) throw err
							
							if (self.local.cache[path] === undefined)
								self.local.cache[path] = {}
							
							self.local.cache[path][version] = required[version]
						})
					})
				}
				else
				{
					// already cached
					if (self.local.cache[path] === undefined)
						self.local.cache[path] = {}
					
					console.log("cached version " + version + " of asset " + path)
					self.local.cache[path][version] = required[version]
					console.log(self.local.cache)
				}
			})
		}
	}
	
	/*var asset = this.assets[path]
	var latest = asset[Object.keys(asset)[0]]
	
	// look for the latest version in the local cache
	this.cache.find(latest.hash, function(location)
	{
		if (location != "cache")
		{
			// the latest version was not found, start download
			self._download(latest.hash)
		}
	})*/
	
	/*asset.state = "outdated"
	
	// find last revision number
	var revisions = asset.revisions
	if (!revisions)
		return
	var lastRevision = Math.max.apply(Math, Object.keys(revisions))
	
	// check associated blob
	var blob = revisions[lastRevision].blob
	if ((blob != undefined) && (!(id in this.blobs) || (this.blobs[id].indexOf(blob) == -1)))
	{
		// this blob must be dowloaded
		asset.state = "downloading"
		revisions[lastRevision].state = "downloading"
		this._download(id, blob)
	}
	else
	{
		// the asset is up to date
		asset.state = "uptodate"
		revisions[lastRevision].state = "cached"
	}
	
	console.log("STATE of " + id + " -> " + asset.state)*/
	
	//this.emit("changed", path, latest)
}
