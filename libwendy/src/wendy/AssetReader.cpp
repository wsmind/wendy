#include <wendy/AssetReader.hpp>

#include <wendy/LocalStream.hpp>
#include <wendy/Queue.hpp>
#include <wendy/Thread.hpp>

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
	while (1)
	{
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

