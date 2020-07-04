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
