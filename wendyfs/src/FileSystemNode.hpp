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

#ifndef __WENDY_FILESYSTEMNODE_HPP__
#define __WENDY_FILESYSTEMNODE_HPP__

#include <string>
#include <vector>
#include <map>

/**
 * \class FileSystemNode
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
class FileSystemNode
{
	public:
		/**
		 * \brief Constructor
		 * \param id asset ID, or an empty string for folders
		 */
		FileSystemNode(const bool directory, unsigned long long size = 0);
		
		~FileSystemNode();
		
		bool isDirectory() const;
		
		bool isEmpty() const;
		
		unsigned long long getSize() const;
		
		/**
		 * \brief List nodes contained in this one (not recursive)
		 */
		std::vector<std::string> list() const;
		
		/**
		 * \brief Find one of the relative subnodes of this one
		 * \return NULL if not found
		 */
		FileSystemNode *find(const std::string &path);
		
		/**
		 * \brief Insert a node in the tree
		 *
		 * The given path is relative to this node.
		 */
		void insert(const std::string &path, const bool directory, unsigned long long size = 0);
		
		/**
		 * \brief Remove a node from the tree
		 *
		 * The given path is relative to this node.
		 */
		void remove(const std::string &path);
	
	private:
		// separate the path in two at the next '/' (or end of string, in which case childPath is "")
		// output parameters can be NULL if their value is not wanted
		void extractPathComponents(const std::string &path, std::string *childName, std::string *childPath) const;
		
		bool directory;
		unsigned long long size;
		
		// child nodes
		typedef std::map<std::string, FileSystemNode *> NodeMap;
		NodeMap subNodes;
};

#endif //  __WENDY_FILESYSTEMNODE_HPP__
