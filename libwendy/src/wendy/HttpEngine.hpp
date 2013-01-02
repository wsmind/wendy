/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2013 Remi Papillie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 *****************************************************************************/

#ifndef __WENDY_HTTPENGINE_HPP__
#define __WENDY_HTTPENGINE_HPP__

#include <wendy/common.hpp>

#include <string>

namespace wendy {

class HttpRequest;

/**
 * \class HttpEngine
 */
class WENDYAPI HttpEngine
{
	public:
		/**
		 * \brief Constructor
		 *
		 * The host and port provided here will apply to all requests made
		 * through this engine.
		 */
		HttpEngine(const std::string &host, unsigned short port);
		
		~HttpEngine();
		
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
		
		void startRequest(HttpRequest *request);
		
	private:
		// curl write callback
		static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
		
		// check for finished requests
		void pollCurlMessages();
		
		// for global curl init/cleanup
		static unsigned int instanceCount;
		
		std::string host;
		unsigned short port;
		
		// using void avoids propagating curl headers to libwendy users
		/* CURLM */ void *curlMulti;
};

} // wendy namespace

#endif // __WENDY_HTTPENGINE_HPP__
