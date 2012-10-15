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

#include <wendy/ListRequest.hpp>

#include <wendy/HttpEngine.hpp>
#include <wendy/RequestState.hpp>

#include <cJSON.h>

#include <iostream>

namespace wendy {

ListRequest::ListRequest(HttpEngine *httpEngine, const std::string &filter, PathList *pathList)
{
	this->pathList = pathList;
	
	this->request.method = "GET";
	this->request.path = "/list/" + filter;
	this->request.timeoutMilliseconds = 10000;
	this->request.writer = this;
	
	httpEngine->startRequest(&this->request);
}

ListRequest::~ListRequest()
{
}

void ListRequest::update(RequestState *state)
{
	if (request.finished)
	{
		if (request.status == 200)
		{
			this->parsePathList(this->json, this->pathList);
			state->succeed();
		}
		else
		{
			state->fail();
		}
	}
}

void ListRequest::writeHttpData(const char *buffer, unsigned int size)
{
	// accumulate JSON data
	for (unsigned int i = 0; i < size; i++)
		this->json += buffer[i];
}

void ListRequest::parsePathList(const std::string &json, PathList *pathList)
{
	cJSON *root = cJSON_Parse(json.c_str());
	
	for (int i = 0; i < cJSON_GetArraySize(root); i++)
	{
		cJSON *path = cJSON_GetArrayItem(root, i);
		pathList->push_back(path->valuestring);
	}
	
	cJSON_Delete(root);
}

} // wendy namespace
