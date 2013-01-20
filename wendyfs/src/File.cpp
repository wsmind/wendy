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

#include <File.hpp>
#include <wendy/Client.hpp>

#include <cstdio>
#include <cassert>
#include <iostream>

#ifdef _WIN32
	#undef UNICODE
	#include <windows.h>
#endif // _WIN32

File::File(wendy::Client *client, const std::string &path)
{
	this->client = client;
	this->path = path;
	
	this->cacheFile = NULL;
}

File::~File()
{
}

void File::open(bool reading, bool writing, bool truncate)
{
	this->reading = reading;
	this->writing = writing;
	this->truncate = truncate;
	
	// create temp cache file
	this->cacheFilename = this->makeTemporaryFilename();
	this->cacheFile = fopen(this->cacheFilename.c_str(), "w+b");
	
	if (this->cacheFile)
	{
		wendy::RequestState readState;
		CacheFileWriter writer(this->cacheFile);
		client->read(&readState, this->path, &writer);
		client->waitRequest(&readState);
	}
}

bool File::close()
{
	if (this->cacheFile)
	{
		if (this->writing)
		{
			// upload the modified file
			//this->client->save()
		}
		
		// delete temporary file
		fclose(this->cacheFile);
		remove(this->cacheFilename.c_str());
	}
	
	return true;
}

bool File::read(unsigned long offset, void *buffer, unsigned long length)
{
	std::cout << "reading at " << offset << "; length " << length << std::endl;
	if (this->cacheFile)
	{
		fseek(this->cacheFile, offset, SEEK_SET);
		size_t bytesRead = fread(buffer, 1, length, this->cacheFile);
		std::cout << "actually read: " << bytesRead << std::endl;
		//assert(bytesRead == length);
		return true;
	}
	
	return false;
}

bool File::write(unsigned long offset, const void *buffer, unsigned long length)
{
	if (this->cacheFile)
	{
		fseek(this->cacheFile, offset, SEEK_SET);
		fwrite(buffer, 1, length, this->cacheFile);
		return true;
	}
	
	return false;
}

std::string File::makeTemporaryFilename() const
{
	#if defined(_WIN32)
		char tempFolder[MAX_PATH];
		char filename[MAX_PATH];
		GetTempPath(MAX_PATH, tempFolder);
		GetTempFileName(tempFolder, "wendy", 0, filename);
		
		return std::string(filename);
	#else
		assert(!"/tmp/todo");
		return std::string("/tmp/todo");
	#endif
}

void File::CacheFileWriter::writeAssetData(unsigned long long offset, const char *buffer, unsigned long long size)
{
	fseek(this->file, offset, SEEK_SET);
	fwrite(buffer, size, 1, this->file);
}
