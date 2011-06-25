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

class Engine(MetadataListener):
	def __init__(self, server, project):
		self.couch = couchdb.Server(server)
		self.db = self.couch[project]
		self.metadataDownloader = MetadataDownloader(self.db, self)
		self.watcher = Watcher(self.db, self.metadataDownloader)
		self.assets = {} # dictionary of couchdb.Document; TODO: no, add local info
		self.listeners = []
	
	def addListener(self, listener):
		self.listeners.append(listener)
	
	def removeListener(self, listener):
		self.listeners.remove(listener)
	
	def assetChanged(self, assetId, asset):
		# check for duplicate notification
		if assetId in self.assets:
			if self.assets[assetId].rev == asset.rev:
				return
		
		# TODO: translate to local asset information (+ download info, local rev, etc.)
		self.assets[asset.id] = asset
		print("plotch")
		print(asset)
		
		for listener in self.listeners:
			listener.assetChanged(assetId, asset)
	
	def assetRemoved(self, assetId):
		if assetId in self.assets:
			del(self.assets[assetId])
			
			for listener in self.listeners:
				listener.assetRemoved(assetId)

class EngineListener:
	def assetChanged(self, assetId, asset):
		pass
	
	def assetRemoved(self, assetId):
		pass
