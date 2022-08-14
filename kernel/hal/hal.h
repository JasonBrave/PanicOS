#ifndef _HAL_HAL_H
#define _HAL_HAL_H

#include <common/spinlock.h>
#include <common/types.h>

// HAL Block Device
struct BlockDeviceDriver {
	int (*block_read)(void* private, unsigned int begin, int count, void* buf);
	int (*block_write)(void* private, unsigned int begin, int count, const void* buf);
};

#define HAL_BLOCK_CACHE_MAX 512

struct BlockCache {
	int lba;
	void* buf;
};

struct BlockDevice {
	const struct BlockDeviceDriver* driver;
	void* private;
	struct BlockCache* cache;
	int cache_next;
	struct spinlock cache_lock;
};

#define HAL_BLOCK_MAX 8

enum HalPartitionFilesystemType {
	HAL_PARTITION_TYPE_NONE,
	HAL_PARTITION_TYPE_OTHER,
	HAL_PARTITION_TYPE_FAT32,
	HAL_PARTITION_TYPE_LINUX,
	HAL_PARTITION_TYPE_DATA,
	HAL_PARTITION_TYPE_ESP
};

// partition table cache
struct HalPartitionMap {
	enum HalPartitionFilesystemType fs_type;
	unsigned int dev;
	unsigned int begin;
	unsigned int size;
};

#define HAL_PARTITION_MAX 8

// block.c
extern struct BlockDevice hal_block_map[HAL_BLOCK_MAX];
extern struct HalPartitionMap hal_partition_map[HAL_PARTITION_MAX];
struct HalPartitionMap* hal_partition_map_insert(enum HalPartitionFilesystemType fs,
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

// gpt.c
void gpt_probe_partition(int block_id);

// display.c
struct FramebufferDriver {
	phyaddr_t (*enable)(void* private, int xres, int yres);
	void (*disable)(void* private);
	void (*update)(void* private);
	unsigned int (*read_edid)(void* private, void* buffer, unsigned int bytes);
};

void hal_display_init(void);
void hal_display_register_device(const char* name, void* private,
								 const struct FramebufferDriver* driver);

// hid.c
extern int hal_kbd_send_legacy;
void hal_hid_init(void);
void hal_mouse_update(unsigned int data);
void hal_keyboard_update(unsigned int data);

// power.c
void hal_power_init(void);
void hal_shutdown(void);
void hal_reboot(void);

#endif
