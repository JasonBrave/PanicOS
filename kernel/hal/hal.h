#ifndef _HAL_HAL_H
#define _HAL_HAL_H

#include <common/types.h>

// HAL Block Device
struct BlockDeviceDriver {
	int (*block_read)(void* private, unsigned int begin, int count, void* buf);
	int (*block_write)(void* private, unsigned int begin, int count, const void* buf);
};

struct BlockDevice {
	const struct BlockDeviceDriver* driver;
	void* private;
};

#define HAL_BLOCK_MAX 8

// partition table cache
enum HalPartitionFsType {
	HAL_PARTITION_NONE,
	HAL_PARTITION_OTHER,
	HAL_PARTITION_FAT32,
	HAL_PARTITION_LINUX,
};

struct HalPartitionMap {
	enum HalPartitionFsType fs_type;
	unsigned int dev;
	unsigned int begin;
	unsigned int size;
};

#define HAL_PARTITION_MAX 8

// block.c
extern struct BlockDevice hal_block_map[HAL_BLOCK_MAX];
extern struct HalPartitionMap hal_partition_map[HAL_PARTITION_MAX];
struct HalPartitionMap* hal_partition_map_insert(enum HalPartitionFsType fs,
												 unsigned int dev, unsigned int begin,
												 unsigned int size);
void hal_block_init(void);
void hal_block_register_device(const char* name, void* private,
							   const struct BlockDeviceDriver* driver);
int hal_block_read(int id, int begin, int count, void* buf);
int hal_disk_read(int id, int begin, int count, void* buf);
int hal_partition_read(int id, int begin, int count, void* buf);
int hal_block_write(int id, int begin, int count, const void* buf);
int hal_disk_write(int id, int begin, int count, const void* buf);
int hal_partition_write(int id, int begin, int count, const void* buf);

// mbr.c
void mbr_probe_partition(int block_id);

// display.c

struct FramebufferDriver {
	phyaddr_t (*enable)(void* private, int xres, int yres);
	void (*disable)(void* private);
};

void hal_display_init(void);
void hal_display_register_device(const char* name, void* private,
								 struct FramebufferDriver* driver);

#endif
