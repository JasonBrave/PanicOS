/*
 * Virtual filesystem directory support
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/errorcode.h>
#include <defs.h>
#include <filesystem/fat32/fat32.h>
#include <filesystem/initramfs/initramfs.h>

#include "vfs.h"

int vfs_dir_open(struct FileDesc* fd, const char* dirname) {
	struct VfsPath path;
	path.pathbuf = kalloc();
	path.parts = vfs_path_split(dirname, path.pathbuf);
	int fs_id;
	if (path.parts == 0) {
		fs_id = 0;
	} else {
		if (strncmp(path.pathbuf, "fat32", 16) == 0) {
			fs_id = 1;
		} else {
			panic("vfs_dir_open");
		}
	}
	fd->fs_id = fs_id;

	if (vfs_mount_table[fs_id].fs_type == VFS_FS_INITRAMFS) {
		int off = initramfs_dir_open();
		if (off < 0) {
			kfree(path.pathbuf);
			return off;
		}
		fd->offset = off;
	} else if (vfs_mount_table[fs_id].fs_type == VFS_FS_FAT32) {
		struct VfsPath fpath = {.parts = path.parts - 1, .pathbuf = path.pathbuf + 128};
		fd->block = fat32_open(vfs_mount_table[fs_id].partition_id, fpath);
		if (fd->block == 0xffffffff) {
			kfree(path.pathbuf);
			return -1;
		}
		fd->offset =
			fat32_dir_first_file(vfs_mount_table[fs_id].partition_id, fd->block);
	} else {
		return ERROR_INVAILD;
	}
	fd->dir = 1;
	fd->read = 1;
	fd->used = 1;
	kfree(path.pathbuf);
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
	if (vfs_mount_table[fd->fs_id].fs_type == VFS_FS_INITRAMFS) {
		fd->offset = initramfs_dir_read(fd->offset, buffer);
		if (fd->offset == 0) {
			return 0; // EOF
		}
	} else if (vfs_mount_table[fd->fs_id].fs_type == VFS_FS_FAT32) {
		if (fd->offset == 0) {
			return 0; // EOF
		}
		fat32_dir_name(vfs_mount_table[fd->fs_id].partition_id, buffer, fd->block,
					   fd->offset);
		fd->offset = fat32_dir_next_file(vfs_mount_table[fd->fs_id].partition_id,
										 fd->block, fd->offset);
	} else {
		panic("vfs_dir_read");
	}
	return 1;
}

int vfs_dir_close(struct FileDesc* fd) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->dir) {
		return ERROR_INVAILD;
	}

	fd->used = 0;
	return 0;
}
