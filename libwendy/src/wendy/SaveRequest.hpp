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

#ifndef __WENDY_SAVEREQUEST_HPP__
#define __WENDY_SAVEREQUEST_HPP__

#include <wendy/common.hpp>

#include <string>

#include <wendy/Request.hpp>
#include <wendy/HttpRequest.hpp>
#include <wendy/HttpReader.hpp>

namespace wendy {

class AssetReader;
class HttpEngine;

/**
 * \class SaveRequest
 */
class WENDYAPI SaveRequest: public Request, public HttpReader
{
	public:
		SaveRequest(HttpEngine *httpEngine, const std::string &path, AssetReader *reader);
		virtual ~SaveRequest();
		
		virtual void update(RequestState *state);
		
		virtual unsigned int readHttpData(char *buffer, unsigned int size);
	
	private:
		HttpRequest request;
		
		// current writing offset
		unsigned long long offset;
		
		// client handler
		AssetReader *reader;
};

} // wendy namespace

#endif // __WENDY_SAVEREQUEST_HPP__
