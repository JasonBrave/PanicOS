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

struct BlockDevice hal_block_map[HAL_BLOCK_MAX];
struct HalPartitionMap hal_partition_map[HAL_PARTITION_MAX];

struct HalPartitionMap* hal_partition_map_insert(enum HalPartitionFilesystemType fs,
												 unsigned int dev, unsigned int begin,
												 unsigned int size) {
	for (int i = 0; i < HAL_PARTITION_MAX; i++) {
		if (hal_partition_map[i].fs_type == HAL_PARTITION_TYPE_NONE) {
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
	uint64_t* gptsect = kalloc();
	if (hal_disk_read(block_id, 1, 1, gptsect) < 0) {
		panic("disk read error");
	}
	if (gptsect[0] == 0x5452415020494645) { // EFI PART
		// GPT
		cprintf("[hal] GPT partition table on block %d\n", block_id);
		gpt_probe_partition(block_id);
	} else {
		// MBR
		cprintf("[hal] MBR partition table on block %d\n", block_id);
		mbr_probe_partition(block_id);
	}
	kfree(gptsect);
}

static void hal_block_cache_init(int block_id) {
	hal_block_map[block_id].cache = kalloc();
	memset(hal_block_map[block_id].cache, 0, 4096);
	hal_block_map[block_id].cache_next = 0;
	initlock(&hal_block_map[block_id].cache_lock, "block-cache");
}

void hal_block_register_device(const char* name, void* private,
							   const struct BlockDeviceDriver* driver) {
	for (int i = 0; i < HAL_BLOCK_MAX; i++) {
		if (!hal_block_map[i].driver) {
			cprintf("[hal] Block device %s added\n", name);
			hal_block_map[i].driver = driver;
			hal_block_map[i].private = private;
			hal_block_probe_partition(i);
			hal_block_cache_init(i);
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
	if (count > 1) {
		return hal_disk_read(id, begin, count, buf);
	}

	struct BlockDevice* blk = &hal_block_map[id];
	acquire(&blk->cache_lock);
	for (int i = 0; i < HAL_BLOCK_CACHE_MAX; i++) {
		if (blk->cache[i].buf && blk->cache[i].lba == begin) {
			memmove(buf, blk->cache[i].buf, 512);
			release(&blk->cache_lock);
			return 0;
		}
	}

	if (!blk->cache[blk->cache_next].buf) {
		blk->cache[blk->cache_next].buf = kalloc();
	}
	blk->cache[blk->cache_next].lba = begin;
	release(&blk->cache_lock);
	hal_disk_read(id, begin, count, blk->cache[blk->cache_next].buf);
	acquire(&blk->cache_lock);
	memmove(buf, blk->cache[blk->cache_next].buf, 512);
	blk->cache_next++;
	if (blk->cache_next >= HAL_BLOCK_CACHE_MAX) {
		blk->cache_next = 0;
	}
	release(&blk->cache_lock);
	return 0;
}

int hal_disk_read(int id, int begin, int count, void* buf) {
	if (id >= HAL_BLOCK_MAX) {
		return ERROR_INVAILD;
	}
	if (!hal_block_map[id].driver) {
		return ERROR_INVAILD;
	}

	return hal_block_map[id].driver->block_read(hal_block_map[id].private, begin, count, buf);
}

int hal_partition_read(int id, int begin, int count, void* buf) {
	if (id >= HAL_PARTITION_MAX) {
		return -1;
	}
	if (hal_partition_map[id].fs_type == HAL_PARTITION_TYPE_NONE) {
		return -1;
	}
	return hal_block_read(hal_partition_map[id].dev, hal_partition_map[id].begin + begin, count,
						  buf);
}

int hal_block_write(int id, int begin, int count, const void* buf) {
	if (count > 1) {
		for (int i = 0; i < HAL_BLOCK_CACHE_MAX; i++) {
			if (hal_block_map[id].cache[i].buf) {
				kfree(hal_block_map[id].cache[i].buf);
			}
		}
		hal_disk_write(id, begin, count, buf);
	}

	struct BlockDevice* blk = &hal_block_map[id];
	acquire(&blk->cache_lock);
	for (int i = 0; i < HAL_BLOCK_CACHE_MAX; i++) {
		if (blk->cache[i].buf && blk->cache[i].lba == begin) {
			memmove(blk->cache[i].buf, buf, 512);
			release(&blk->cache_lock);
			return hal_disk_write(id, begin, count, blk->cache[i].buf);
		}
	}
	release(&blk->cache_lock);
	return hal_disk_write(id, begin, count, buf);
}

int hal_disk_write(int id, int begin, int count, const void* buf) {
	if (id >= HAL_BLOCK_MAX) {
		return -1;
	}
	if (!hal_block_map[id].driver) {
		return ERROR_INVAILD;
	}

	return hal_block_map[id].driver->block_write(hal_block_map[id].private, begin, count, buf);
}

int hal_partition_write(int id, int begin, int count, const void* buf) {
	if (id >= HAL_PARTITION_MAX) {
		return -1;
	}
	if (hal_partition_map[id].fs_type == HAL_PARTITION_TYPE_NONE) {
		return -1;
	}
	return hal_block_write(hal_partition_map[id].dev, hal_partition_map[id].begin + begin, count,
						   buf);
}
