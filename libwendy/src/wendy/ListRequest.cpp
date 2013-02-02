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

#include <wendy/ListRequest.hpp>

#include <wendy/HttpEngine.hpp>
#include <wendy/RequestState.hpp>

#include <cassert>
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
	if (this->request.finished)
	{
		if (this->request.status == 200)
		{
			if (this->parsePathList(this->json, this->pathList))
				state->succeed();
			else
				state->fail();
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

bool ListRequest::parsePathList(const std::string &json, PathList *pathList)
{
	cJSON *root = cJSON_Parse(json.c_str());
	if (!root)
		return false;
	
	cJSON *asset = root->child;
	while (asset)
	{
		cJSON *hash = cJSON_GetObjectItem(asset, "hash");
		cJSON *size = cJSON_GetObjectItem(asset, "size");
		
		if (hash && size)
		{
			FileInfo info;
			info.path = std::string(asset->string);
			info.hash = std::string(hash->valuestring);
			info.size = size->valueint;
			pathList->push_back(info);
		}
		
		asset = asset->next;
	}
	
	cJSON_Delete(root);
	
	return true;
}

} // wendy namespace
