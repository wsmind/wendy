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
var minimatch = require("minimatch")

function Engine(metadb, storage, cache)
{
	events.EventEmitter.call(this)
	
	this.metadb = metadb
	this.storage = storage
	this.cache = cache
	
	// path -> {version -> {hash, size}}
	this.requiredVersions = {}
	
	// waiting (not started) uploads
	this.uploadQueue = []
	
	// currently uploading
	this.currentUploads = []
	this.MAX_UPLOADS = 4
	
	// currently sharing (includes multiple uploads)
	this.currentShares = {}
}
util.inherits(Engine, events.EventEmitter)
exports.Engine = Engine

// callback(err)
Engine.prototype.start = function(callback)
{
	// first, read information about assets stored locally
	var self = this
	self.cache.readLocalMetadata(function(err, localMetadata)
	{
		if (err)
		{
			callback(err)
			return
		}
		
		self.local = localMetadata
		
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
	var hash = null
	
	// check for a local version first
	if (path in this.local.wip)
	{
		hash = this.local.wip[path].hash
	}
	/*else
	{
		// if nothing local, check downloaded revisions
		assert(false, "TODO: reimplement")
		var asset = this.assets[path]
		
		// check asset existence
		if (!asset)
		{
			callback(new Error("Asset not found: '" + path + "'"))
			return
		}
		
		// find latest blob
		var latest = asset[Object.keys(asset)[0]]
		var hash = latest.hash
	}*/
	
	// check that this blob is available
	this.cache.find(hash, function(location, filePath)
	{
		if (!location)
		{
			callback(new Error("Data not available for asset '" + path + "'"))
			return
		}
		
		// blob successfully found
		var stream = fs.createReadStream(filePath)
		callback(null, stream)
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
		self.cache.upgradeTemporary(tempFilename, "wip", function(err, hash, size)
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
			
			// tag the asset as modified locally
			self._updateMetadata(path, hash, size)
			
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
	var mm = minimatch.Minimatch(filter, {})
	var result = {}
	for (var path in this.local.wip)
	{
		if (mm.match(path))
		{
			result[path] = this.local.wip[path]
		}
	}
	
	callback(null, result)
}

// commit everything to servers
// callback(err, version) // version is a JS time value
Engine.prototype.share = function(description, callback)
{
	var version = (new Date()).getTime()
	
	var shareDescription = {
		hashes: [],
		callback: callback,
		meta: {
			description: description,
			author: "Mr Plop",
			assets: this.local.wip
		}
	}
	
	// upload all locally modified assets
	for (var name in this.local.wip)
	{
		var asset = this.local.wip[name]
		this._upload(asset.hash)
		shareDescription.hashes.push(asset.hash)
	}
	
	// the share description is then handled in the upload callback
	this.currentShares[version] = shareDescription
}

// callback(err, version)
Engine.prototype.readVersion = function(version, callback)
{
	if (version == "local")
	{
		callback(null, {
			author: "CurrentUser",
			description: "The description does not exist",
			assets: this.local.wip
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

Engine.prototype._updateMetadata = function(path, hash, size)
{
	// save new local revision
	this.local.wip[path] = {hash: hash, size: size}
	
	// write metadata
	this.cache.writeLocalMetadata(this.local, function(err)
	{
		if (err)
		{
			console.log("FATAL ERROR: unable to update metadata for asset '" + path + "': new hash should be '" + hash + "'")
			throw err
		}
	})
}

/*Engine.prototype.dump = function(callback)
{
	for (var id in this.assets)
	{
		callback(id, this.assets[id])
	}
}

Engine.prototype.create = function(path)
{
	var asset = {
		revisions: {
			"0": {
				author: "Mr Blob",
				path: path,
				date: 42,
			}
		}
	}
	
	this.storage.create(42, asset)
}

Engine.prototype.lock = function(id, application)
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
}

// mode must be "r" or "w"
// callback(err, file)
Engine.prototype.open = function(id, mode, callback)
{
	assert((mode == "r") || (mode == "w"))
	
	if (mode == "r")
	{
		if (this.blobs[id] === undefined)
		{
			callback(new Error("No blob available for asset " + id))
			return
		}
		
		// when opening for reading, find the latest complete blob available
		// for this asset and open it
		var lastBlob = null
		var revisions = this.assets[id].revisions
		if (revisions === undefined)
		{
			callback(new Error("Asset " + id + " has no revisions"))
			return
		}
		
		var blobs = this.blobs[id]
		for (var revisionId in revisions)
		{
			// check is this blob has finished downloading
			var blob = revisions[revisionId].blob
			if (blobs.indexOf(blob) != -1)
				lastBlob = revisions[revisionId].blob
		}
		
		if (lastBlob === null)
		{
			callback(new Error("No blob found in revisions (asset " + id + ")"))
			return
		}
		
		console.log("opening asset " + id + ", blob " + lastBlob)
		
		this.cache.open(id, lastBlob, mode, function(file)
		{
			callback(null, file)
		})
	}
	else
	{
		// to open an asset for writing, the asset must be locked by the current
		// user
		assert(!"not implemented")
	}
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
						})
					})
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

Engine.prototype._uploadCallback = function(err, hash)
{
	var self = this
	
	if (err)
	{
		console.log("upload of " + hash + " failed: " + err.message)
		return
	}
	
	// check current share operations
	for (var version in self.currentShares)
	{
		var share = self.currentShares[version]
		
		// check finished upload
		var hashIndex = share.hashes.indexOf(hash)
		if (hashIndex != -1)
		{
			// remove from upload list
			share.hashes.splice(hashIndex, 1)
			
			// move from wip to cache
			self.cache.move(hash, "wip", "cache", function(err)
			{
				if (err)
				{
					share.callback(err, version)
					return
				}
				
				// check if everything was uploaded
				if (share.hashes.length == 0)
				{
					// all data uploads are done, now upload metadata
					self.metadb.save(version, share.meta, function(err, result)
					{
						if (err)
						{
							share.callback(err, version)
							return
						}
						
						// finally, update local metadata
						self.local.wip = {}
						self.cache.writeLocalMetadata(self.local, function(err)
						{
							share.callback(err, version)
						})
					})
				}
			})
		}
	}
	
	console.log("upload finished: " + hash)
}

Engine.prototype._upload = function(hash)
{
	// check if already planned for uploading
	if (this.uploadQueue.indexOf(hash) != -1)
		return
	
	// check if already uploading
	if (this.currentUploads.indexOf(hash) != -1)
		return
	
	// start actual upload
	this.uploadQueue.push(hash)
	this._processUploads()
}

Engine.prototype._processUploads = function()
{
	while ((this.uploadQueue.length > 0) && (this.currentUploads.length < this.MAX_UPLOADS))
	{
		var hash = this.uploadQueue.shift()
		this.currentUploads.push(hash)
		
		console.log("starting upload: " + hash)
		
		var self = this
		
		// find a full path for this hash
		self.cache.find(hash, function(location, filePath)
		{
			assert(location == "wip") // only uploads from the wip directory make sense
			
			// start actual upload
			self.storage.upload(hash, filePath, function(err)
			{
				// remove from current uploads
				self.currentUploads.splice(self.currentUploads.indexOf(hash), 1)
				
				// start next uploads (if any)
				self._processUploads()
				
				if (err)
				{
					self._uploadCallback(err, hash)
					return
				}
				
				// signal success
				self._uploadCallback(null, hash)
			})
		})
	}
}
