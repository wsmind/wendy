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
