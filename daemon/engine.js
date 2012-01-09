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

var assert = require("assert")
var util = require("util")
var events = require("events")

function Engine(storage, cache)
{
	events.EventEmitter.call(this)
	
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
	//    [path: <string>,]
	//    [blob: <int>,]
	//    date: <unix timestamp>
	// }
	//
	// Blobs
	// {<id>: [<int array>]}
	this.assets = {}
	this.blobs = {}
	
	// waiting (not started) downloads
	this.downloadQueue = []
	
	// currently downloading
	this.currentDownloads = []
	this.MAX_DOWNLOADS = 4
	
	var self = this
	this.cache.dump(function(blobs)
	{
		self.blobs = blobs
		
		// start listening to storage changes
		self.storage.watchChanges(function(id, asset)
		{
			self.assets[id] = asset
			
			self._checkAssetState(id)
		})
	})
}
util.inherits(Engine, events.EventEmitter)
exports.Engine = Engine

Engine.prototype.dump = function(callback)
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
	this.storage.lock(id, application)
}

// mode must be "r" or "w"
Engine.prototype.open = function(id, mode, callback)
{
	assert((mode == "r") || (mode == "w"))
	
	if (mode == "r")
	{
		if (this.blobs[id] === undefined)
		{
			callback("No blob available for asset " + id)
			return
		}
		
		// when opening for reading, find the latest complete blob available
		// for this asset and open it
		var lastBlob = null
		var revisions = this.assets[id].revisions
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
			callback("No blob found in revisions (asset " + id + ")")
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
}

// Determine asset state, and take necessary actions
// to get it up to date
Engine.prototype._checkAssetState = function(id)
{
	var asset = this.assets[id]
	
	asset.state = "outdated"
	
	// find last revision number
	var revisions = asset.revisions
	var lastRevision = Math.max.apply(Math, Object.keys(revisions))
	
	// check associated blob
	var blob = revisions[lastRevision].blob
	if ((blob != undefined) && (!(id in this.blobs) || (this.blobs[id].indexOf(blob) == -1)))
	{
		// this blob must be dowloaded
		asset.state = "downloading"
		this._download(id, blob)
	}
	else
	{
		// the asset is up to date
		asset.state = "uptodate"
	}
	
	console.log("STATE of " + id + " -> " + asset.state)
	this.emit("changed", id, asset)
}

Engine.prototype._download = function(id, blob)
{
	var download = {"id": id, "blob": blob}
	
	// TODO: check if already in this.downloadQueue or this.currentDownloads
	
	this.downloadQueue.push(download)
	this._processDownloads()
}

Engine.prototype._processDownloads = function()
{
	while ((this.downloadQueue.length > 0) && (this.currentDownloads.length < this.MAX_DOWNLOADS))
	{
		var download = this.downloadQueue.shift()
		this.currentDownloads.push(download)
		
		var self = this
		this.cache.open(download.id, download.blob, "w", function(file)
		{
			self.storage.download(download.id, download.blob, file, function()
			{
				// flag new blob
				if (!(download.id in self.blobs))
					self.blobs[download.id] = []
				self.blobs[download.id].push(download.blob)
				
				// remove from current downloads
				self.currentDownloads.splice(self.currentDownloads.indexOf(download), 1)
				
				// determine new asset state
				self._checkAssetState(download.id)
				
				// start next downloads (if any)
				self._processDownloads()
			})
		})
	}
}
