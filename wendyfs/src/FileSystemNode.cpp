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

#include <FileSystemNode.hpp>

#include <iostream>

FileSystemNode::FileSystemNode(const bool directory, unsigned long long size)
{
	std::cout << "creating node (directory = '" << directory << "'" << std::endl;
	this->directory = directory;
	this->size = size;
}

FileSystemNode::~FileSystemNode()
{
	// destroy all subnodes
	NodeMap::iterator it;
	for (it = this->subNodes.begin(); it != this->subNodes.end(); ++it)
		delete it->second;
}

bool FileSystemNode::isDirectory() const
{
	return this->directory;
}

bool FileSystemNode::isEmpty() const
{
	return (this->subNodes.size() == 0);
}

unsigned long long FileSystemNode::getSize() const
{
	return this->size;
}

std::vector<std::string> FileSystemNode::list() const
{
	std::vector<std::string> subElements;
	
	// list subfolders first
	NodeMap::const_iterator it;
	for (it = this->subNodes.begin(); it != this->subNodes.end(); ++it)
		subElements.push_back(it->first);
	
	return subElements;
}

FileSystemNode *FileSystemNode::find(const std::string &path)
{
	if (path == "")
		return this;
	
	std::string childName;
	std::string childPath;
	this->extractPathComponents(path, &childName, &childPath);
	
	NodeMap::const_iterator it = this->subNodes.find(childName);
	if (it == this->subNodes.end())
		return NULL;
	
	return it->second->find(childPath);
}

void FileSystemNode::insert(const std::string &path, const bool directory, unsigned long long size)
{
	std::string childName;
	std::string childPath;
	this->extractPathComponents(path, &childName, &childPath);
	
	if (childPath == "")
	{
		// leaf node creation
		this->subNodes[childName] = new FileSystemNode(directory, size);
	}
	else
	{
		// check if subfolder exists
		NodeMap::iterator it = this->subNodes.find(childName);
		if (it == this->subNodes.end())
			this->subNodes[childName] = new FileSystemNode(true);
		
		// insert asset into subfolder
		this->subNodes[childName]->insert(childPath, directory, size);
	}
}

void FileSystemNode::remove(const std::string &path)
{
	std::string childName;
	std::string childPath;
	this->extractPathComponents(path, &childName, &childPath);
	
	if (childPath == "")
	{
		// leaf node destruction
		delete this->subNodes[childName];
		this->subNodes.erase(childName);
	}
	else
	{
		// remove path in subfolders
		NodeMap::iterator it = this->subNodes.find(childName);
		if (it != this->subNodes.end())
			this->subNodes[childName]->remove(childPath);
	}
}

void FileSystemNode::extractPathComponents(const std::string &path, std::string *childName, std::string *childPath) const
{
	size_t slashPos = path.find('/');
	if (childName)
		*childName = path.substr(0, slashPos);
	if (childPath)
	{
		if (slashPos != std::string::npos)
			*childPath = path.substr(slashPos + 1);
		else
			*childPath = "";
	}
}
