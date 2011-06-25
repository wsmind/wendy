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

