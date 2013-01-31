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

#include <wendy/HttpEngine.hpp>

#include <curl/curl.h>

#include <wendy/HttpRequest.hpp>
#include <wendy/HttpReader.hpp>
#include <wendy/HttpWriter.hpp>

#include <sstream>
#include <iostream> // temp debug

namespace wendy {

unsigned int HttpEngine::instanceCount = 0;

HttpEngine::HttpEngine(const std::string &host, unsigned short port)
{
	if (HttpEngine::instanceCount == 0)
	{
		// first instance, we must initialize curl
		curl_global_init(CURL_GLOBAL_ALL);
	}
	HttpEngine::instanceCount++;
	
	this->host = host;
	this->port = port;
	
	this->curlMulti = curl_multi_init();
}

HttpEngine::~HttpEngine()
{
	curl_multi_cleanup(this->curlMulti);
	
	HttpEngine::instanceCount--;
	if (HttpEngine::instanceCount == 0)
	{
		// this was the last instance, we must release curl
		curl_global_cleanup();
	}
}

void HttpEngine::poll()
{
	int runningHandles;
	curl_multi_perform(this->curlMulti, &runningHandles);
	
	// check for finished requests
	this->pollCurlMessages();
}

void HttpEngine::waitUpdate()
{
	// maximum wait time, according to the timeouts configured on the easy handles
	long timeoutMilliseconds;
	curl_multi_timeout(this->curlMulti, &timeoutMilliseconds);
	
	// actual socket file requests
	fd_set readFdSet;
	fd_set writeFdSet;
	fd_set exceptionFdSet;
	int fdCount;
	FD_ZERO(&readFdSet);
	FD_ZERO(&writeFdSet);
	FD_ZERO(&exceptionFdSet);
	curl_multi_fdset(this->curlMulti, &readFdSet, &writeFdSet, &exceptionFdSet, &fdCount);
	
	// blocking wait on the soockets
	timeval maxWait;
	maxWait.tv_sec = timeoutMilliseconds / 1000;
	maxWait.tv_usec = (timeoutMilliseconds % 1000) * 1000;
	select(fdCount, &readFdSet, &writeFdSet, &exceptionFdSet, &maxWait);
	
	// curl should now find something interesting to do
	int runningHandles;
	curl_multi_perform(this->curlMulti, &runningHandles);
	
	// check for finished requests
	this->pollCurlMessages();
}

void HttpEngine::startRequest(HttpRequest *request)
{
	// create new curl request
	request->curlHandle = curl_easy_init();
	
	// save the HttpRequest in the curl handle for future retrieval
	curl_easy_setopt(request->curlHandle, CURLOPT_PRIVATE, request);
	
	// build complete url
	std::stringstream url;
	url << "http://" << this->host << ":" << this->port << request->path;
	curl_easy_setopt(request->curlHandle, CURLOPT_URL, url.str().c_str());
	std::cout << "starting " << request->method << " " << url.str() << std::endl;
	
	// method
	if (request->method == "GET") /*default*/;
	else if (request->method == "PUT") curl_easy_setopt(request->curlHandle, CURLOPT_UPLOAD, 1);
	else if (request->method == "POST") curl_easy_setopt(request->curlHandle, CURLOPT_POST, 1);
	
	// timeout
	curl_easy_setopt(request->curlHandle, CURLOPT_TIMEOUT_MS, request->timeoutMilliseconds);
	
	// read callback
	curl_easy_setopt(request->curlHandle, CURLOPT_READFUNCTION, HttpEngine::readCallback);
	curl_easy_setopt(request->curlHandle, CURLOPT_READDATA, request);
	
	// write callback
	curl_easy_setopt(request->curlHandle, CURLOPT_WRITEFUNCTION, HttpEngine::writeCallback);
	curl_easy_setopt(request->curlHandle, CURLOPT_WRITEDATA, request);
	
	// start executing the new request
	curl_multi_add_handle(this->curlMulti, request->curlHandle);
}

size_t HttpEngine::readCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	HttpRequest *request = (HttpRequest *)userdata;
	
	// if no reader is set, we cannot send anything
	if (!request->reader)
		return 0;
	
	return request->reader->readHttpData((char *)ptr, size * nmemb);
}

size_t HttpEngine::writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	HttpRequest *request = (HttpRequest *)userdata;
	
	// if no writer is set, the data is lost
	if (request->writer)
		request->writer->writeHttpData(ptr, size * nmemb);
	
	return size * nmemb;
}

void HttpEngine::pollCurlMessages()
{
	CURLMsg *message;
	int remainingMessageCount;
	while ((message = curl_multi_info_read(this->curlMulti, &remainingMessageCount)))
	{
		if (message->msg == CURLMSG_DONE)
		{
			// retrieve associated request
			HttpRequest *request;
			curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &request);
			
			// signal request termination
			request->finished = true;
			
			// expose the resulting status code
			curl_easy_getinfo(message->easy_handle, CURLINFO_RESPONSE_CODE, &request->status);
			
			// detach the finished request
			curl_multi_remove_handle(this->curlMulti, request->curlHandle);
		}
	}
}

} // wendy namespace
