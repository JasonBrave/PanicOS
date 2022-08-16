/*
 * FAT32 filesystem driver
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
#include <hal/hal.h>

#include "fat32-struct.h"

struct FAT32BootSector* fat32_superblock;

int fat32_mount(int partition_id) {
	struct FAT32BootSector* bootsect = (void*)kalloc();
	if (hal_partition_read(partition_id, 0, 1, bootsect) < 0) {
		return ERROR_READ_FAIL;
	}
	if (strncmp(bootsect->fstype, "FAT32", 5)) {
		panic("Mounting a non-FAT32 filesystem");
	}
	char label[12];
	strncpy(label, bootsect->volume_label, 12);
	label[11] = '\0';
	cprintf("[fat32] Mount %s\n", label);
	fat32_superblock = bootsect;
	return 0;
}

int fat32_probe(int partition_id) {
	struct FAT32BootSector* bootsect = (void*)kalloc();
	if (hal_partition_read(partition_id, 0, 1, bootsect) < 0) {
		return ERROR_READ_FAIL;
	}
	return strncmp(bootsect->fstype, "FAT32", 5);
}
