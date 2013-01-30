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

#include <wendy/Client.hpp>

#include <wendy/HttpEngine.hpp>
#include <wendy/ListRequest.hpp>
#include <wendy/ReadRequest.hpp>
#include <wendy/SaveRequest.hpp>

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

void Client::waitRequest(RequestState *state)
{
	while (!state->isFinished())
		this->waitUpdate();
}

void Client::list(RequestState *state, const std::string &filter, PathList *paths)
{
	Request *request = new ListRequest(this->httpEngine, filter, paths);
	
	RequestDescriptor descriptor;
	descriptor.request = request;
	descriptor.state = state;
	this->requests.push_back(descriptor);
}

void Client::read(RequestState *state, const std::string &path, AssetWriter *writer)
{
	Request *request = new ReadRequest(this->httpEngine, path, writer);
	
	RequestDescriptor descriptor;
	descriptor.request = request;
	descriptor.state = state;
	this->requests.push_back(descriptor);
}

void Client::save(RequestState *state, const std::string &path, AssetReader *reader)
{
	Request *request = new SaveRequest(this->httpEngine, path, reader);
	
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
		
		// if request is done, destroy it
		if (descriptor.state->isFinished())
		{
			Request *request = descriptor.request;
			
			it = this->requests.erase(it);
			
			delete request;
		}
		else
		{
			it++;
		}
	}
}

} // wendy namespace
