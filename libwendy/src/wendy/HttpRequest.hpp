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

#ifndef __WENDY_HTTPREQUEST_HPP__
#define __WENDY_HTTPREQUEST_HPP__

#include <wendy/common.hpp>

#include <string>

namespace wendy {

class HttpWriter;

/**
 * \struct HttpRequest
 */
struct WENDYAPI HttpRequest
{
	// input parameters
	std::string method;
	std::string path;
	long timeoutMilliseconds;
	HttpWriter *writer;
	
	/// termination state, will be set to true when the request finishes
	bool finished;
	
	/// output status (set after the request has finished)
	/// will be 0 if the request is not finished, or if it was aborted (e.g timeout)
	long status;
	
	/// \internal curl handle associated to this request, will be setup by the http engine
	void *curlHandle;
	
	/**
	 * \brief Constructor
	 * Initialize all members to a reasonable default.
	 */
	HttpRequest()
	{
		this->method = "GET";
		this->path = "/";
		this->writer = NULL;
		
		this->finished = false;
		this->status = 0;
		
		this->curlHandle = NULL;
	}
};

} // wendy namespace

#endif // __WENDY_HTTPREQUEST_HPP__
