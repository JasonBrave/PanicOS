#ifndef _FAT32_FAT32_H
#define _FAT32_FAT32_H

#include <filesystem/vfs/vfs.h>

// mount.c
int fat32_mount(int partition_id);

// dir.c
int fat32_open(int partition_id, struct VfsPath path);
int fat32_dir_first_file(int partition_id, unsigned int cluster);
int fat32_dir_read(int partition_id, char* buf, unsigned int cluster,
				   unsigned int entry);

#endif
