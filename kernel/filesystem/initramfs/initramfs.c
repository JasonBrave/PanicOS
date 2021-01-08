/*
 * Initramfs driver
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/errorcode.h>
#include <defs.h>
#include <memlayout.h>

#include "initramfs.h"

static struct cpio_binary_header* initramfs_search(const char* filename) {
	struct cpio_binary_header* cpio = (void*)INITRAMFS_BASE;
	while (strncmp((void*)cpio + sizeof(struct cpio_binary_header), "TRAILER!!!", 11)) {
		if (strncmp((void*)cpio + sizeof(struct cpio_binary_header), filename, 64) == 0) {
			return cpio;
		}
		cpio = (void*)cpio + sizeof(struct cpio_binary_header) + cpio->namesize +
			   (cpio->filesize[0] << 16 | cpio->filesize[1]) + cpio->namesize % 2 +
			   cpio->filesize[1] % 2;
	}
	return 0; // not found
}

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
	if (strncmp((void*)cpio + sizeof(struct cpio_binary_header), "TRAILER!!!", 11) == 0) {
		return 0; // EOF
	}

	strncpy(name, (void*)cpio + sizeof(struct cpio_binary_header), 64);
	ino += sizeof(struct cpio_binary_header) + cpio->namesize;
	ino += (cpio->filesize[0] << 16 | cpio->filesize[1]) + cpio->namesize % 2;
	ino += cpio->filesize[1] % 2;
	return ino;
}

int initramfs_init(void) {
	unsigned short* cpio_magic = (void*)INITRAMFS_BASE;
	if (*cpio_magic == 070707) {
		cprintf("[initramfs] initramfs found\n");
		return 0;
	} else {
		return ERROR_NOT_EXIST;
	}
}

int initramfs_file_get_size(const char* filename) {
	struct cpio_binary_header* cpio = initramfs_search(filename);
	if (!cpio) {
		return ERROR_NOT_EXIST;
	}
	return cpio->filesize[0] << 16 | cpio->filesize[1];
}

int initramfs_open(const char* filename) {
	struct cpio_binary_header* cpio = initramfs_search(filename);
	if (!cpio) {
		return ERROR_NOT_EXIST;
	}
	return (unsigned int)cpio - INITRAMFS_BASE;
}

// return bytes read
int initramfs_read(unsigned int block, void* buf, unsigned int offset, unsigned int size) {
	const struct cpio_binary_header* cpio = (void*)INITRAMFS_BASE + block;
	const char* data =
		(void*)cpio + sizeof(struct cpio_binary_header) + cpio->namesize + cpio->namesize % 2;

	unsigned int copysize;
	if (offset + size > (unsigned int)(cpio->filesize[0] << 16 | cpio->filesize[1])) {
		copysize = (cpio->filesize[0] << 16 | cpio->filesize[1]) - offset;
	} else {
		copysize = size;
	}
	if (copysize <= 0) {
		return 0;
	}
	memmove(buf, data + offset, copysize);
	return copysize;
}

int initramfs_file_get_mode(const char* filename) {
	struct cpio_binary_header* cpio = initramfs_search(filename);
	if (!cpio) {
		return ERROR_NOT_EXIST;
	}
	return cpio->mode;
}
