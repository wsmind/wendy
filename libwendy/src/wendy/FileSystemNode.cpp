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

#include <wendy/FileSystemNode.hpp>

#include <iostream>

namespace wendy {

FileSystemNode::FileSystemNode(const std::string &id)
{
	std::cout << "creating node with ID '" << id << "'" << std::endl;
	this->id = id;
}

FileSystemNode::~FileSystemNode()
{
	// destroy all subnodes
	NodeMap::iterator it;
	for (it = this->subNodes.begin(); it != this->subNodes.end(); ++it)
		delete it->second;
}

const std::string &FileSystemNode::getId()
{
	return this->id;
}

std::vector<std::string> FileSystemNode::list()
{
	std::vector<std::string> subElements;
	
	// list subfolders first
	NodeMap::iterator it;
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
	
	NodeMap::iterator it = this->subNodes.find(childName);
	if (it == this->subNodes.end())
		return NULL;
	
	return it->second->find(childPath);
}

void FileSystemNode::insert(const std::string &path, const std::string &id)
{
	std::string childName;
	std::string childPath;
	this->extractPathComponents(path, &childName, &childPath);
	
	if (childPath == "")
	{
		// leaf node creation
		this->subNodes[childName] = new FileSystemNode(id);
	}
	else
	{
		// check if subfolder exists
		NodeMap::iterator it = this->subNodes.find(childName);
		if (it == this->subNodes.end())
			this->subNodes[childName] = new FileSystemNode;
		
		// insert asset into subfolder
		this->subNodes[childName]->insert(childPath, id);
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

void FileSystemNode::extractPathComponents(const std::string &path, std::string *childName, std::string *childPath)
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

} // wendy namespace
