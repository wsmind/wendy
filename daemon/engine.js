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
	
	// Asset structure
	// {
	//    lock: {user: <string>, application: <string>},
	//    revisions: {<int>: {rev}, <int>: {rev2}, ...},
	//    state: "outofdate" | "downloading" | "uptodate"
	// }
	//
	// Revision structure
	// (path and blob will be undefined if the asset was deleted in this revision)
	// {
	//    author: <string>,
	//    date: <unix timestamp>
	//    //tag: <string>
	//    [path: <string>,]
	//    [blob: <int>,]
	//    [type: <mime>]
	//    [length: <int>]
	// }
	//
	// Blobs
	// {<id>: [<int array>]}
	this.assets = {}
	//this.blobs = {}
	
	// waiting (not started) downloads
	this.downloadQueue = []
	
	// currently downloading
	this.currentDownloads = []
	this.MAX_DOWNLOADS = 4
	
	// waiting (not started) uploads
	this.uploadQueue = []
	
	// currently uploading
	this.currentUploads = []
	this.MAX_UPLOADS = 4
	
	// currently sharing (includes multiple uploads)
	this.currentShares = {}
	
	// first, read information about assets locally modified
	var self = this
	self.cache.readLocalMetadata(function(err, localMetadata)
	{
		if (err) throw err
		
		self.local = localMetadata
		
		// start polling latest state continuously
		self._pollChanges()
	})
}
util.inherits(Engine, events.EventEmitter)
exports.Engine = Engine

// callback(err, stream)
Engine.prototype.read = function(path, callback)
{
	var hash = null
	
	// check for a local version first
	if (path in this.local)
	{
		hash = this.local[path].hash
	}
	else
	{
		// if nothing local, check downloaded revisions
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
	}
	
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
		self.cache.upgradeTemporary(tempFilename, "wip", function(err, hash)
		{
			// check the file was moved correctly
			if (err)
			{
				// fallback: try to remove the temporary
				self.cache.deleteTemporary(tempFilename, function(deleteErr)
				{
					// double error; just log
					console.log("cannot remove temporary file '" + tempFilename + "': " + deleteErr.message)
				})
				
				// signal the move error
				signalResult(err)
				return
			}
			
			// tag the asset as modified locally
			self._updateMetadata(path, hash)
			
			// success!
			signalResult()
		})
	})
	
	// stream the file
	stream.pipe(writeStream)
}

// enumerate assets
// the filter is in glob format
// callback(err, pathList)
Engine.prototype.list = function(filter, callback)
{
	callback(null, minimatch.match(Object.keys(this.assets), filter, {nonull: true}))
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
			assets: this.local
		}
	}
	
	// upload all locally modified assets
	for (var name in this.local)
	{
		var asset = this.local[name]
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
		callback(null, this.local)
	}
	else
	{
		callback(new Error("Not implemented (yet)"))
	}
}

Engine.prototype._updateMetadata = function(path, hash)
{
	// save new local revision
	this.local[path] = {hash: hash}
	
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
Engine.prototype._pollChanges = function()
{
	var self = this
	this.metadb.view("history/latest", {group: true, update_seq: true}, function(err, result)
	{
		if (err) throw err
		
		result.forEach(function(path, latestVersion)
		{
			console.log(path)
			self.assets[path] = latestVersion
			self._checkAssetState(path)
		})
		
		var feed = self.metadb.changes({since: result.json.update_seq, include_docs: true})
		feed.on("change", function(change)
		{
			console.log(change)
		})
	})
}

// Determine asset state, and take necessary actions
// to get it up to date
Engine.prototype._checkAssetState = function(path)
{
	var self = this
	var asset = this.assets[path]
	var latest = asset[Object.keys(asset)[0]]
	
	// look for the latest version in the local cache
	this.cache.find(latest.hash, function(location)
	{
		if (location != "cache")
		{
			// the latest version was not found, start download
			self._download(latest.hash)
		}
	})
	
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
	
	this.emit("changed", path, latest)
}

Engine.prototype._downloadCallback = function(err, hash)
{
	if (err)
	{
		console.log("download of " + hash + " failed: " + err.message)
		return
	}
	
	console.log("download finished: " + hash)
}

Engine.prototype._download = function(hash)
{
	// check if already planned for downloading
	if (this.downloadQueue.indexOf(hash) != -1)
		return
	
	// check if already downloading
	if (this.currentDownloads.indexOf(hash) != -1)
		return
	
	// start actual download
	this.downloadQueue.push(hash)
	this._processDownloads()
}

Engine.prototype._processDownloads = function()
{
	while ((this.downloadQueue.length > 0) && (this.currentDownloads.length < this.MAX_DOWNLOADS))
	{
		var hash = this.downloadQueue.shift()
		this.currentDownloads.push(hash)
		
		console.log("starting download: " + hash)
		
		var self = this
		
		var tempFilename = self.cache.createTemporaryFilename()
		this.storage.download(hash, tempFilename, function(err)
		{
			// remove from current downloads
			self.currentDownloads.splice(self.currentDownloads.indexOf(hash), 1)
			
			// start next downloads (if any)
			self._processDownloads()
			
			//if (err) throw err
			if (err)
			{
				// remove temporary file
				self.cache.deleteTemporary(tempFilename, function(deleteErr)
				{
					// double error; just print
					if (deleteErr)
						console.log("unable to remove temporary file " + tempFilename + " (because of failed download): " + deleteErr.message)
					
					// signal download error
					self._downloadCallback(err, hash)
				})
				return
			}
			
			// download finished, save in cache
			self.cache.upgradeTemporary(tempFilename, "cache", function(err)
			{
				//if (err) throw err
				if (err)
				{
					self._downloadCallback(err, hash)
					return
				}
				
				// download complete
				self._downloadCallback(null, hash)
			})
		})
	}
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
						self.local = {}
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
