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
		
		bool isConnected();
		
		bool readLine(std::string *line);
		bool writeLine(const std::string &line);
		
	private:
		void connect();
		void disconnect();
		
		bool connected;
		
		sockaddr_in serverAddress;
		SOCKET_OBJECT_TYPE socketId;
};

} // wendy namespace

#endif // __WENDY_LOCALSTREAM_HPP__

