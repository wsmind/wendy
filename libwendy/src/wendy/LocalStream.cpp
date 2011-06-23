#include <wendy/LocalStream.hpp>

#include <cstring>

namespace wendy {

LocalStream::LocalStream(unsigned short port)
{
	this->serverAddress.sin_family = AF_INET;
	this->serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	this->serverAddress.sin_port = htons(port);
	memset(this->serverAddress.sin_zero, 0, sizeof(this->serverAddress.sin_zero));
	
	this->socketId = socket(AF_INET, SOCK_STREAM, 0);
	
	this->connected = false;
	
	// try to connect
	this->connect();
}

LocalStream::~LocalStream()
{
	this->disconnect();
	
#	ifdef _WIN32
		closesocket(this->socketId);
#	else
		close(this->socketId);
#	endif
}

bool LocalStream::isConnected()
{
	return this->connected;
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
	if (::connect(this->socketId, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) != -1)
		this->connected = true;
}

void LocalStream::disconnect()
{
	shutdown(this->socketId, SHUT_RDWR);
	this->connected = false;
}

} // wendy namespace
