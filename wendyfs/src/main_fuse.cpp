#ifndef _WIN32

#include <fuse.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "ProjectProxy.hpp"

static ProjectProxy *proxy = NULL;

static int wendy_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	
	if (!strcmp(path, "/"))
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

static int wendy_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	std::vector<std::string> files = proxy->listDirectory("");
	for (unsigned int i = 0; i < files.size(); ++i)
		filler(buf, files[i].c_str(), NULL, 0);
	
	return 0;
}

int main(int argc, char **argv)
{
	fuse_operations operations;
	memset(&operations, 0, sizeof(operations));
	operations.getattr = wendy_getattr;
	operations.readdir = wendy_readdir;
	
	proxy = new ProjectProxy();
	
	int fuse_stat = fuse_main(argc, argv, &operations, NULL);
	
	delete proxy;
	
	return fuse_stat;
}

#endif // other than _WIN32

