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
function Cache(path)
{
	this.root = path
}
exports.Cache = Cache

// mode must be "r" or "w"
Cache.prototype.open = function(id, revision, mode, callback)
{
	assert((mode == "r") || (mode == "w"))
	
	fs.open(path.join(this.root, id + "-" + revision), mode, 0666, function(err, fd)
	{
		// TODO: handle error
		assert(!err)
		
		var file = new AssetFile(fd)
		callback(file)
	})
}

function AssetFile(fd)
{
	this.fd = fd
}

AssetFile.prototype.read = function(buffer, callback)
{
	fs.read(this.fd, buffer, 0, buffer.length, null, function(err, bytesRead, buffer)
	{
		callback(err, bytesRead, buffer)
	})
}

AssetFile.prototype.write = function(buffer, callback)
{
	fs.write(this.fd, buffer, 0, buffer.length, null, function(err, written, buffer)
	{
		callback(err, written, buffer)
	})
}

AssetFile.prototype.close = function()
{
	fs.close(this.fd)
}
