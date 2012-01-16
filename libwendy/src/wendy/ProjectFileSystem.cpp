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

#include <wendy/ProjectFileSystem.hpp>

#include <wendy/Asset.hpp>
#include <wendy/FileSystemNode.hpp>
#include <wendy/Project.hpp>

namespace wendy {

ProjectFileSystem::ProjectFileSystem()
{
	this->root = new FileSystemNode;
	
	this->project = new Project(this);
	this->project->connect();
}

ProjectFileSystem::~ProjectFileSystem()
{
	this->project->disconnect();
	delete this->project;
	
	delete this->root;
}

bool ProjectFileSystem::stat(const std::string &path, FileAttributes *attributes)
{
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false;
	
	attributes->folder = (node->getId() == "");
	
	return true;
}

long ProjectFileSystem::open(const std::string &path, OpenMode mode)
{
	this->project->checkChanges();
	
	// check if file exists
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return -1;
	
	const std::string &id = node->getId();
	
	// check if directory
	if (id == "")
		return -1;
	
	this->project->openAsset(id, (mode == READING) ? Project::READING : Project::WRITING);
	
	return -1;
}

void ProjectFileSystem::close(long fd)
{
	this->project->checkChanges();
	
	if (fd >= 0)
		this->project->closeAsset((unsigned long)fd);
}

bool ProjectFileSystem::mkdir(const std::string &path)
{
	this->project->checkChanges();
	
	FileSystemNode *node = this->root->find(path);
	if (node)
		return false; // already exists
	
	this->root->insert(path, "");
	return true;
}

bool ProjectFileSystem::rmdir(const std::string &path)
{
	this->project->checkChanges();
	
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false; // not found
	
	if (node->getId() != "")
		return false; // not a directory
	
	if (!node->isEmpty())
		return false; // not empty
	
	this->root->remove(path);
	
	return true;
}

bool ProjectFileSystem::readdir(const std::string &path, std::vector<std::string> *items)
{
	this->project->checkChanges();
	
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false; // not found
	
	if (node->getId() != "")
		return false; // not a directory
	
	*items = node->list();
	
	return true;
}

void ProjectFileSystem::assetChanged(const Asset &asset)
{
	AssetMap::iterator it = this->assets.find(asset.id);
	if (it != this->assets.end())
	{
		// this asset is already referenced, it needs to be removed first
		Asset &asset = it->second;
		this->root->remove(asset.path);
	}
	
	// reinsert the new asset into the hierarchy, if it was not deleted
	if (asset.path != "")
		this->root->insert(asset.path, asset.id);
	
	// updated saved metadata
	this->assets[asset.id] = asset;
}

void ProjectFileSystem::assetOpened(const std::string &id, unsigned long fd)
{
}

} // wendy namespace
