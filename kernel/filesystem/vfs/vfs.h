#ifndef _VFS_H
#define _VFS_H

#include <filesystem/filesystem.h>

struct FileDesc {
	struct {
		int used : 1;
		int read : 1;
		int write : 1;
		int append : 1; // append mode
		int dir : 1; // directory
	};

	unsigned int fs_id; // ID in vfs_mount_table
	unsigned int block; // file data starting block/cluster
	unsigned int offset; // file pointer offset
	unsigned int size; // current file size
					   // below are not used in read-only files
	struct VfsPath path; // for update file size,not used for read-only file
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

struct VfsMountTableEntry {
	const struct FilesystemDriver* fs_driver;
	void* private;
};

#define VFS_MOUNT_TABLE_MAX 8

extern struct VfsMountTableEntry vfs_mount_table[VFS_MOUNT_TABLE_MAX];

// vfs.c
void vfs_init(void);
int vfs_path_to_fs(struct VfsPath orig_path, struct VfsPath* path);
int vfs_file_get_size(const char* filename);
int vfs_file_get_mode(const char* filename);
int vfs_mkdir(const char* dirname);
int vfs_file_remove(const char* file);

// filedesc.c
int vfs_fd_open(struct FileDesc* fd, const char* filename, int mode);
int vfs_fd_read(struct FileDesc* fd, void* buf, unsigned int size);
int vfs_fd_write(struct FileDesc* fd, const char* buf, unsigned int size);
int vfs_fd_close(struct FileDesc* fd);
int vfs_fd_seek(struct FileDesc* fd, unsigned int off, enum FileSeekMode mode);

// dir.c
int vfs_dir_open(struct FileDesc* fd, const char* dirname);
int vfs_dir_read(struct FileDesc* fd, char* buffer);
int vfs_dir_close(struct FileDesc* fd);

// path.c
int vfs_path_split(const char* path, char* buf);
int vfs_path_compare(int lhs_parts, const char* lhs_buf, int rhs_parts, const char* rhs_buf);
void vfs_path_tostring(struct VfsPath path, char* buf);
void vfs_get_absolute_path(struct VfsPath* path);

#endif
