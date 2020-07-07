/*
 * FAT32 filesystem driver
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
#include <hal/hal.h>

#include "fat32-struct.h"

unsigned int fat32_fat_read(int partition_id, unsigned int current) {
	uint32_t* buf = kalloc();
	unsigned int sect = fat32_superblock->reserved_sector + current / 128;
	hal_partition_read(partition_id, sect, 1, buf);
	unsigned int val = buf[current % 128];
	kfree(buf);
	return val;
}

unsigned int fat32_offset_cluster(int partition_id, unsigned int cluster,
								  unsigned int offset) {
	int n = offset / 512 / fat32_superblock->sector_per_cluster;
	while (n--) {
		cluster = fat32_fat_read(partition_id, cluster);
		if (cluster >= 0x0ffffff8) {
			return 0;
		}
	}
	return cluster;
}
