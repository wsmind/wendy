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

#ifndef __WENDY_LISTREQUEST_HPP__
#define __WENDY_LISTREQUEST_HPP__

#include <wendy/common.hpp>

#include <string>
#include <vector>

#include <wendy/Request.hpp>
#include <wendy/HttpRequest.hpp>
#include <wendy/HttpWriter.hpp>

namespace wendy {

class HttpEngine;

struct FileInfo
{
	std::string path;
	std::string hash;
	unsigned long long size;
};
typedef std::vector<FileInfo> PathList;

/**
 * \class ListRequest
 */
class WENDYAPI ListRequest: public Request, public HttpWriter
{
	public:
		ListRequest(HttpEngine *httpEngine, const std::string &filter, PathList *pathList);
		virtual ~ListRequest();
		
		virtual void update(RequestState *state);
		
		virtual void writeHttpData(const char *buffer, unsigned int size);
	
	private:
		// extract path list from JSON data
		bool parsePathList(const std::string &json, PathList *pathList);
		
		HttpRequest request;
		std::string json;
		
		// result
		PathList *pathList;
};

} // wendy namespace

#endif // __WENDY_LISTREQUEST_HPP__
