#include <common/errorcode.h>
#include <defs.h>
#include <filesystem/vfs/vfs.h>
#include <hal/hal.h>

#include "fat32-struct.h"

#define SECTORSIZE 512

static int fat32_cluster_to_sector(unsigned int cluster) {
	return (cluster - 2) * (fat32_superblock->sector_per_cluster) +
		   (fat32_superblock->reserved_sector) +
		   (fat32_superblock->fat_number) * (fat32_superblock->fat_size);
}

static int fat32_read_cluster(int partition_id, void* dest, unsigned int cluster,
							  unsigned int begin, unsigned int size) {
	void* sect = kalloc();
	unsigned int off = 0;
	while (off < size) {
		int copysize;
		if ((begin + off) / SECTORSIZE < (begin + size) / SECTORSIZE) {
			copysize = ((begin + off) / SECTORSIZE + 1) * SECTORSIZE - (begin + off);
		} else {
			copysize = size - off;
		}
		unsigned int sector =
			fat32_cluster_to_sector(cluster) + (begin + off) / SECTORSIZE;
		if (hal_partition_read(partition_id, sector, 1, sect) < 0) {
			kfree(sect);
			return ERROR_READ_FAIL;
		}
		memmove(dest + off, sect + (begin + off) % SECTORSIZE, copysize);
		off += copysize;
	}
	kfree(sect);
	return 0;
}

int fat32_open(int partition_id, struct VfsPath path) {
	if (path.parts != 0) {
		return ERROR_NOT_EXIST;
	}
	return fat32_superblock->root_cluster;
}

int fat32_dir_first_file(int partition_id, unsigned int cluster) {
	for (int i = 0; i < SECTORSIZE * fat32_superblock->sector_per_cluster; i += 32) {
		struct FAT32DirEntry dir;
		fat32_read_cluster(partition_id, &dir, cluster, i, sizeof(dir));
		if (dir.name[0] == 0xe5) {
			continue;
		} else if (dir.name[0] == 0) {
			break;
		} else if (dir.attr == ATTR_LONG_NAME) {
			continue;
		} else if (dir.attr == ATTR_VOLUME_ID) {
			continue;
		}
		return i / 32;
	}
	return -1;
}

int fat32_dir_next_file(int partition_id, unsigned int cluster, unsigned int previous) {
	for (int i = previous * 32 + 32;
		 i < SECTORSIZE * fat32_superblock->sector_per_cluster; i += 32) {
		struct FAT32DirEntry dir;
		fat32_read_cluster(partition_id, &dir, cluster, i, sizeof(dir));
		if (dir.name[0] == 0xe5) {
			continue;
		} else if (dir.name[0] == 0) {
			return 0;
		} else if (dir.attr == ATTR_LONG_NAME) {
			continue;
		} else if (dir.attr == ATTR_VOLUME_ID) {
			continue;
		}
		return i / 32;
	}
	return 0;
}

static char* fat32_get_full_name(const struct FAT32DirEntry* dir, char* name) {
	// copy name, change to lower-case
	memmove(name, dir->name, 11);
	for (int i = 0; i < 11; i++) {
		if ((name[i] >= 'A') && (name[i] <= 'Z')) {
			name[i] += 'a' - 'A';
		}
	}
	// move extension to the right, reserve space for dot
	name[11] = name[10];
	name[10] = name[9];
	name[9] = name[8];
	// remove space at end of name
	unsigned int off = 7;
	while (name[off] == ' ') {
		off--;
	}
	// check if extension is empty, if so, do not put dot
	if (name[9] == ' ') {
		name[off + 1] = '\0';
		return name;
	}
	name[off + 1] = '.';
	for (int i = 2; i < 5; i++) {
		// copy extension,remove space at the end
		if (name[i + 7] == ' ') {
			name[off + i] = '\0';
			return name;
		}
		name[off + i] = name[i + 7];
	}
	name[off + 5] = '\0';
	return name;
}

char* fat32_dir_name(int partition_id, char* buf, unsigned int cluster,
					 unsigned int entry) {
	struct FAT32DirEntry dir;
	fat32_read_cluster(partition_id, &dir, cluster, entry * 32, sizeof(dir));
	fat32_get_full_name(&dir, buf);
	return buf;
}
