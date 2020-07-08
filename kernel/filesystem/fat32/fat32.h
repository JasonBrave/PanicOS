#ifndef _FAT32_FAT32_H
#define _FAT32_FAT32_H

#include <filesystem/vfs/vfs.h>

#define SECTORSIZE 512

// cluster.c
int fat32_cluster_to_sector(unsigned int cluster);
int fat32_read_cluster(int partition_id, void* dest, unsigned int cluster,
					   unsigned int begin, unsigned int size);

// dir.c
int fat32_open(int partition_id, struct VfsPath path);
int fat32_dir_first_file(int partition_id, unsigned int cluster);
int fat32_dir_read(int partition_id, char* buf, unsigned int cluster,
				   unsigned int entry);
int fat32_file_size(int partition_id, struct VfsPath path);

// fat.c
unsigned int fat32_fat_read(int partition_id, unsigned int current);
unsigned int fat32_offset_cluster(int partition_id, unsigned int cluster,
								  unsigned int offset);

// mount.c
int fat32_mount(int partition_id);

#endif
