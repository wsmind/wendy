#include <wendy/AssetReader.hpp>

#include <wendy/LocalStream.hpp>
#include <wendy/Queue.hpp>

namespace wendy {

AssetReader::AssetReader(LocalStream *stream)
{
	this->stream = stream;
}

AssetReader::~AssetReader()
{
}

void AssetReader::run()
{
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

