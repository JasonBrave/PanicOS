/*
 * block device hardware abstraction layer
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

#include "hal.h"

struct MbrEntry {
	uint8_t boot; // boot signature
	uint8_t first[3]; // first sector CHS address
	uint8_t type; // partition type
	uint8_t last[3]; // last sector CHS address
	uint32_t lba; // start sector LBA
	uint32_t size; // number of sectors
} PACKED;

struct BlockDevice hal_block_map[HAL_BLOCK_MAX];
struct HalPartitionMap hal_partition_map[HAL_PARTITION_MAX];

static struct HalPartitionMap* hal_partition_map_insert(enum HalPartitionFsType fs,
														unsigned int dev,
														unsigned int begin,
														unsigned int size) {
	for (int i = 0; i < HAL_PARTITION_MAX; i++) {
		if (hal_partition_map[i].fs_type == HAL_PARTITION_NONE) {
			hal_partition_map[i].fs_type = fs;
			hal_partition_map[i].dev = dev;
			hal_partition_map[i].begin = begin;
			hal_partition_map[i].size = size;
			return &hal_partition_map[i];
		}
	}
	return 0;
}

static void hal_block_probe_partition(int block_id) {
	void* bootsect = kalloc();
	if (hal_disk_read(block_id, 0, 1, bootsect) < 0) {
		panic("disk read error");
	}
	for (int j = 0; j < 4; j++) {
		struct MbrEntry* entry = bootsect + 0x1be + j * 0x10;
		enum HalPartitionFsType fs;
		if (entry->type == 0) {
			continue;
		} else if ((entry->type == 0xb) || (entry->type == 0xc)) {
			cprintf("[mbr] FAT32 partition on block device %d MBR %d\n", block_id, j);
			fs = HAL_PARTITION_FAT32;
		} else if (entry->type == 0x83) {
			cprintf("[mbr] Linux partition on block device %d MBR %d\n", block_id, j);
			fs = HAL_PARTITION_LINUX;
		} else {
			cprintf("[mbr] Partition on block device %d MBR %d type %x\n", block_id, j,
					entry->type);
			fs = HAL_PARTITION_OTHER;
		}
		if (!hal_partition_map_insert(fs, block_id, entry->lba, entry->size)) {
			panic("hal too many partition");
		}
	}
	kfree(bootsect);
}

void hal_block_register_device(const char* name, void* private,
							   const struct BlockDeviceDriver* driver) {
	for (int i = 0; i < HAL_BLOCK_MAX; i++) {
		if (!hal_block_map[i].driver) {
			cprintf("[hal] Block device %s added\n", name);
			hal_block_map[i].driver = driver;
			hal_block_map[i].private = private;
			hal_block_probe_partition(i);
			return;
		}
	}
	panic("too many block devices");
}

void hal_block_init(void) {
	memset(hal_block_map, 0, sizeof(hal_block_map));
	memset(hal_partition_map, 0, sizeof(hal_partition_map));
}

int hal_block_read(int id, int begin, int count, void* buf) {
	return hal_disk_read(id, begin, count, buf);
}

int hal_disk_read(int id, int begin, int count, void* buf) {
	if (id >= HAL_BLOCK_MAX) {
		return ERROR_INVAILD;
	}
	if (!hal_block_map[id].driver) {
		return ERROR_INVAILD;
	}

	return hal_block_map[id].driver->block_read(hal_block_map[id].private, begin, count,
												buf);
}

int hal_partition_read(int id, int begin, int count, void* buf) {
	if (id >= HAL_PARTITION_MAX) {
		return -1;
	}
	if (hal_partition_map[id].fs_type == HAL_PARTITION_NONE) {
		return -1;
	}
	return hal_disk_read(hal_partition_map[id].dev, hal_partition_map[id].begin + begin,
						 count, buf);
}

int hal_block_write(int id, int begin, int count, const void* buf) {
	return hal_disk_write(id, begin, count, buf);
}

int hal_disk_write(int id, int begin, int count, const void* buf) {
	if (id >= HAL_BLOCK_MAX) {
		return -1;
	}
	if (!hal_block_map[id].driver) {
		return ERROR_INVAILD;
	}

	return hal_block_map[id].driver->block_write(hal_block_map[id].private, begin,
												 count, buf);
}

int hal_partition_write(int id, int begin, int count, const void* buf) {
	if (id >= HAL_PARTITION_MAX) {
		return -1;
	}
	if (hal_partition_map[id].fs_type == HAL_PARTITION_NONE) {
		return -1;
	}
	return hal_disk_write(hal_partition_map[id].dev,
						  hal_partition_map[id].begin + begin, count, buf);
}
