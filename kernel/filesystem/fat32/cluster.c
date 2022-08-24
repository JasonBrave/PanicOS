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

int fat32_cluster_to_sector(struct FAT32Private* priv, unsigned int cluster) {
	return (cluster - 2) * (priv->boot_sector->sector_per_cluster)
		   + (priv->boot_sector->reserved_sector)
		   + (priv->boot_sector->fat_number) * (priv->boot_sector->fat_size);
}

int fat32_read_cluster(struct FAT32Private* priv, void* dest, unsigned int cluster,
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
		unsigned int sector = fat32_cluster_to_sector(priv, cluster) + (begin + off) / SECTORSIZE;
		if (hal_partition_read(priv->partition_id, sector, 1, sect) < 0) {
			kfree(sect);
			return ERROR_READ_FAIL;
		}
		memmove(dest + off, sect + (begin + off) % SECTORSIZE, copysize);
		off += copysize;
	}
	kfree(sect);
	return 0;
}

int fat32_read(void* private, unsigned int cluster, void* buf, unsigned int offset,
			   unsigned int size) {
	struct FAT32Private* priv = private;
	int clussize = priv->boot_sector->sector_per_cluster * SECTORSIZE;
	unsigned int off = 0;
	while (off < size) {
		int copysize;
		if ((offset + off) / clussize < (offset + size) / clussize) {
			copysize = ((offset + off) / clussize + 1) * clussize - (offset + off);
		} else {
			copysize = size - off;
		}
		unsigned int clus = fat32_offset_cluster(priv, cluster, offset + off);
		if (fat32_read_cluster(priv, buf + off, clus, (offset + off) % clussize, copysize) < 0) {
			return ERROR_READ_FAIL;
		}
		off += copysize;
	}
	return size;
}

int fat32_write_cluster(struct FAT32Private* priv, const void* src, unsigned int cluster,
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
		unsigned int sector = fat32_cluster_to_sector(priv, cluster) + (begin + off) / SECTORSIZE;
		if (hal_partition_read(priv->partition_id, sector, 1, sect) < 0) {
			kfree(sect);
			return ERROR_READ_FAIL;
		}
		memmove(sect + (begin + off) % SECTORSIZE, src + off, copysize);
		if (hal_partition_write(priv->partition_id, sector, 1, sect) < 0) {
			kfree(sect);
			return ERROR_WRITE_FAIL;
		}
		off += copysize;
	}
	kfree(sect);
	return 0;
}

unsigned int fat32_allocate_cluster(struct FAT32Private* priv) {
	unsigned int* buf = kalloc();
	for (unsigned int fat = 0; fat < priv->boot_sector->fat_size; fat++) {
		int sect = priv->boot_sector->reserved_sector + fat;
		if (hal_partition_read(priv->partition_id, sect, 1, buf) < 0) {
			return ERROR_READ_FAIL;
		}
		for (int i = 0; i < 128; i++) {
			if (buf[i] == 0) {
				buf[i] = 0x0fffffff;
				// first FAT
				if (hal_partition_write(priv->partition_id, sect, 1, buf) < 0) {
					return ERROR_WRITE_FAIL;
				}
				// second FAT
				if (hal_partition_write(priv->partition_id, sect + priv->boot_sector->fat_size, 1,
										buf)
					< 0) {
					return ERROR_WRITE_FAIL;
				}
				kfree(buf);
				return fat * 128 + i;
			}
		}
	}
	kfree(buf);
	return 0;
}

int fat32_write(void* private, unsigned int cluster, const void* buf, unsigned int offset,
				unsigned int size) {
	struct FAT32Private* priv = private;
	int clussize = priv->boot_sector->sector_per_cluster * SECTORSIZE;
	unsigned int off = 0;
	while (off < size) {
		int copysize;
		if ((offset + off) / clussize < (offset + size) / clussize) {
			copysize = ((offset + off) / clussize + 1) * clussize - (offset + off);
		} else {
			copysize = size - off;
		}
		unsigned int clus = fat32_offset_cluster(priv, cluster, offset + off);
		if (clus == 0) { // end of cluster chain
			if ((clus = fat32_allocate_cluster(priv)) < 0) {
				return ERROR_OUT_OF_SPACE;
			}
			if (fat32_append_cluster(priv, cluster, clus) < 0) {
				return ERROR_WRITE_FAIL;
			}
		}
		if (fat32_write_cluster(priv, buf + off, clus, (offset + off) % clussize, copysize) < 0) {
			return ERROR_WRITE_FAIL;
		}
		off += copysize;
	}
	return size;
}
