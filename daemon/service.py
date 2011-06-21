import threading
import socket

import engine

class Server:
	def __init__(self, engine):
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.bind(("", 46288))
		self.socket.listen(10)
		self.engine = engine
		self.clients = []
	
	def waitConnections(self):
		while True:
			(clientSocket, clientAddress) = self.socket.accept()
			self.clients.append(ClientHandler(clientSocket))

class ClientHandler:
	def __init__(self, socket):
		self.socket = socket
		self.socket.send("HELLOOOO\n")
		#print(self.socket.recv())

