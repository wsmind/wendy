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

#ifndef __PROJECTPROXY_HPP__
#define __PROJECTPROXY_HPP__

#include <string>
#include <vector>
#include <map>
#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>

class Node;

class ProjectProxy: public wendy::ProjectListener
{
	public:
		ProjectProxy();
		~ProjectProxy();
		
		virtual void assetAdded(wendy::Project *project, const wendy::Asset &asset);
		virtual void assetUpdated(wendy::Project *project, const wendy::Asset &asset);
		virtual void assetRemoved(wendy::Project *project, const wendy::Asset &asset);
		
		std::vector<std::string> listDirectory(std::string name);
	
	private:
		wendy::Project *project;
		
		// cross-reference asset IDs by path
		Node *root;
};

#endif //  __PROJECTPROXY_HPP__
