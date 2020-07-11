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
#include "fat32.h"

int fat32_cluster_to_sector(unsigned int cluster) {
	return (cluster - 2) * (fat32_superblock->sector_per_cluster) +
		   (fat32_superblock->reserved_sector) +
		   (fat32_superblock->fat_number) * (fat32_superblock->fat_size);
}

int fat32_read_cluster(int partition_id, void* dest, unsigned int cluster,
					   unsigned int begin, unsigned int size) {
	void* sect = kalloc();
	unsigned int off = 0;
	while (off < size) {
		int copysize;
		if ((begin + off) / SECTORSIZE < (begin + size) / SECTORSIZE) {
			copysize = ((begin + off) / SECTORSIZE + 1) * SECTORSIZE - (begin + off);
		} else {
			copysize = size - off;
		}
		unsigned int sector =
			fat32_cluster_to_sector(cluster) + (begin + off) / SECTORSIZE;
		if (hal_partition_read(partition_id, sector, 1, sect) < 0) {
			kfree(sect);
			return ERROR_READ_FAIL;
		}
		memmove(dest + off, sect + (begin + off) % SECTORSIZE, copysize);
		off += copysize;
	}
	kfree(sect);
	return 0;
}

int fat32_read(int partition_id, unsigned int cluster, void* buf, unsigned int offset,
			   unsigned int size) {
	int clussize = fat32_superblock->sector_per_cluster * SECTORSIZE;
	unsigned int off = 0;
	while (off < size) {
		int copysize;
		if ((offset + off) / clussize < (offset + size) / clussize) {
			copysize = ((offset + off) / clussize + 1) * clussize - (offset + off);
		} else {
			copysize = size - off;
		}
		unsigned int clus = fat32_offset_cluster(partition_id, cluster, offset + off);
		if (fat32_read_cluster(partition_id, buf + off, clus, (offset + off) % clussize,
							   copysize) < 0) {
			return ERROR_READ_FAIL;
		}
		off += copysize;
	}
	return size;
}
