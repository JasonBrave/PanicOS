#include <common/errorcode.h>
#include <defs.h>
#include <memlayout.h>

#include "initramfs.h"

int initramfs_dir_open(void) {
	unsigned short* cpio_magic = (void*)INITRAMFS_BASE;
	if (*cpio_magic != 070707) {
		return ERROR_INVAILD;
	}
	return 0;
}

int initramfs_dir_read(int ino, char* name) {
	unsigned short* cpio_magic = (void*)INITRAMFS_BASE;
	if (*cpio_magic != 070707) {
		return ERROR_INVAILD;
	}

	struct cpio_binary_header* cpio = (void*)INITRAMFS_BASE + ino;
	if (strncmp((void*)cpio + sizeof(struct cpio_binary_header), "TRAILER!!!", 11) ==
		0) {
		return 0; // EOF
	}

	strncpy(name, (void*)cpio + sizeof(struct cpio_binary_header), 64);
	ino += sizeof(struct cpio_binary_header) + cpio->namesize;
	ino += (cpio->filesize[0] << 16 | cpio->filesize[1]) + cpio->namesize % 2;
	ino += cpio->filesize[1] % 2;
	return ino;
}

void initramfs_init(void) {
	unsigned short* cpio_magic = (void*)INITRAMFS_BASE;
	if (*cpio_magic == 070707) {
		cprintf("[initramfs] initramfs found\n");
	}
}
