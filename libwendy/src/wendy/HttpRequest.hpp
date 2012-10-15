/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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
