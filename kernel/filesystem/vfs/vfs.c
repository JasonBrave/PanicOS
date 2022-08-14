/*
 * Virtual filesystem
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/errorcode.h>
#include <defs.h>
#include <filesystem/fat32/fat32.h>
#include <filesystem/initramfs/initramfs.h>
#include <hal/hal.h>

#include "vfs.h"

struct VfsMountTableEntry vfs_mount_table[VFS_MOUNT_TABLE_MAX];

void vfs_init(void) {
	memset(vfs_mount_table, 0, sizeof(vfs_mount_table));
	int fs_id = 0;

	int status = initramfs_init();
	if (status == 0) {
		cprintf("[vfs] mount initramfs on /\n");
		vfs_mount_table[0].fs_type = VFS_FS_INITRAMFS;
		vfs_mount_table[0].partition_id = 0; // N/A
		fs_id++;
	}

	for (int i = 0; i < HAL_PARTITION_MAX; i++) {
		if (hal_partition_map[i].fs_type == HAL_PARTITION_TYPE_FAT32) {
			if (fs_id == 0) {
				cprintf("[vfs] mount fat32 on /\n");
			} else if (fs_id == 1) {
				cprintf("[vfs] mount fat32 on /fat32\n");
			}
			fat32_mount(i);
			vfs_mount_table[fs_id].fs_type = VFS_FS_FAT32;
			vfs_mount_table[fs_id].partition_id = i;
			fs_id++;
			break;
		}
	}
	if (fs_id == 0) {
		panic("root filesystem not found");
	}
}

int vfs_path_to_fs(struct VfsPath orig_path, struct VfsPath* path) {
	if (orig_path.parts == 0) {
		path->parts = 0;
		return 0;
	} else {
		if (strncmp(orig_path.pathbuf, "fat32", 64) == 0) {
			path->parts = orig_path.parts - 1;
			path->pathbuf = orig_path.pathbuf + 128;
			return 1;
		} else {
			path->parts = orig_path.parts;
			path->pathbuf = orig_path.pathbuf;
			return 0;
		}
	}
}

int vfs_file_get_size(const char* filename) {
	struct VfsPath filepath;
	filepath.pathbuf = kalloc();
	filepath.parts = vfs_path_split(filename, filepath.pathbuf);
	if (filename[0] != '/') {
		vfs_get_absolute_path(&filepath);
	}
	struct VfsPath path;
	int fs_id = vfs_path_to_fs(filepath, &path);

	int sz;
	if (vfs_mount_table[fs_id].fs_type == VFS_FS_INITRAMFS) {
		sz = initramfs_file_get_size(path.pathbuf);
	} else if (vfs_mount_table[fs_id].fs_type == VFS_FS_FAT32) {
		sz = fat32_file_size(vfs_mount_table[fs_id].partition_id, path);
	} else {
		return ERROR_INVAILD;
	}
	kfree(filepath.pathbuf);
	return sz;
}

int vfs_file_get_mode(const char* filename) {
	struct VfsPath filepath;
	filepath.pathbuf = kalloc();
	filepath.parts = vfs_path_split(filename, filepath.pathbuf);
	if (filename[0] != '/') {
		vfs_get_absolute_path(&filepath);
	}
	struct VfsPath path;
	int fs_id = vfs_path_to_fs(filepath, &path);

	int sz;
	if (vfs_mount_table[fs_id].fs_type == VFS_FS_INITRAMFS) {
		sz = initramfs_file_get_mode(path.pathbuf);
	} else if (vfs_mount_table[fs_id].fs_type == VFS_FS_FAT32) {
		sz = fat32_file_mode(vfs_mount_table[fs_id].partition_id, path);
	} else {
		return ERROR_INVAILD;
	}
	kfree(filepath.pathbuf);
	return sz;
}

int vfs_mkdir(const char* dirname) {
	struct VfsPath filepath;
	filepath.pathbuf = kalloc();
	filepath.parts = vfs_path_split(dirname, filepath.pathbuf);
	if (dirname[0] != '/') {
		vfs_get_absolute_path(&filepath);
	}
	struct VfsPath path;
	int fs_id = vfs_path_to_fs(filepath, &path);

	int ret;
	if (vfs_mount_table[fs_id].fs_type == VFS_FS_FAT32) {
		ret = fat32_mkdir(vfs_mount_table[fs_id].partition_id, path);
	} else {
		ret = ERROR_INVAILD;
	}
	kfree(filepath.pathbuf);
	return ret;
}

int vfs_file_remove(const char* file) {
	struct VfsPath filepath;
	filepath.pathbuf = kalloc();
	filepath.parts = vfs_path_split(file, filepath.pathbuf);
	if (file[0] != '/') {
		vfs_get_absolute_path(&filepath);
	}
	struct VfsPath path;
	int fs_id = vfs_path_to_fs(filepath, &path);

	int ret;
	if (vfs_mount_table[fs_id].fs_type == VFS_FS_FAT32) {
		ret = fat32_file_remove(vfs_mount_table[fs_id].partition_id, path);
	} else {
		ret = ERROR_INVAILD;
	}
	kfree(filepath.pathbuf);
	return ret;
}
