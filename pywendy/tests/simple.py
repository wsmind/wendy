import pywendy

class TestListener(pywendy.ProjectListener):
	def assetChanged(self, asset):
		print("changed !!!")

listener = TestListener()
project = pywendy.Project(listener)

project.connect()
while project.isConnected():
	project.waitChanges()

print("end!")
