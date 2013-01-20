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
#include <cassert>
#include <wendy/AssetWriter.hpp>
#include <wendy/Client.hpp>
#include <wendy/RequestState.hpp>

class TestWriter: public wendy::AssetWriter
{
	public:
		virtual ~TestWriter() {}
		
		virtual void writeAssetData(unsigned long long offset, const char *buffer, unsigned long long size)
		{
			for (unsigned long long i = 0; i < size; i++)
				std::cout << buffer[i];
		}
};

int main()
{
	wendy::Client *client = new wendy::Client;
	
	wendy::RequestState listState;
	wendy::Client::PathList files;
	client->list(&listState, "*", &files);
	client->waitRequest(&listState);
	
	if (listState.isSuccess())
	{
		for (unsigned int i = 0; i < files.size(); i++)
			std::cout << "Found file: " << files[i] << std::endl;
	}
	
	wendy::RequestState listState2;
	wendy::Client::PathList files2;
	client->list(&listState2, "non-existent-file", &files2);
	client->waitRequest(&listState2);
	
	if (listState2.isSuccess())
	{
		for (unsigned int i = 0; i < files2.size(); i++)
			std::cout << "Found file: " << files2[i] << std::endl;
	}
	
	wendy::RequestState listState3;
	wendy::Client::PathList files3;
	client->list(&listState3, "lapins/**", &files3);
	client->waitRequest(&listState3);
	
	if (listState3.isSuccess())
	{
		for (unsigned int i = 0; i < files3.size(); i++)
			std::cout << "Found file: " << files3[i] << std::endl;
	}
	
	wendy::RequestState readState;
	TestWriter writer;
	client->read(&readState, "engine.js", &writer);
	client->waitRequest(&readState);
	
	assert(readState.isSuccess());
	
	delete client;
	
	return 0;
}
