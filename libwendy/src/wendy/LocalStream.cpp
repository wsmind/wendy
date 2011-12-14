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
		
		if (chr != '\n')
			*line += chr;
	}
	
	return true;
}

bool LocalStream::readChunk(char *buffer, unsigned long size)
{
	int total = 0;
	while (total < size)
	{
		int read = recv(this->socketId, buffer, size - total, 0);
		if (read <= 0)
			return false;
		
		buffer += read;
		total += read;
	}
	
	return true;
}

bool LocalStream::writeLine(const std::string &line)
{
	for (unsigned int i = 0; i < line.size(); ++i)
	{
		if (send(this->socketId, &line[i], 1, 0) <= 0)
			return false;
	}
	
	return false;
}

bool LocalStream::writeChunk(const char *buffer, unsigned long size)
{
	int total = 0;
	while (total < size)
	{
		int written = send(this->socketId, buffer, size - total, 0);
		if (written <= 0)
			return false;
		
		buffer += written;
		total += written;
	}
	
	return true;
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
