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

#include <iostream>
#include <cstring>

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
	attributes->length = 0;
	
	if (!attributes->folder)
	{
		Asset &asset = this->assets[node->getId()];
		attributes->date = asset.date;
		attributes->length = asset.length;
	}
	
	return true;
}

long ProjectFileSystem::open(const std::string &path, OpenMode mode, const std::string &applicationName)
{
	this->project->checkChanges();
	
	// check if file exists
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return -1;
	
	// initialize operation data
	this->openData.id = node->getId();
	this->openData.fd = -1;
	
	// check if directory
	if (this->openData.id == "")
		return -1;
	
	Asset &asset = this->assets[this->openData.id];
	
	// check lock for write accesses
	if ((mode == WRITING) && !(asset.rights & Asset::WRITE_ACCESS))
	{
		if (asset.lock.user == "")
			this->project->lockAsset(this->openData.id, applicationName);
		
		// wait for lock
		while (!(asset.rights & Asset::WRITE_ACCESS))
		{
			if (!this->project->isConnected())
				return -1; // connection lost
			
			if ((asset.lock.user != "") && (asset.lock.user != "<you>"))
				return -1; // asset was locked by someone else
			
			this->project->waitChanges();
		}
	}
	
	// start asynchronous operation
	this->project->openAsset(this->openData.id, (mode == READING) ? Project::READING : Project::WRITING);
	
	while (this->openData.fd == -1)
	{
		if (!this->project->isConnected())
			return -1; // connection lost
		
		// TODO: check errors on this->assets[asset.id]
		
		this->project->waitChanges();
	}
	
	// open succeeded
	return this->openData.fd;
}

void ProjectFileSystem::close(long fd)
{
	this->project->checkChanges();
	
	if (fd >= 0)
		this->project->closeAsset((unsigned long)fd);
}

bool ProjectFileSystem::read(long fd, unsigned long offset, void *buffer, unsigned long length)
{
	if (fd < 0)
		return false; // BAD FD
	
	// TODO: check that offset < file length
	
	if (length == 0)
		return false; // cannot read 0 bytes
	
	this->project->checkChanges();
	
	// initialize operation data
	this->readData.fd = fd;
	this->readData.offset = offset;
	this->readData.buffer = buffer;
	this->readData.length = length;
	this->readData.sizeRead = 0;
	
	// start asynchronous operation
	this->project->readAsset(this->readData.fd, offset, length);
	
	while (this->readData.sizeRead != length)
	{
		if (!this->project->isConnected())
			return false; // connection lost
		
		// TODO: check errors on this->assets[asset.id]
		
		this->project->waitChanges();
	}
	
	// read succeeded
	return true;
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
	
	// reinsert the new asset into the hierarchy, if it was not deleted and at least
	// one revision is available locally
	if ((asset.path != "") && (asset.rights & Asset::READ_ACCESS))
		this->root->insert(asset.path, asset.id);
	
	// updated saved metadata
	this->assets[asset.id] = asset;
}

void ProjectFileSystem::assetOpened(const std::string &id, unsigned long fd)
{
	if (this->openData.id != id)
	{
		std::cout << "Received OPENED for the wrong asset! (" << id << " instead of " << this->openData.id << ")" << std::endl;
		return;
	}
	
	this->openData.fd = fd;
}

void ProjectFileSystem::chunkReceived(unsigned long fd, unsigned long offset, const void *buffer, unsigned long length)
{
	if (fd != this->readData.fd)
		return; // wrong fd
	
	if (offset != this->readData.offset)
		return; // wrong offset
	
	if (length > this->readData.length)
		return; // too much data
	
	memcpy(this->readData.buffer, buffer, length);
	this->readData.sizeRead = length;
}

} // wendy namespace
