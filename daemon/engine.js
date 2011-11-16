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
	//    revisions: {<int>: {rev}, <int>: {rev2}, ...}
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
	
	var self = this
	this.cache.dump(function(blobs)
	{
		self.blobs = blobs
		
		// start listening to storage changes
		self.storage.watchChanges(function(id, asset)
		{
			self.assets[id] = asset
			
			self._checkAsset(id)
			
			console.log(asset)
			self.emit("changed", asset)
		})
	})
}
util.inherits(Engine, events.EventEmitter)
exports.Engine = Engine

Engine.prototype.dump = function(callback)
{
	for (var id in this.assets)
	{
		callback(this.assets[id])
	}
}

Engine.prototype._checkAsset = function(id)
{
	// find last revision number
	var revisions = this.assets[id].revisions
	
	// check associated blob
	
	// put into download queue
}
