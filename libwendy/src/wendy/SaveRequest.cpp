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

#include <wendy/SaveRequest.hpp>

#include <wendy/AssetReader.hpp>
#include <wendy/HttpEngine.hpp>
#include <wendy/RequestState.hpp>

#include <cassert>

namespace wendy {

SaveRequest::SaveRequest(HttpEngine *httpEngine, const std::string &path, AssetReader *reader)
{
	this->offset = 0;
	this->reader = reader;
	
	this->request.method = "PUT";
	this->request.path = "/data/" + path;
	this->request.timeoutMilliseconds = 10000;
	this->request.reader = this;
	
	httpEngine->startRequest(&this->request);
}

SaveRequest::~SaveRequest()
{
}

void SaveRequest::update(RequestState *state)
{
	if (this->request.finished)
	{
		if (this->request.status == 200)
		{
			state->succeed();
		}
		else
		{
			state->fail();
		}
	}
}

unsigned int SaveRequest::readHttpData(char *buffer, unsigned int size)
{
	// send to client reader
	unsigned long long readBytes = this->reader->readAssetData(this->offset, buffer, size);
	
	// update current offset
	this->offset += readBytes;
	
	return readBytes;
}

} // wendy namespace
