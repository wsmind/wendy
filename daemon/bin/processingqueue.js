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

/**
 * The goal of a processing queue is to provide a generic way to
 * manage resource wisely.
 *
 * It works by guaranteeing that only a maximum number of processes will
 * take place at the same time, and, conversely, by maximizing the number
 * of running processes to its capacity whenever possible.
 *
 * Typical usage: download/upload queues with a fixed number of
 * parallel transfers.
 */
function ProcessingQueue(capacity)
{
	this.capacity = capacity
	this.pending = [] // array of handlers
	this.running = 0
}
exports.ProcessingQueue = ProcessingQueue

// Do something as soon as a free slot is available.
//
// handler(done) must call done() when processing has finished.
//
// The done() function will forward all its parameters to the given callback,
// so that the termination information can be sent outside the queue.
//
// If no callback is required, just set it to null.
ProcessingQueue.prototype.process = function(callback, handler)
{
	var request = {
		callback: callback,
		handler: handler,
		finished: false
	}
	this.pending.push(request)
	this._checkPending()
}

ProcessingQueue.prototype._checkPending = function()
{
	while ((this.pending.length > 0) && (this.running < this.capacity))
	{
		// there is enough room to run one more
		var request = this.pending.shift()
		this.running++
		
		// do the actual processing
		var self = this
		request.handler(function()
		{
			assert(!request.finished)
			request.finished = true
			
			// signal end to the outside world
			if (request.callback)
				request.callback.apply(this, arguments)
			
			// when done, flag a free slot and check the pending queue
			self.running--
			self._checkPending()
		})
	}
}
