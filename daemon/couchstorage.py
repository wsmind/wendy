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
import engine

class CouchDBStorage(engine.Storage):
	
	def __init__(self, server, project):
		self.couch = couchdb.Server(server)
		self.db = self.couch[project]
	
	# blocking generator
	def pollChanges(self):
		# initial asset list traversal
		for row in self.db.view("_all_docs"):
			yield self.db[row.id]
		
		# _changes feed
		changes = self.db.changes(feed = "continuous")
		last_seq = 0
		while True:
			for change in changes:
				if "id" in change:
					assetId = change["id"]
					yield self.db[assetId]
				elif "last_seq" in change:
					last_seq = change["last_seq"]
			
			# when the stream ends, start again
			changes = self.db.changes(feed = "continuous", since = last_seq)
	
	# blocking
	# return True on success, False on failure
	def download(self, assetId, revision, filename):
		asset = self.db[assetId]
		attachementFilename = asset["revisions"][str(revision)]["file"]
		attachement = self.db.get_attachement(assetId, attachementFilename)
		
		output = open(filename)
		output.write(attachement)
		output.close()
		
		attachement.close()
		
		return True
	
	# blocking
	# return True on success, False on failure
	def upload(self, assetId, revision, filename):
		return False

# this test requires the server assets.emp.fr.nf to host a wendy/couch database named "plop"
if __name__ == "__main__":
	storage = CouchDBStorage("http://assets.emp.fr.nf", "plop")
	for asset in storage.pollChanges():
		print(asset)
	
	storage.download("sesegseg", 1, "ploptest")

