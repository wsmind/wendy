/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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

#ifndef _WIN32

#include <fuse.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <wendy/ProjectFileSystem.hpp>

#include <iostream>

static wendy::ProjectFileSystem *fs = NULL;

// remove leading slash
static std::string makePathStandard(const char *filename)
{
	return std::string(filename).substr(1);
}

static int wendy_getattr(const char *filename, struct stat *stbuf)
{
	std::string path = makePathStandard(filename);
	wendy::ProjectFileSystem::FileAttributes attributes;
	
	if (!fs->stat(path, &attributes))
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
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (fs->stat(path, &attributes))
		return -EEXIST;
	
	fs->mkdir(path);
	
	return 0;
}

static int wendy_unlink(const char *filename)
{
	return 0;
}

static int wendy_rmdir(const char *filename)
{
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ENOENT;
	
	if (!attributes.folder)
		return -ENOTDIR;
	
	if (!fs->rmdir(path))
		return -ENOTEMPTY;
	
	return 0;
}

static int wendy_readdir(const char *filename, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	std::string path = makePathStandard(filename);
	
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	std::vector<std::string> files;
	fs->readdir(path, &files);
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
	operations.readlink = wendy_readlink;
	operations.mknod = wendy_mknod;
	operations.mkdir = wendy_mkdir;
	operations.unlink = wendy_unlink;
	operations.rmdir = wendy_rmdir;
	operations.readdir = wendy_readdir;
	
	fs = new wendy::ProjectFileSystem;
	
	int fuse_stat = fuse_main(argc, argv, &operations, NULL);
	
	delete fs;
	
	return fuse_stat;
}

#endif // other than _WIN32

