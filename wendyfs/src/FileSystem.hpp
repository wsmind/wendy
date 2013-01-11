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

#ifndef __WENDY_FILESYSTEM_HPP__
#define __WENDY_FILESYSTEM_HPP__

#include <string>
#include <vector>
#include <map>

class FileSystemNode;

namespace wendy { class Client; }

/**
 * \class FileSystem
 * \brief Synchronous filesystem-like project wrapper (*not* thread-safe)
 */
class FileSystem
{
	public:
		FileSystem();
		~FileSystem();
		
		struct FileAttributes
		{
			bool folder;
			unsigned long long date;
			unsigned long long length;
		};
		bool stat(const std::string &path, FileAttributes *attributes);
		
		bool unlink(const std::string &path);
		
		enum OpenMode
		{
			READING,
			WRITING,
		};
		long open(const std::string &path, OpenMode mode, const std::string &applicationName);
		
		void close(long fd);
		
		bool read(long fd, unsigned long offset, void *buffer, unsigned long length);
		
		bool mkdir(const std::string &path);
		
		bool rmdir(const std::string &path);
		
		bool readdir(const std::string &path, std::vector<std::string> *items);
		
	private:
		FileSystemNode *root;
		wendy::Client *client;
};

#endif // __WENDY_FILESYSTEM_HPP__
