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

	unsigned int block; // file data starting block/cluster
	unsigned int offset; // file pointer offset
	unsigned int size; // current file size
};

enum OpenMode {
	O_READ = 1,
	O_WRITE = 2,
	O_CREATE = 4,
	O_APPEND = 8,
	O_TRUNC = 16,
};

enum FileSeekMode {
	SEEK_SET,
	SEEK_CUR,
	SEEK_END,
};

// vfs.c
int vfs_file_get_size(const char* filename);

// filedesc.c
int vfs_fd_open(struct FileDesc* fd, const char* filename, int mode);
int vfs_fd_read(struct FileDesc* fd, void* buf, unsigned int size);
int vfs_fd_close(struct FileDesc* fd);
int vfs_fd_seek(struct FileDesc* fd, unsigned int off, enum FileSeekMode mode);

// dir.c
int vfs_dir_open(struct FileDesc* fd, const char* dirname);
int vfs_dir_read(struct FileDesc* fd, char* buffer);
int vfs_dir_close(struct FileDesc* fd);

#endif
