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

unsigned int fat32_fat_read(int partition_id, unsigned int current) {
	uint32_t* buf = kalloc();
	unsigned int sect = fat32_superblock->reserved_sector + current / 128;
	hal_partition_read(partition_id, sect, 1, buf);
	unsigned int val = buf[current % 128];
	kfree(buf);
	return val;
}

unsigned int fat32_offset_cluster(int partition_id, unsigned int cluster, unsigned int offset) {
	int n = offset / 512 / fat32_superblock->sector_per_cluster;
	while (n--) {
		cluster = fat32_fat_read(partition_id, cluster);
		if (cluster >= 0x0ffffff8) {
			return 0;
		}
	}
	return cluster;
}

int fat32_write_fat(int partition_id, unsigned int cluster, unsigned int data) {
	uint32_t* buf = kalloc();
	int sect = fat32_superblock->reserved_sector + cluster / 128;
	if (hal_partition_read(partition_id, sect, 1, buf) < 0) {
		kfree(buf);
		return ERROR_READ_FAIL;
	}
	buf[cluster % 128] = data;
	// first fat
	if (hal_partition_write(partition_id, sect, 1, buf) < 0) {
		kfree(buf);
		return ERROR_WRITE_FAIL;
	}
	// second fat
	if (hal_partition_write(partition_id, sect + fat32_superblock->fat_size, 1, buf) < 0) {
		kfree(buf);
		return ERROR_WRITE_FAIL;
	}
	kfree(buf);
	return 0;
}

int fat32_append_cluster(int partition_id, unsigned int begin_cluster, unsigned int end_cluster) {
	unsigned int clus = begin_cluster;
	while (fat32_fat_read(partition_id, clus) < 0x0ffffff8) {
		clus = fat32_fat_read(partition_id, clus);
	}
	return fat32_write_fat(partition_id, clus, end_cluster);
}

int fat32_free_chain(int partition_id, unsigned int cluster) {
	do {
		// read it out and clear FAT entry
		unsigned int clus = fat32_fat_read(partition_id, cluster);
		if (fat32_write_fat(partition_id, cluster, 0) < 0) {
			return ERROR_WRITE_FAIL;
		}
		// advance to next FAT entry
		cluster = clus;
	} while (cluster < 0x0ffffff8);
	return 0;
}
