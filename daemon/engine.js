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
	// (author and path will be null if the asset was deleted in this revision)
	// {
	//    author: <string>,
	//    path: <string>,
	//    blob: <int>
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
	if (!(id in this.blobs) || (this.blobs[id].indexOf(blob) == -1))
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
