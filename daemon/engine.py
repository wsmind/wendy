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

import threading
import zmq
import uuid

class StorageWatcher:
	def __init__(self, storage, context):
		self.storage = storage
		self.socket = context.socket(zmq.PUB)
		self.socket.bind("inproc://storage-changes")
		self.thread = threading.Thread(target = self)
		self.thread.daemon = True
		self.thread.start()
	
	def __call__(self):
		for metadata in self.storage.pollChanges():
			metadata["type"] = "metadata-update"
			self.socket.send_pyobj(metadata)

class LocalAsset:
	def __init__(self):
		self.id = "0"
		self.path = ""
		self.lock = None
		self.lastRevision = ""
		self.localRevision = ""

class Engine:
	def __init__(self, context, storage):
		self.context = context
		self.storage = storage
		self.remoteAssets = {} # dictionary of couchdb.Document
		self.localAssets = {} # dictionary of LocalAsset
		self.listeners = []
	
	def run(self):
		"Start all action threads and engine main event loop"
		watcher = StorageWatcher(self.storage, self.context)
		
		socket = self.context.socket(zmq.SUB)
		socket.setsockopt(zmq.SUBSCRIBE, "")
		socket.connect("inproc://storage-changes")
		socket.connect("inproc://service-actions")
		
		# main event loop
		while True:
			msg = socket.recv_pyobj()
			if msg["type"] == "metadata-update":
				print("NEW METADATA RECEIVED!!")
			elif msg["type"] == "create-action":
				print("CREATE ASSET")
			elif msg["type"] == "delete-action":
				print("DELETE ASSET")
	
	def addListener(self, listener):
		self.listeners.append(listener)
	
	def removeListener(self, listener):
		self.listeners.remove(listener)
	
	def dumpAssets(self, listener):
		"Dump all assets to a given listener, as if they were all just updated"
		
		# TODO: use local assets instead
		# TODO: add locking on localAssets
		for asset in self.remoteAssets.values():
			listener.assetChanged(asset.id, asset)
	
	def assetChanged(self, assetId, asset):
		# check for duplicate notification
		if assetId in self.remoteAssets:
			if self.remoteAssets[assetId].rev == asset.rev:
				return
		
		# TODO: translate to local asset information (+ download info, local rev, etc.)
		self.remoteAssets[asset.id] = asset
		
		for listener in self.listeners:
			listener.assetChanged(assetId, asset)
	
	def assetRemoved(self, assetId):
		if assetId in self.remoteAssets:
			del(self.remoteAssets[assetId])
			
			for listener in self.listeners:
				listener.assetRemoved(assetId)
	
	def addAsset(self, path):
		newId = uuid.uuid4().hex
		self.db[newId] = {"path": path}

class EngineListener:
	def assetChanged(self, assetId, asset):
		pass
	
	def assetRemoved(self, assetId):
		pass

if __name__ == "__main__":
	
	class PlopStorage:
		"Fake storage implementation. Generates one change per second."
		
		# blocking generator
		def pollChanges(self):
			import time
			import random
			ids = [uuid.uuid4().hex for i in range(0, 4)]
			while True:
				time.sleep(1)
				currentIndex = random.randint(0, 3)
				doc = {
					"id": ids[currentIndex],
					"revisions": {
						"1": {
							"path": "plop" + ids[currentIndex],
							"user": "mrducky"
						}
					}
				}
				yield doc
		
		# blocking
		# return True on success, False on failure
		def download(self, assetId, revision, filename):
			pass
		
		# blocking
		# return True on success, False on failure
		def upload(self, assetId, revision, filename):
			pass
	
	class FakeService:
		"Emulate client actions"
		def __init__(self, context):
			self.socket = context.socket(zmq.PUB)
			self.socket.bind("inproc://service-actions")
			self.thread = threading.Thread(target = self)
			self.thread.daemon = True
			self.thread.start()
		
		def __call__(self):
			import time
			time.sleep(2)
			self.socket.send_pyobj({
				"type": "create-action",
				"path": "/ploppath"
			})
			
			time.sleep(1)
			self.socket.send_pyobj({
				"type": "delete-action",
				"path": "/ploppath"
			})
	
	context = zmq.Context()
	engine = Engine(context, PlopStorage())
	service = FakeService(context)
	engine.run()

