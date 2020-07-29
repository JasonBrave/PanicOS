#ifndef _HAL_HAL_H
#define _HAL_HAL_H

// HAL Block Device
enum HalBlockHwType {
	HAL_BLOCK_HWTYPE_NONE,
	HAL_BLOCK_HWTYPE_VIRTIO_BLK,
};

struct HalBlockMap {
	enum HalBlockHwType hw_type;
	unsigned int hw_id;
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
extern struct HalBlockMap hal_block_map[HAL_BLOCK_MAX];
extern struct HalPartitionMap hal_partition_map[HAL_PARTITION_MAX];
void hal_block_init(void);
int hal_block_read(int id, int begin, int count, void* buf);
int hal_disk_read(int id, int begin, int count, void* buf);
int hal_partition_read(int id, int begin, int count, void* buf);
int hal_block_write(int id, int begin, int count, const void* buf);
int hal_disk_write(int id, int begin, int count, const void* buf);
int hal_partition_write(int id, int begin, int count, const void* buf);

// display.c
void hal_display_init(void);

#endif
