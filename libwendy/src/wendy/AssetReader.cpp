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
	delete this->thread;
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
			notification.asset.id = headerLine.substr(spacePos + 1);
			
			if (notificationType == "UPDATED") notification.type = AssetNotification::UPDATED;
			else if (notificationType == "REMOVED") notification.type = AssetNotification::REMOVED;
			else continue;
			
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
				
				if (attribute == "path") notification.asset.path = value;
				else if (attribute == "author") notification.asset.author = value;
				
			}
		}
	}
}

bool AssetReader::hasNotification()
{
	return !this->queue->isEmpty();
}

AssetNotification AssetReader::getNextNotification()
{
	AssetNotification notification = this->queue->receive();
	return notification;
}

} // wendy namespace

