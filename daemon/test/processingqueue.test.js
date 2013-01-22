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

var ProcessingQueue = require("../bin/processingqueue").ProcessingQueue

describe("processingqueue", function()
{
	it("processes one thing", function(done)
	{
		var queue = new ProcessingQueue(10)
		queue.process(done, function(done)
		{
			done()
		})
	})
	
	it("forwards arguments", function(done)
	{
		function callback(param1, param2, param3)
		{
			assert.strictEqual(param1, null)
			assert.strictEqual(param2, "20")
			assert.strictEqual(param3, 30)
			done()
		}
		
		var queue = new ProcessingQueue(10)
		queue.process(callback, function(callback)
		{
			callback(null, "20", 30)
		})
	})
	
	it("runs processes in parallel", function(done)
	{
		var queue = new ProcessingQueue(3)
		var running = []
		
		// push queue to maximum capacity
		for (var i = 0; i < 3; i++)
		{
			queue.process(null, function(done)
			{
				running.push(done)
			})
			
			assert.equal(running.length, i + 1);
		}
		
		// add more
		for (var i = 0; i < 10; i++)
		{
			queue.process(null, function(done)
			{
				running.push(done)
			})
			
			// number of running processes should not overflow capacity
			assert.equal(running.length, 3);
		}
		
		// finish some
		for (var i = 0; i < 5; i++)
		{
			var handler = running.shift()
			handler()
			
			// we are still over capacity
			assert.equal(running.length, 3);
		}
		
		// finish some more
		for (var i = 0; i < 5; i++)
		{
			var handler = running.shift()
			handler()
			
			// we are still over capacity
			assert.equal(running.length, 3);
		}
		
		// finally, free all the slots
		for (var i = 0; i < 3; i++)
		{
			var handler = running.shift()
			handler()
			
			// there are some free slots
			assert.equal(running.length, 3 - (i + 1));
		}
		
		// everything flushed
		assert.equal(running.length, 0);
		
		done()
	})
	
	it("checks multiple calls to done()", function(testDone)
	{
		var queue = new ProcessingQueue(10)
		queue.process(null, function(done)
		{
			// first time
			done()
			
			assert.throws(
				function()
				{
					// second time
					done()
				},
				assert.AssertionError
			)
			
			testDone()
		})
	})
})
