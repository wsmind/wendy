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

#include <wendy/Client.hpp>

#include <wendy/HttpEngine.hpp>
#include <wendy/ListRequest.hpp>

namespace wendy {

Client::Client(const std::string &host, unsigned short port)
{
	this->httpEngine = new HttpEngine(host, port);
}

Client::~Client()
{
	delete this->httpEngine;
}

void Client::poll()
{
	this->httpEngine->poll();
	this->updateRequests();
}

void Client::waitUpdate()
{
	this->httpEngine->waitUpdate();
	this->updateRequests();
}

void Client::list(RequestState *state, const std::string &filter, PathList *paths)
{
	Request *request = new ListRequest(this->httpEngine, filter, paths);
	
	RequestDescriptor descriptor;
	descriptor.request = request;
	descriptor.state = state;
	this->requests.push_back(descriptor);
}

void Client::updateRequests()
{
	std::list<RequestDescriptor>::iterator it = this->requests.begin();
	while (it != this->requests.end())
	{
		RequestDescriptor &descriptor = *it;
		descriptor.request->update(descriptor.state);
		it++;
	}
}

} // wendy namespace
