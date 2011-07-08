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

} // wendy namespace

