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

import couchdb
import threading
import Queue

class Watcher:
	"""
	Follow the _changes feed of the database, and notifies the
	metadata downloader whenever an asset has changed.
	"""
	
	def __init__(self, db, metadataDownloader):
		self.db = db
		self.metadataDownloader = metadataDownloader
		self.thread = threading.Thread(target = self)
		self.thread.start()
	
	def __call__(self):
		# initial asset list traversal
		for row in self.db.view("_all_docs"):
			self.metadataDownloader.fetchAssetInfo(row.id)
			self.metadataDownloader.fetchAssetInfo(row.id)
		
		# _changes feed
		changes = self.db.changes(feed = "continuous")
		last_seq = 0
		while True:
			for change in changes:
				if "id" in change:
					self.metadataDownloader.fetchAssetInfo(change["id"])
				elif "last_seq" in change:
					last_seq = change["last_seq"]
			
			# when the stream ends, start again
			changes = self.db.changes(feed = "continuous", since = last_seq)

class MetadataListener:
	"""
	Asset notification interface
	"""
	
	def assetChanged(self, assetId, asset):
		pass
	
	def assetRemoved(self, assetId):
		pass

class MetadataDownloader:
	"""
	Fetch asset metadata on demand.
	"""
	
	def __init__(self, db, listener):
		self.db = db
		self.idQueue = Queue.Queue()
		self.listener = listener
		self.thread = threading.Thread(target = self)
		self.thread.start()
	
	def __call__(self):
		while True:
			assetId = self.idQueue.get()
			try:
				asset = self.db[assetId]
				self.listener.assetChanged(assetId, asset)
			except:
				self.listener.assetRemoved(assetId)
	
	def fetchAssetInfo(self, assetId):
		self.idQueue.put(assetId)

class LocalAsset:
	def __init__(self):
		self.id = "0"
		self.path = ""
		self.lock = None
		self.lastRevision = ""
		self.localRevision = ""

class Engine(MetadataListener):
	def __init__(self, server, project):
		self.couch = couchdb.Server(server)
		self.db = self.couch[project]
		self.metadataDownloader = MetadataDownloader(self.db, self)
		self.watcher = Watcher(self.db, self.metadataDownloader)
		self.remoteAssets = {} # dictionary of couchdb.Document
		self.localAssets = {} # dictionary of LocalAsset
		self.listeners = []
	
	def addListener(self, listener):
		self.listeners.append(listener)
	
	def removeListener(self, listener):
		self.listeners.remove(listener)
	
	def assetChanged(self, assetId, asset):
		# check for duplicate notification
		if assetId in self.remoteAssets:
			if self.remoteAssets[assetId].rev == asset.rev:
				return
		
		# TODO: translate to local asset information (+ download info, local rev, etc.)
		self.remoteAssets[asset.id] = asset
		print("plotch")
		print(asset)
		
		for listener in self.listeners:
			listener.assetChanged(assetId, asset)
	
	def assetRemoved(self, assetId):
		if assetId in self.remoteAssets:
			del(self.remoteAssets[assetId])
			
			for listener in self.listeners:
				listener.assetRemoved(assetId)

class EngineListener:
	def assetChanged(self, assetId, asset):
		pass
	
	def assetRemoved(self, assetId):
		pass
