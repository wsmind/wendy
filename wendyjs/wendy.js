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

"use strict";

function WendyClient()
{
}

// callback(err, data)
WendyClient.prototype.list = function(filter, callback)
{
	var xhr = new XMLHttpRequest()
	xhr.open("GET", "/list/" + encodeURIComponent(filter))
	
	xhr.onload = function(progress)
	{
		callback(null, JSON.parse(this.response))
	}
	
	xhr.onerror = function(progress)
	{
		callback(new Error("errooooooooooor: " + this.status))
	}
	
	xhr.send()
}

// callback(err, data)
WendyClient.prototype.read = function(path, responseType, callback)
{
	var xhr = new XMLHttpRequest()
	xhr.open("GET", "/data" + path)
	xhr.responseType = responseType
	
	xhr.onload = function(progress)
	{
		callback(null, this.response)
	}
	
	xhr.onerror = function(progress)
	{
		callback(new Error("errooooooooooor: " + this.status))
	}
	
	xhr.send()
}

// version must be "local", or a time value
// callback(err, version)
WendyClient.prototype.readVersion = function(version, callback)
{
	var xhr = new XMLHttpRequest()
	xhr.open("GET", "/version/" + version)
	
	xhr.onload = function(progress)
	{
		callback(null, JSON.parse(this.response))
	}
	
	xhr.onerror = function(progress)
	{
		callback(new Error("errooooooooooor: " + this.status))
	}
	
	xhr.send()
}

// callback(err)
WendyClient.prototype.share = function(description, callback)
{
	var xhr = new XMLHttpRequest()
	xhr.open("POST", "/share")
	
	xhr.onload = function(progress)
	{
		alert("shared: " + this.status)
		callback()
	}
	
	xhr.onerror = function(progress)
	{
		callback(new Error("errooooooooooor"))
	}
	
	xhr.send("description=" + encodeURIComponent(description))
}
