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
