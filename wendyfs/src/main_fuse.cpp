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

#ifndef _WIN32

#include <fuse.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "ProjectProxy.hpp"

#include <iostream>

static ProjectProxy *proxy = NULL;

// remove leading slash
static std::string makePathStandard(const char *filename)
{
	return std::string(filename).substr(1);
}

static int wendy_getattr(const char *filename, struct stat *stbuf)
{
	std::string path = makePathStandard(filename);
	ProjectProxy::FileAttributes attributes;
	
	if (!proxy->getFileAttributes(path, &attributes))
		return -ENOENT;
	
	memset(stbuf, 0, sizeof(struct stat));
	
	if (attributes.folder)
	{
		stbuf->st_mode = S_IFDIR | 0750;
		stbuf->st_nlink = 2;
	}
	else
	{
		stbuf->st_mode = S_IFREG | 0736;
		stbuf->st_nlink = 1;
	}
	
	return 0;
}

static int wendy_readlink(const char *filename, char *target, size_t targetSize)
{
	// no support for symlinks
	strcpy(target, "");
	return 0;
}

static int wendy_mknod(const char *filename, mode_t mode, dev_t dev)
{
	return 0;
}

static int wendy_mkdir(const char *filename, mode_t mode)
{
	std::string path = makePathStandard(filename);
	proxy->createFolder(path);
	return 0;
}

static int wendy_unlink(const char *filename)
{
	return 0;
}

static int wendy_rmdir(const char *filename)
{
	std::string path = makePathStandard(filename);
	proxy->removeFolder(path);
	return 0;
}

static int wendy_readdir(const char *filename, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	std::string path = makePathStandard(filename);
	
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	std::string dirname = path;
	std::vector<std::string> files = proxy->listFolder(path);
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		std::cout << "found: " << files[i] << std::endl;
		filler(buf, files[i].c_str(), NULL, 0);
	}
	
	return 0;
}

int main(int argc, char **argv)
{
	fuse_operations operations;
	memset(&operations, 0, sizeof(operations));
	operations.getattr = wendy_getattr;
	//operations.readlink = wendy_readlink;
	//operations.mknod = wendy_mknod;
	operations.mkdir = wendy_mkdir;
	//operations.unlink = wendy_unlink;
	operations.rmdir = wendy_rmdir;
	operations.readdir = wendy_readdir;
	
	proxy = new ProjectProxy();
	
	int fuse_stat = fuse_main(argc, argv, &operations, NULL);
	
	delete proxy;
	
	return fuse_stat;
}

#endif // other than _WIN32

