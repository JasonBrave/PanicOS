/*
 * Virtual filesystem
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

#include <defs.h>
#include <filesystem/fat32/fat32.h>
#include <filesystem/initramfs/initramfs.h>
#include <hal/hal.h>

#include "vfs.h"

void vfs_init(void) {
	cprintf("[vfs] mount initramfs on /\n");
	int status = initramfs_init();
	if (status < 0)
		panic("root filesystem not found");

	for (int i = 0; i < HAL_PARTITION_MAX; i++) {
		if (hal_partition_map[i].fs_type == HAL_PARTITION_FAT32) {
			cprintf("[vfs] mount fat32 on /fat32\n");
			fat32_mount(i);
			break;
		}
	}
}

int vfs_file_get_size(const char* filename) {
	struct VfsPath path;
	path.pathbuf = kalloc();
	path.parts = vfs_path_split(filename, path.pathbuf);
	int sz = initramfs_file_get_size(path.pathbuf);
	kfree(path.pathbuf);
	return sz;
}
