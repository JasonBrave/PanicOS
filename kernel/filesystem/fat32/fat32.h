#ifndef _FAT32_FAT32_H
#define _FAT32_FAT32_H

#include <filesystem/vfs/vfs.h>

#define SECTORSIZE 512

// cluster.c
int fat32_cluster_to_sector(unsigned int cluster);
int fat32_read_cluster(int partition_id, void* dest, unsigned int cluster,
					   unsigned int begin, unsigned int size);
int fat32_read(int partition_id, unsigned int cluster, void* buf, unsigned int offset,
			   unsigned int size);
int fat32_write_cluster(int partition_id, const void* src, unsigned int cluster,
						unsigned int begin, unsigned int size);
unsigned int fat32_allocate_cluster(int partition_id);

// dir.c
int fat32_open(int partition_id, struct VfsPath path);
int fat32_dir_first_file(int partition_id, unsigned int cluster);
int fat32_dir_read(int partition_id, char* buf, unsigned int cluster,
				   unsigned int entry);
int fat32_file_size(int partition_id, struct VfsPath path);
int fat32_file_mode(int partition_id, struct VfsPath path);
int fat32_mkdir(int partition_id, struct VfsPath path);

// fat.c
unsigned int fat32_fat_read(int partition_id, unsigned int current);
unsigned int fat32_offset_cluster(int partition_id, unsigned int cluster,
								  unsigned int offset);
int fat32_write_fat(int partition_id, unsigned int cluster, unsigned int data);
int fat32_append_cluster(int partition_id, unsigned int begin_cluster,
						 unsigned int end_cluster);

// mount.c
int fat32_mount(int partition_id);

#endif
