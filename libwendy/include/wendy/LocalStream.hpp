#ifndef __WENDY_LOCALSTREAM_HPP__
#define __WENDY_LOCALSTREAM_HPP__

#include <wendy/common.hpp>

// cross-platform socket definitions
#ifdef _WIN32
	#include <winsock.h>
	#define socklen_t int
	#define SOCKET_DATA_TYPE char *
	#define SOCKET_OBJECT_TYPE SOCKET
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <signal.h>
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
		
		bool read(char *buffer, unsigned int size);
		bool write(char *buffer, unsigned int size);
		
	private:
		void connect();
		void disconnect();
		
		sockaddr_in serverAddress;
		SOCKET_OBJECT_TYPE socket;
};

} // wendy namespace

#endif // __WENDY_LOCALSTREAM_HPP__

