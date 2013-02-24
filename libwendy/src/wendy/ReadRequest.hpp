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

#ifndef __WENDY_READREQUEST_HPP__
#define __WENDY_READREQUEST_HPP__

#include <wendy/common.hpp>

#include <string>

#include <wendy/Request.hpp>
#include <wendy/HttpRequest.hpp>
#include <wendy/HttpWriter.hpp>

namespace wendy {

class AssetWriter;
class HttpEngine;

/**
 * \class ReadRequest
 */
class WENDYAPI ReadRequest: public Request, public HttpWriter
{
	public:
		ReadRequest(HttpEngine *httpEngine, const std::string &path, AssetWriter *writer, const std::string &version = "", unsigned long long rangeStart = 0, unsigned long long rangeEnd = 0);
		virtual ~ReadRequest();
		
		virtual void update(RequestState *state);
		
		virtual void writeHttpData(const char *buffer, unsigned int size);
	
	private:
		HttpRequest request;
		
		// current writing offset
		unsigned long long offset;
		
		// client handler
		AssetWriter *writer;
};

} // wendy namespace

#endif // __WENDY_READREQUEST_HPP__
