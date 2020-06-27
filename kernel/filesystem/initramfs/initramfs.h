#ifndef _INITRAMFS_H
#define _INITRAMFS_H

struct cpio_binary_header {
	unsigned short magic;
	unsigned short dev;
	unsigned short ino;
	unsigned short mode;
	unsigned short uid;
	unsigned short gid;
	unsigned short nlink;
	unsigned short rdev;
	unsigned short mtime[2];
	unsigned short namesize;
	unsigned short filesize[2];
} __attribute__((packed));

void initramfs_init(void);
int initramfs_dir_open(void);
int initramfs_dir_read(int ino, char* name);
int initramfs_file_get_size(const char* filename);
int initramfs_open(const char* filename);
int initramfs_read(unsigned int block, void* buf, unsigned int offset,
				   unsigned int size);

#endif
