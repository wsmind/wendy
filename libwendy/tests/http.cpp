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

#include <iostream>
#include <wendy/HttpEngine.hpp>
#include <wendy/HttpRequest.hpp>
#include <wendy/HttpReader.hpp>
#include <wendy/HttpWriter.hpp>

class TestReader: public wendy::HttpReader
{
	public:
		unsigned long long currentOffset;
		std::string text;
		
		TestReader()
			: currentOffset(0)
			, text("{\"title\": \"this is simple JSON!\"}")
		{}
		
		virtual ~TestReader() {}
		
		virtual unsigned int readHttpData(char *buffer, unsigned int size)
		{
			unsigned int read = 0;
			while ((currentOffset < text.size()) && (size > 0))
			{
				buffer[read] = text[currentOffset];
				currentOffset++;
				read++;
				size--;
			}
			
			return read;
		}
};

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
	wendy::HttpEngine *httpEngine = new wendy::HttpEngine("localhost", 5984);
	
	TestReader *reader = new TestReader;
	wendy::HttpRequest request2;
	request2.method = "PUT";
	request2.path = "/plop/http-test";
	request2.timeoutMilliseconds = 5000;
	request2.reader = reader;
	httpEngine->startRequest(&request2);
	
	while (!request2.finished)
		httpEngine->waitUpdate();
	
	std::cout << "request finished with status " << request2.status << std::endl;
	
	delete reader;
	
	TestWriter *writer = new TestWriter;
	wendy::HttpRequest request;
	request.method = "GET";
	request.path = "/plop/http-test";
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
