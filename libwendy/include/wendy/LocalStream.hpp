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

#ifndef __WENDY_LOCALSTREAM_HPP__
#define __WENDY_LOCALSTREAM_HPP__

#include <wendy/common.hpp>
#include <string>

// cross-platform socket definitions
#ifdef _WIN32
	#include <winsock2.h>
	#define socklen_t int
	#define SOCKET_DATA_TYPE char *
	#define SOCKET_OBJECT_TYPE SOCKET
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <signal.h>
	#include <unistd.h>
	#define SOCKET_DATA_TYPE void *
	#define SOCKET_OBJECT_TYPE int
#endif

namespace wendy {

/**
 * \class LocalStream
 * \brief Simple socket abstraction
 *
 * This class is designed to establish streamed socket connections
 * on localhost only.
 */
class WENDYAPI LocalStream
{
	public:
		LocalStream(unsigned short port);
		~LocalStream();
		
		void connect();
		void disconnect();
		bool isConnected();
		
		bool readLine(std::string *line);
		bool readChunk(char *buffer, unsigned long size);
		bool writeLine(const std::string &line);
		bool writeChunk(const char *buffer, unsigned long size);
		
	private:
		bool connected;
		
		sockaddr_in serverAddress;
		SOCKET_OBJECT_TYPE socketId;
};

} // wendy namespace

#endif // __WENDY_LOCALSTREAM_HPP__

