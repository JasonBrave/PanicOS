#ifndef _VFS_H
#define _VFS_H

struct FileDesc {
	struct {
		int used : 1;
		int read : 1;
		int write : 1;
		int append : 1; // append mode
		int dir : 1; // directory
	};

	unsigned int offset; // file pointer offset
	unsigned int size; // current file size
};

int vfs_dir_open(struct FileDesc* fd, const char* dirname);
int vfs_dir_read(struct FileDesc* fd, char* buffer);
int vfs_dir_close(struct FileDesc* fd);

#endif
