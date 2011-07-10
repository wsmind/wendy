#ifndef _WIN32

#include <fuse.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

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
	filler(buf, "this_directory", NULL, 0);
	filler(buf, "is_a-cake-", NULL, 0);
	return 0;
}

int main(int argc, char **argv)
{
	fuse_operations operations;
	memset(&operations, 0, sizeof(operations));
	operations.getattr = wendy_getattr;
	operations.readdir = wendy_readdir;
	
	int fuse_stat = fuse_main(argc, argv, &operations, NULL);
	
	return fuse_stat;
}

#endif // other than _WIN32

