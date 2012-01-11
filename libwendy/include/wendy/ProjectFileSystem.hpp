/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
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

#ifndef __WENDY_PROJECTFILESYSTEM_HPP__
#define __WENDY_PROJECTFILESYSTEM_HPP__

#include <wendy/common.hpp>

#include <string>
#include <vector>
#include <map>

#include <wendy/ProjectListener.hpp>

namespace wendy {

struct Asset;
class FileSystemNode;
class Project;

/**
 * \class ProjectFileSystem
 * \brief Synchronous filesystem-like project wrapper (*not* thread-safe)
 */
class WENDYAPI ProjectFileSystem: public ProjectListener
{
	public:
		ProjectFileSystem();
		~ProjectFileSystem();
		
		struct FileAttributes
		{
			bool folder;
		};
		bool stat(const std::string &path, FileAttributes *attributes);
		
		enum OpenMode
		{
			READING,
			WRITING,
		};
		std::string open(const std::string &path, OpenMode mode);
		
		void close(const std::string &id);
		
		bool mkdir(const std::string &path);
		
		bool rmdir(const std::string &path);
		
		bool readdir(const std::string &path, std::vector<std::string> *items);
		
		virtual void assetChanged(const Asset &asset);
		
	private:
		FileSystemNode *root;
		Project *project;
		
		typedef std::map<std::string, Asset> AssetMap;
		AssetMap assets;
};

} // wendy namespace

#endif // __WENDY_PROJECTFILESYSTEM_HPP__