/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

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

void Project::createAsset(const std::string &path)
{
	this->stream->writeLine("CREATE " + path + "\n");
}

void Project::deleteAsset(const std::string &id)
{
	this->stream->writeLine("DELETE " + id + "\n");
}

void Project::lockAsset(const std::string &id, const std::string &applicationName)
{
	// TODO: escape application name
	this->stream->writeLine("LOCK " + id + " " + applicationName + "\n");
}

void Project::unlockAsset(const std::string &id)
{
	this->stream->writeLine("UNLOCK " + id + "\n");
}

AssetFile *Project::openAsset(const std::string &id, AssetFile::OpenMode mode)
{
	return NULL;
}

void Project::closeAsset(AssetFile *file)
{
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
			Asset old = this->assets[id];
			this->assets[id] = notification.asset;
			
			if (!existing)
				this->listener->assetAdded(this, notification.asset);
			else
				this->listener->assetUpdated(this, old, notification.asset);
			
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

