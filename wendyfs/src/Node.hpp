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

#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <string>
#include <vector>
#include <map>

#include <wendy/Asset.hpp>

/**
 * \class Node
 * \brief Hierarchical representation of the assets
 *
 * The folder hierarchy acts as a virtual filesystem that
 * cross references assets ids only. No actual data is
 * stored in the tree.
 *
 * All nodes carry a string ID. This ID matches the asset
 * ID for leaf nodes, and is just an empty string for folders.
 *
 * The main goal is to allow a quick reference to assets
 * from a filesystem interface. Another important aspect
 * is that folders are embodied into regular nodes, allowing
 * them to be manipulated as such instead of implicitly through
 * asset paths. This is needed for empty folders to exist
 * before some assets are created inside them.
 */
class Node
{
	public:
		/**
		 * \brief Constructor
		 * \param id asset ID, or an empty string for folders
		 */
		Node(const std::string &id = "");
		
		const std::string &getId();
		
		/**
		 * \brief List nodes contained in this one (not recursive)
		 */
		std::vector<std::string> list();
		
		/**
		 * \brief Find one of the relative subnodes of this one
		 */
		Node *find(const std::string &path);
		
		/**
		 * \brief Insert a node in the tree
		 *
		 * The given path is relative to this node. Pass an empty string id to
		 * virtually create an empty folder instead of an asset.
		 */
		void insert(const std::string &path, const std::string &id = "");
		
		/**
		 * \brief Remove a node from the tree
		 *
		 * The given path is relative to this node.
		 */
		void remove(const std::string &path);
	
	private:
		// separate the path in two at the next '/' (or end of string, in which case childPath is "")
		// output parameters can be NULL if their value is not wanted
		void extractPathComponents(const std::string &path, std::string *childName, std::string *childPath);
		
		std::string id;
		
		// child nodes
		typedef std::map<std::string, Node *> NodeMap;
		NodeMap subNodes;
};

#endif //  __NODE_HPP__
