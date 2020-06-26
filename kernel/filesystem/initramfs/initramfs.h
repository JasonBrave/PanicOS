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
};

void initramfs_init(void);
int initramfs_dir_open(void);
int initramfs_dir_read(int ino, char* name);

#endif
