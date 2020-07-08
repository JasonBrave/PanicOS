#ifndef _INITRAMFS_H
#define _INITRAMFS_H

#include <common/types.h>

struct cpio_binary_header {
	uint16_t magic;
	uint16_t dev;
	uint16_t ino;
	uint16_t mode;
	uint16_t uid;
	uint16_t gid;
	uint16_t nlink;
	uint16_t rdev;
	uint16_t mtime[2];
	uint16_t namesize;
	uint16_t filesize[2];
} PACKED;

int initramfs_init(void);
int initramfs_dir_open(void);
int initramfs_dir_read(int ino, char* name);
int initramfs_file_get_size(const char* filename);
int initramfs_open(const char* filename);
int initramfs_read(unsigned int block, void* buf, unsigned int offset,
				   unsigned int size);
int initramfs_file_get_mode(const char* filename);

#endif
