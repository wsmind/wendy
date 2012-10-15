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

#include <iostream>
#include <wendy/HttpEngine.hpp>
#include <wendy/HttpRequest.hpp>
#include <wendy/HttpWriter.hpp>

class TestWriter: public wendy::HttpWriter
{
	public:
		std::string content;
		
		virtual ~TestWriter() {}
		
		virtual void writeHttpData(const char *buffer, unsigned int size)
		{
			std::cout << "received " << size << " bytes!" << std::endl;
			
			for (unsigned int i = 0; i < size; i++)
				content += buffer[i];
		}
};

int main()
{
	wendy::HttpEngine *httpEngine = new wendy::HttpEngine("www.google.com", 80);
	
	TestWriter *writer = new TestWriter;
	wendy::HttpRequest request;
	request.method = "GET";
	request.path = "/";
	request.timeoutMilliseconds = 5000;
	request.writer = writer;
	httpEngine->startRequest(&request);
	
	while (!request.finished)
		httpEngine->waitUpdate();
	
	std::cout << "request finished with status " << request.status << std::endl;
	
	std::cout << "content received:" << std::endl;
	std::cout << writer->content;
	
	delete writer;
	delete httpEngine;
	
	return 0;
}
