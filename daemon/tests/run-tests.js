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
var child_process = require("child_process")

if (!process.argv[2])
{
	console.log("Usage: " + process.argv[1] + " <folder>")
	process.exit()
}

var testFolder = process.argv[2]
console.log("Running tests from '" + testFolder + "'")

fs.readdir(testFolder, function(err, files)
{
	if (err) throw err
	
	// run each test in sequence
	function runNextTest(tests)
	{
		if (tests.length == 0)
			return
		
		var testFile = tests.shift()
		console.log("== " + testFile + " ==")
		
		// start test
		var child = child_process.fork(path.join(testFolder, testFile))
		
		// wait for termination
		child.on("exit", function(code, signal)
		{
			// output result
			console.log(testFile + " exited with code " + code)
			
			// start next test
			runNextTest(tests)
		})
	}
	
	// start first test
	runNextTest(files)
})
