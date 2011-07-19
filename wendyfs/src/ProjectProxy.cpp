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

#include "ProjectProxy.hpp"
#include "Node.hpp"

#include <wendy/Project.hpp>

ProjectProxy::ProjectProxy()
{
	this->root = new Node;
	
	this->project = new wendy::Project(this);
	this->project->connect();
}

ProjectProxy::~ProjectProxy()
{
	this->project->disconnect();
	
	delete this->root;
}

void ProjectProxy::assetAdded(wendy::Project *project, const wendy::Asset &asset)
{
	this->root->insert(asset.path, asset.id);
}

void ProjectProxy::assetUpdated(wendy::Project *project, const wendy::Asset &asset)
{
	this->root->remove(asset.path);
	this->root->insert(asset.path, asset.id);
}

void ProjectProxy::assetRemoved(wendy::Project *project, const wendy::Asset &asset)
{
	this->root->remove(asset.path);
}

std::vector<std::string> ProjectProxy::listDirectory(std::string name)
{
	// update local asset list
	this->project->checkChanges();
	
	Node *node = this->root->find(name);
	return node->list();
}
