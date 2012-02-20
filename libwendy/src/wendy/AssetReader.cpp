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

#include <wendy/AssetReader.hpp>

#include <wendy/LocalStream.hpp>
#include <wendy/Queue.hpp>
#include <wendy/Thread.hpp>

#include <iostream>
#include <sstream>

// standard conversions workaround
#ifdef WIN32
#	define atoll _atoi64
#endif

namespace wendy {

AssetReader::AssetReader(LocalStream *stream)
{
	this->stream = stream;
	this->queue = new Queue<AssetNotification>;
	
	// start asynchronous read
	this->thread = new Thread(this);
}

AssetReader::~AssetReader()
{
	delete this->thread; // will join before destructing the thread
	delete this->queue;
}

void AssetReader::run()
{
	while (this->stream->isConnected())
	{
		std::string headerLine;
		AssetNotification notification;
		
		if (this->stream->readLine(&headerLine))
		{
			// extract notification type and asset ID
			size_t spacePos = headerLine.find(' ');
			std::string notificationType = headerLine.substr(0, spacePos);
			std::string notificationParams = headerLine.substr(spacePos + 1);
			
			if (notificationType == "ASSET")
			{
				notification.type = AssetNotification::CHANGED;
				notification.changed.asset.id = notificationParams;
				
				// initialization
				notification.changed.asset.rights = 0;
				notification.changed.asset.date = 0;
				notification.changed.asset.length = 0;
				
				// extract asset attributes
				std::string attributeLine;
				while (this->stream->readLine(&attributeLine))
				{
					if (attributeLine == "END")
					{
						// notification is complete
						// send it though the queue
						this->queue->send(notification);
						
						break;
					}
					
					// read attribute
					size_t separatorPos = attributeLine.find(' ');
					std::string attribute = attributeLine.substr(0, separatorPos);
					std::string value = attributeLine.substr(separatorPos + 1);
					
					if (attribute == "revision") notification.changed.asset.revision = value;
					else if (attribute == "author") notification.changed.asset.author = value;
					else if (attribute == "rights")
					{
						if (value.find("r") != std::string::npos) notification.changed.asset.rights |= Asset::READ_ACCESS;
						if (value.find("w") != std::string::npos) notification.changed.asset.rights |= Asset::WRITE_ACCESS;
					}
					else if (attribute == "date") notification.changed.asset.date = atoll(value.c_str());
					else if (attribute == "path") notification.changed.asset.path = value;
					else if (attribute == "type") notification.changed.asset.type = value;
					else if (attribute == "length") notification.changed.asset.length = atoll(value.c_str());
					/*else if (attribute == "state")
					{
						notification.changed.asset.state = Asset::MISSING;
						if (value == "DOWNLOADING") notification.changed.asset.state = Asset::DOWNLOADING;
						else if (value == "CACHED") notification.changed.asset.state = Asset::CACHED;
					}*/
					else if (attribute == "lockUser") notification.changed.asset.lock.user = value;
					else if (attribute == "lockApp") notification.changed.asset.lock.application = value;
				}
			}
			
			else if (notificationType == "OPENED")
			{
				notification.type = AssetNotification::OPENED;
				
				std::stringstream ss(notificationParams);
				ss >> notification.opened.id >> notification.opened.mode >> notification.opened.fd;
				
				this->queue->send(notification);
			}
			
			else if (notificationType == "CHUNK")
			{
				notification.type = AssetNotification::CHUNK;
				
				std::stringstream ss(notificationParams);
				ss >> notification.chunk.fd >> notification.chunk.offset >> notification.chunk.length;
				
				notification.chunk.buffer = new char[notification.chunk.length];
				this->stream->readChunk(notification.chunk.buffer, notification.chunk.length);
				
				this->queue->send(notification);
			}
		}
	}
	
	// the stream was disconnected
	this->queue->sendTermination();
}

bool AssetReader::hasNotification()
{
	return !this->queue->isEmpty();
}

bool AssetReader::getNextNotification(AssetNotification *notification)
{
	return this->queue->receive(notification);
}

} // wendy namespace
