###############################################################################
# 
# Wendy asset manager
# Copyright (c) 2011 Remi Papillie
# 
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 
#    1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 
#    2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 
#    3. This notice may not be removed or altered from any source
#    distribution.
# 
###############################################################################

import os
import threading

class OpenFile:
	"""
	Information on a file being accessed
	"""
	
	def __init__(self):
		# number of client reading or writing this file
		self.reading = 0
		self.writing = 0

class Cache:
	"""
	Monitors access to actual files containing asset content. The
	goal of this module is mainly to ensure read and write consistency
	even when bad events occur (e.g crashes).
	
	Access to the cache is thread-safe.
	"""
	
	def __init__(self, root):
		self._root = root
		self._lock = threading.Lock()
		
		self._lock.acquire()
		
		# keep track of open files at runtime
		# (lock files are also used to know when a crash happened)
		self._openFiles = {} # map filename -> OpenFile
		
		self._lock.release()
	
	def open(self, assetId, revision, mode):
		"""
		Try to access a given asset/rev pair.
		mode should be "read" or "write"
		
		Note that this function does not actually open a filesystem
		object, but just gives access to it.
		"""
		
		self._lock.acquire()
		
		# compute filename from ID and revision
		filename = self._makeFilename(assetId, revision)
		
		# check if the file is already opened for writing
		# (writing is exclusive for a given revision)
		if filename in self._openFiles:
			if mode == "write":
				self._lock.release()
				return None
			
			if self._openFiles[filename].writing == 1:
				self._lock.release()
				return None
		
		# check if the file actually exists (when reading)
		if (mode == "read") and (not os.path.isfile(filename)):
			self._lock.release()
			return None
		
		# flag the file as opened
		if not filename in self._openFiles:
			self._openFiles[filename] = OpenFile()
		
		if mode == "read":
			self._openFiles[filename].reading = self._openFiles[filename].reading + 1
		else:
			self._openFiles[filename].writing = self._openFiles[filename].writing + 1
		
		self._lock.release()
		
		return filename
	
	def close(self, assetId, revision):
		self._lock.acquire()
		
		filename = self._makeFilename(assetId, revision)
		
		if filename in self._openFiles:
			if self._openFiles[filename].reading > 0:
				self._openFiles[filename].reading = self._openFiles[filename].reading - 1
			else:
				self._openFiles[filename].writing = self._openFiles[filename].writing - 1
			
			# remove handle if no more needed
			if (self._openFiles[filename].reading == 0) and (self._openFiles[filename].writing == 0):
				del(self._openFiles[filename])
		
		self._lock.release()
	
	def _makeFilename(self, assetId, revision):
		return os.path.join(self._root, assetId + "-" + revision)

if __name__ == "__main__":
	# create folder for tests
	root = "cache-test-root-folder/"
	
	try:
		os.mkdir(root)
		
		# create some existing cache elements
		fake = open(os.path.join(root, "plop-45"), "w")
		fake.write("hahahaha")
		fake.close()
		
		c = Cache(root)
		
		# existing cache file
		read1 = c.open("plop", "45", "read")
		read2 = c.open("plop", "45", "read")
		assert(read1 != None)
		assert(os.path.isfile(read1))
		assert(read2 != None)
		assert(os.path.isfile(read2))
		
		# unexisting
		read3 = c.open("plop", "42", "read")
		assert(read3 == None)
		
		# leave only one open reference to plop-45
		c.close("plop", "45")
		
		# cannot open for writing if someone is reading
		assert(c.open("plop", "45", "write") == None)
		
		# close all reading handles
		c.close("plop", "45")
		
		# try writing again (should now work)
		write1 = c.open("plop", "45", "write")
		assert(write1 != None)
		assert(os.path.isfile(write1))
	
	finally:
		for file in os.listdir(root):
			os.remove(os.path.join(root, file))
		os.rmdir(root)
