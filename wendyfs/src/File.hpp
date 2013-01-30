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

#ifndef __WENDY_FILE_HPP__
#define __WENDY_FILE_HPP__

#include <wendy/AssetReader.hpp>
#include <wendy/AssetWriter.hpp>

#include <string>

namespace wendy { class Client; }

/**
 * \class File
 * \brief A virtual file, mapping to a wendy asset
 */
class File: public wendy::AssetWriter
{
	public:
		/**
		 * \brief Constructor
		 * \param path repository-relative path of the asset
		 */
		File(wendy::Client *client, const std::string &path);
		
		~File();
		
		void open(bool reading, bool writing, bool truncate);
		bool close();
		
		bool read(unsigned long offset, void *buffer, unsigned long length);
		bool write(unsigned long offset, const void *buffer, unsigned long length);
		
		const std::string &getPath() const;
		unsigned long long getSize() const;
		
	private:
		std::string makeTemporaryFilename() const;
		
		wendy::Client *client;
		std::string path;
		
		bool reading;
		bool writing;
		bool truncate;
		
		// local temp file
		std::string cacheFilename;
		FILE *cacheFile;
		
		// writer for an actual disk file
		class CacheFileWriter: public wendy::AssetWriter
		{
			public:
				CacheFileWriter(FILE *file): file(file) {}
				virtual ~CacheFileWriter() {}
				
				virtual void writeAssetData(unsigned long long offset, const char *buffer, unsigned long long size);
			
			private:
				FILE *file;
		};
		
		// reader for an actual disk file
		class CacheFileReader: public wendy::AssetReader
		{
			public:
				CacheFileReader(FILE *file): file(file) {}
				virtual ~CacheFileReader() {}
				
				virtual unsigned long long readAssetData(unsigned long long offset, char *buffer, unsigned long long size);
			
			private:
				FILE *file;
		};
};

#endif //  __WENDY_FILE_HPP__
