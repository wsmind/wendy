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
#include <sstream>
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
	this->disconnect();
}

void Project::connect()
{
	this->stream = new LocalStream(46288);
	this->stream->connect();
	
	this->reader = new AssetReader(this->stream);
}

void Project::disconnect()
{
	if (this->stream)
		this->stream->disconnect();
	
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
	AssetNotification notification;
	if (this->reader->getNextNotification(&notification))
		this->disconnect();
	else
		this->processNotification(notification);
}

void Project::checkChanges()
{
	// process only available notifications, in order not to block
	while (this->reader->hasNotification())
	{
		AssetNotification notification;
		if (this->reader->getNextNotification(&notification))
			this->disconnect();
		else
			this->processNotification(notification);
	}
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

void Project::openAsset(const std::string &id, OpenMode mode)
{
	this->stream->writeLine("OPEN " + id + " " + ((mode == READING) ? "READING" : "WRITING") + "\n");
}

void Project::closeAsset(unsigned long fd)
{
	std::stringstream ss;
	ss << "CLOSE " << fd << "\n";
	this->stream->writeLine(ss.str());
}

void Project::readAsset(unsigned long fd, unsigned long offset, unsigned long length)
{
	std::stringstream ss;
	ss << "READ " << fd << " " << offset << " " << length << "\n";
	this->stream->writeLine(ss.str());
}

void Project::processNotification(const AssetNotification& notification)
{
	switch (notification.type)
	{
		case AssetNotification::CHANGED:
		{
			this->listener->assetChanged(notification.changed.asset);
			break;
		}
		
		case AssetNotification::OPENED:
		{
			this->listener->assetOpened(notification.opened.id, notification.opened.fd);
			break;
		}
		
		case AssetNotification::CHUNK:
		{
			this->listener->chunkReceived(notification.chunk.fd, notification.chunk.offset, notification.chunk.buffer, notification.chunk.length);
			delete notification.chunk.buffer;
			break;
		}
	}
}

} // wendy namespace

