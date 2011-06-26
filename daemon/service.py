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

import engine

class Server(engine.EngineListener):
	def __init__(self, engine):
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.bind(("", 46288))
		self.socket.listen(10)
		self.engine = engine
		self.clients = []
		
		self.engine.addListener(self)
	
	def waitConnections(self):
		while True:
			(clientSocket, clientAddress) = self.socket.accept()
			self.clients.append(ClientHandler(clientSocket))
	
	def assetChanged(self, assetId, asset):
		print("Asset " + assetId + " changed !!!")
	
	def assetRemoved(self, assetId):
		print("Asset " + assetId + " removed !!!")

class ClientHandler:
	def __init__(self, socket):
		self.socket = socket
		self.socket.send("HELLOOOO\n")
		self.thread = threading.Thread(target = self)
		self.thread.start()
	
	def __call__(self):
		buf = self.socket.recv(100)
		while buf:
			print(buf)
			buf = self.socket.recv(100)

