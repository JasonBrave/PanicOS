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
#include <filesystem/filesystem.h>
#include <hal/hal.h>

#include "vfs.h"

struct VfsMountTableEntry vfs_mount_table[VFS_MOUNT_TABLE_MAX];

void vfs_init(void) {
	memset(vfs_mount_table, 0, sizeof(vfs_mount_table));
	int fs_id = 0;

	for (int i = 0; i < HAL_PARTITION_MAX; i++) {
		if (hal_partition_map[i].fs_type == HAL_PARTITION_TYPE_FAT32) {
			if (fs_id == 0) {
				cprintf("[vfs] mount fat32 on /\n");
			} else if (fs_id == 1) {
				cprintf("[vfs] mount fat32 on /fat32\n");
			}
			filesystem_fat32_driver->mount(i, &vfs_mount_table[fs_id].private);
			vfs_mount_table[fs_id].fs_driver = filesystem_fat32_driver;
			fs_id++;
			break;
		} else if (hal_partition_map[i].fs_type == HAL_PARTITION_TYPE_DATA) {
			if (filesystem_fat32_driver->probe(i) != 0) {
				continue;
			}
			if (fs_id == 0) {
				cprintf("[vfs] mount fat32 on /\n");
			} else if (fs_id == 1) {
				cprintf("[vfs] mount fat32 on /fat32\n");
			}
			filesystem_fat32_driver->mount(i, &vfs_mount_table[fs_id].private);
			vfs_mount_table[fs_id].fs_driver = filesystem_fat32_driver;
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

	int sz = vfs_mount_table[fs_id].fs_driver->get_file_size(vfs_mount_table[fs_id].private, path);
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

	int sz = vfs_mount_table[fs_id].fs_driver->get_file_mode(vfs_mount_table[fs_id].private, path);
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

	int ret
		= vfs_mount_table[fs_id].fs_driver->create_directory(vfs_mount_table[fs_id].private, path);
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

	int ret = vfs_mount_table[fs_id].fs_driver->remove_file(vfs_mount_table[fs_id].private, path);

	kfree(filepath.pathbuf);
	return ret;
}
