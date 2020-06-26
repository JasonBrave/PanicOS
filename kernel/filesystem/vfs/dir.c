#include <common/errorcode.h>
#include <filesystem/initramfs/initramfs.h>

#include "vfs.h"

int vfs_dir_open(struct FileDesc* fd, const char* dirname) {
	int off = initramfs_dir_open();
	if (off < 0) {
		return off;
	}
	fd->offset = off;
	fd->dir = 1;
	fd->read = 1;
	fd->used = 1;
	return 0;
}

int vfs_dir_read(struct FileDesc* fd, char* buffer) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->read) {
		return ERROR_INVAILD;
	}
	if (!fd->dir) {
		return ERROR_INVAILD;
	}

	fd->offset = initramfs_dir_read(fd->offset, buffer);
	if (fd->offset == 0) {
		return 0; // EOF
	}
	return 1;
}

int vfs_dir_close(struct FileDesc* fd) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->read) {
		return ERROR_INVAILD;
	}
	if (!fd->dir) {
		return ERROR_INVAILD;
	}

	fd->used = 0;
	return 0;
}
