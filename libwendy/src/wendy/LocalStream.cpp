#include <wendy/LocalStream.hpp>

namespace wendy {

LocalStream::LocalStream(unsigned short port)
{
}

LocalStream::~LocalStream()
{
	this->disconnect();
}

bool LocalStream::read(char *buffer, unsigned int size)
{
	return false;
}

bool LocalStream::write(char *buffer, unsigned int size)
{
	return false;
}

void LocalStream::connect()
{
}

void LocalStream::disconnect()
{
}

} // wendy namespace
