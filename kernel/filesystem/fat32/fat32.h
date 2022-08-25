#ifndef _FAT32_FAT32_H
#define _FAT32_FAT32_H

#include <filesystem/vfs/vfs.h>

#define SECTORSIZE 512

struct FAT32Private {
	unsigned int partition_id;
	struct FAT32BootSector* boot_sector;
	unsigned int mode, uid, gid;
};

// cluster.c
int fat32_cluster_to_sector(struct FAT32Private* priv, unsigned int cluster);
int fat32_read_cluster(struct FAT32Private* priv, void* dest, unsigned int cluster,
					   unsigned int begin, unsigned int size);
int fat32_read(void* private, unsigned int cluster, void* buf, unsigned int offset,
			   unsigned int size);
int fat32_write_cluster(struct FAT32Private* priv, const void* src, unsigned int cluster,
						unsigned int begin, unsigned int size);
unsigned int fat32_allocate_cluster(struct FAT32Private* priv);
int fat32_write(void* private, unsigned int cluster, const void* buf, unsigned int offset,
				unsigned int size);

// dir.c
int fat32_open(void* private, struct VfsPath path);
int fat32_dir_first_file(void* private, unsigned int cluster);
int fat32_dir_read(void* private, char* buf, unsigned int cluster, unsigned int entry);
int fat32_file_size(void* private, struct VfsPath path);
int fat32_file_mode(void* private, struct VfsPath path);
int fat32_mkdir(void* private, struct VfsPath path);
int fat32_file_remove(void* private, struct VfsPath path);
int fat32_update_size(void* private, struct VfsPath path, unsigned int size);
int fat32_file_create(void* private, struct VfsPath path);

// fat.c
unsigned int fat32_fat_read(struct FAT32Private* priv, unsigned int current);
unsigned int fat32_offset_cluster(struct FAT32Private* priv, unsigned int cluster,
								  unsigned int offset);
int fat32_write_fat(struct FAT32Private* priv, unsigned int cluster, unsigned int data);
int fat32_append_cluster(struct FAT32Private* priv, unsigned int begin_cluster,
						 unsigned int end_cluster);
int fat32_free_chain(struct FAT32Private* priv, unsigned int cluster);

// mount.c
int fat32_mount(int partition_id, void** private);
int fat32_probe(int partition_id);
void fat32_set_default_attr(void* private, unsigned int uid, unsigned int gid, unsigned int mode);

// fat32.c
void fat32_init(void);

#endif
