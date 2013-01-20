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

#include <FileSystem.hpp>

#include <File.hpp>
#include <FileSystemNode.hpp>

#include <wendy/Client.hpp>
#include <wendy/RequestState.hpp>

#include <iostream>
#include <cstring>
#include <cassert>

FileSystem::FileSystem()
{
	this->root = new FileSystemNode(true);
	
	this->client = new wendy::Client;
	
	// list all assets and insert them in the tree
	wendy::RequestState listState;
	wendy::Client::PathList files;
	client->list(&listState, "**", &files);
	client->waitRequest(&listState);
	
	if (listState.isSuccess())
	{
		for (unsigned int i = 0; i < files.size(); i++)
		{
			std::cout << "Found file: " << files[i] << std::endl;
			this->root->insert(files[i].substr(1), false);
		}
	}
}

FileSystem::~FileSystem()
{
	delete this->client;
	
	delete this->root;
}

bool FileSystem::stat(const std::string &path, FileAttributes *attributes)
{
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false;
	
	attributes->folder = node->isDirectory();
	attributes->date = 0;
	attributes->length = 0;
	
	if (!attributes->folder)
	{
		//Asset &asset = this->assets[node->getId()];
		attributes->date = 42;
		attributes->length = 10 * 1024 * 1024;
	}
	
	return true;
}

bool FileSystem::unlink(const std::string &path)
{
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false; // not found
	
	// TODO: lock asset
	
	// TODO: this->project->deleteAsset(node->getId());
	
	return true;
}

File *FileSystem::open(const std::string &path, bool reading, bool writing, bool truncate, const std::string &applicationName)
{
	//this->project->checkChanges();
	
	// must open at least for one of the modes
	assert(reading || writing);
	
	// check if file exists
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return NULL;
	
	File *file = new File(this->client, path);
	
	file->open(reading, writing, truncate);
	
	return file;
	
	// initialize operation data
	/*this->openData.id = node->getId();
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
	return this->openData.fd;*/
	return 0;
}

bool FileSystem::close(File *file)
{
	bool result = file->close();
	
	delete file;
	
	return result;
	
	/*this->project->checkChanges();
	
	if (fd >= 0)
		this->project->closeAsset((unsigned long)fd);*/
}

bool FileSystem::read(File *file, unsigned long offset, void *buffer, unsigned long length)
{
	return file->read(offset, buffer, length);
	
	/*if (fd < 0)
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
	}*/
	
	// read succeeded
	//return true;
}

bool FileSystem::write(File *file, unsigned long offset, const void *buffer, unsigned long length)
{
	return file->write(offset, buffer, length);
}

bool FileSystem::mkdir(const std::string &path)
{
	//this->project->checkChanges();
	
	FileSystemNode *node = this->root->find(path);
	if (node)
		return false; // already exists
	
	this->root->insert(path, "");
	return true;
}

bool FileSystem::rmdir(const std::string &path)
{
	//this->project->checkChanges();
	
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false; // not found
	
	if (!node->isDirectory())
		return false; // not a directory
	
	if (!node->isEmpty())
		return false; // not empty
	
	this->root->remove(path);
	
	return true;
}

bool FileSystem::readdir(const std::string &path, std::vector<std::string> *items)
{
	//this->project->checkChanges();
	
	FileSystemNode *node = this->root->find(path);
	if (!node)
		return false; // not found
	
	if (!node->isDirectory())
		return false; // not a directory
	
	*items = node->list();
	
	return true;
}
