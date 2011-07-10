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

const Asset Project::getAsset(const std::string &id)
{
	AssetMap::iterator it = this->assets.find(id);
	if (it != this->assets.end())
		return it->second;
	
	return Asset();
}

void Project::processNotification(const AssetNotification& notification)
{
	std::cout << "asset " << notification.asset.id << " got notification " << notification.type << std::endl;
	
	std::string id = notification.asset.id;
	bool existing = (this->assets.find(id) != this->assets.end());
	
	switch (notification.type)
	{
		case AssetNotification::UPDATED:
		{
			this->assets[id] = notification.asset;
			
			if (!existing)
				this->listener->assetAdded(this, notification.asset);
			else
				this->listener->assetUpdated(this, notification.asset);
			
			break;
		}
		
		case AssetNotification::REMOVED:
		{
			if (!existing)
				break;
			
			this->listener->assetRemoved(this, this->assets[id]);
			this->assets.erase(id);
			break;
		}
	}
}

} // wendy namespace

