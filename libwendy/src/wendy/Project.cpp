#include <wendy/Project.hpp>

#include <iostream>
#include <wendy/AssetReader.hpp>
#include <wendy/AssetNotification.hpp>
#include <wendy/LocalStream.hpp>
#include <wendy/ProjectListener.hpp>

namespace wendy {

Project::Project(ProjectListener *listener)
{
	this->listener = listener;
	this->stream = NULL;
	this->reader = NULL;
}

Project::~Project()
{
	delete this->stream;
}

void Project::connect()
{
	this->stream = new LocalStream(46288);
	this->reader = new AssetReader(this->stream);
}

void Project::disconnect()
{
	delete this->reader;
	this->reader = NULL;
	
	delete this->stream;
	this->stream = NULL;
}

bool Project::isConnected()
{
	return this->stream && this->stream->isConnected();
}

void Project::waitChanges()
{
	AssetNotification notification = this->reader->getNextNotification();
	this->processNotification(notification);
}

void Project::checkChanges()
{
	// process only available notifications, in order not to block
	while (this->reader->hasNotification())
	{
		AssetNotification notification = this->reader->getNextNotification();
		this->processNotification(notification);
	}
}

void Project::plop()
{
	std::string line;
	while (this->stream->readLine(&line))
	{
		std::cout << "line: " << line << std::endl;
		std::cin >> line;
		this->stream->writeLine(line);
	}
}

void Project::processNotification(const AssetNotification& notification)
{
	std::cout << "asset " << notification.asset.id << " got notification!!" << std::endl;
}

} // wendy namespace

