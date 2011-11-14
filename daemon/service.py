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
import socket
import zmq

class Server():
	def __init__(self, context):
		self.context = context
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.bind(("", 46288))
		self.socket.listen(10)
		self.clients = []
		self.thread = threading.Thread(target = self)
		self.thread.daemon = True
	
	def __call__(self):
		while True:
			(clientSocket, clientAddress) = self.socket.accept()
			self.clients.append(ClientHandler(clientSocket, self.context))
	
	def waitConnections(self):
		self.thread.start()

class ClientHandler:
	def __init__(self, clientSocket, context):
		self.clientStream = clientSocket.makefile()
		self.actionSocket = self.socket = context.socket(zmq.PUB)
		self.actionSocket.bind("inproc://service-actions")
		self.thread = threading.Thread(target = self)
		self.thread.start()
		
		# initial dump
		#self.engine.dumpAssets(self)
	
	def __call__(self):
		buf = self.clientStream.readline()
		while buf:
			print(buf)
			spacePos = buf.find(" ")
			if spacePos != -1:
				command = buf[0:spacePos]
				arg = buf[spacePos:].strip()
				print("command = " + command)
				print("arg = " + arg)
				
				if command == "CREATE":
					self.actionSocket.send_pyobj({
						"type": "create-action",
						"path": arg
					})
				
			buf = self.clientStream.readline()
	
	#def assetChanged(self, assetId, asset):
	#	self.clientStream.write("UPDATED " + str(assetId) + "\n")
	#	self.clientStream.write("path " + asset["path"] + "\n")
	#	self.clientStream.write("END\n")
	#	self.clientStream.flush() # send this now
	
	#def assetRemoved(self, assetId):
	#	self.clientStream.write("REMOVED " + str(assetId) + "\n")
	#	self.clientStream.write("END\n")
	#	self.clientStream.flush() # send this now

