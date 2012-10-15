/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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

#ifndef __WENDY_CLIENT_HPP__
#define __WENDY_CLIENT_HPP__

#include <wendy/common.hpp>
#include <wendy/RequestState.hpp>

#include <string>
#include <vector>
#include <list>

namespace wendy {

class HttpEngine;
class Request;

/**
 * \class Client
 */
class WENDYAPI Client
{
	public:
		Client(const std::string &host = "localhost", unsigned short port = 46288);
		~Client();
		
		/**
		 * \brief Non-blocking update
		 *
		 * This call will never block. It will perform any pending internal operation,
		 * if any (nothing otherwise).
		 *
		 * This method is typically called from regularly executed code (e.g a game frame
		 * update).
		 */
		void poll();
		
		/**
		 * \brief Blocking update
		 *
		 * This method is the blocking equivalent of poll(). If there is nothing to do,
		 * it will block until something useful can be performed.
		 *
		 * Use this method when you want to avoid wasting resources and can afford
		 * blocking the current thread.
		 */
		void waitUpdate();
		
		typedef std::vector<std::string> PathList;
		void list(RequestState *state, const std::string &filter, PathList *paths);
		
	private:
		void updateRequests();
		
		HttpEngine *httpEngine;
		
		struct RequestDescriptor
		{
			Request *request;
			RequestState *state;
		};
		std::list<RequestDescriptor> requests;
};

} // wendy namespace

#endif // __WENDY_CLIENT_HPP__
