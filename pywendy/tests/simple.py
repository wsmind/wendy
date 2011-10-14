import pywendy

class TestListener(pywendy.ProjectListener):
	def assetAdded(self, *args):
		print("added !!!")

listener = TestListener()
project = pywendy.Project(listener)

project.connect()
while project.isConnected():
	project.waitChanges()

print("end!")

