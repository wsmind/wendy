#include <wendy/LocalStream.hpp>

#include <cstring>
#include <string>

namespace wendy {

LocalStream::LocalStream(unsigned short port)
{
#	ifdef _WIN32
		WSAData wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
#	endif
	
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

bool LocalStream::readLine(std::string *line)
{
	*line = "";
	
	char chr = 0;
	while (chr != '\n')
	{
		if (recv(this->socketId, &chr, 1, 0) <= 0)
			return false;
		
		// ignore '\r' for cross-platform line endings
		if (chr == '\r')
			continue;
		
		if (chr != '\n')
			*line += chr;
	}
	
	return true;
}

bool LocalStream::writeLine(const std::string &line)
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
#	ifdef _WIN32
		shutdown(this->socketId, SD_BOTH);
#	else
		shutdown(this->socketId, SHUT_RDWR);
#	endif
	
	this->connected = false;
}

} // wendy namespace
