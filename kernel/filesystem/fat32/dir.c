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

static int fat32_dir_search(int partition_id, unsigned int cluster, const char* name,
							struct FAT32DirEntry* dir_dest) {
	for (int i = 0; i < SECTORSIZE * fat32_superblock->sector_per_cluster; i += 32) {
		struct FAT32DirEntry dir;
		if (fat32_read_cluster(partition_id, &dir, cluster, i, sizeof(dir)) < 0) {
			return ERROR_READ_FAIL;
		}
		if (dir.name[0] == 0xe5) {
			continue;
		} else if (dir.name[0] == 0) {
			break;
		} else if (dir.attr == ATTR_LONG_NAME) {
			continue;
		} else if (dir.attr == ATTR_VOLUME_ID) {
			continue;
		}
		char fullname[256];
		fat32_get_full_name(&dir, fullname);
		if (strncmp(name, fullname, 256) == 0) {
			memmove(dir_dest, &dir, sizeof(struct FAT32DirEntry));
			return i / 32;
		}
	}
	return ERROR_NOT_EXIST;
}

static int fat32_path_search(int partition_id, struct VfsPath path,
							 struct FAT32DirEntry* dir_dest) {
	unsigned int cluster = fat32_superblock->root_cluster;
	for (int i = 0; i < path.parts; i++) {
		int errc;
		if ((errc = fat32_dir_search(partition_id, cluster, path.pathbuf + i * 128,
									 dir_dest)) < 0) {
			return errc;
		}
		cluster = (dir_dest->cluster_hi << 16) | dir_dest->cluster_lo;
	}
	return 0;
}

int fat32_open(int partition_id, struct VfsPath path) {
	if (path.parts == 0) {
		return fat32_superblock->root_cluster;
	}
	struct FAT32DirEntry dir;
	int errc;
	dir.cluster_hi = dir.cluster_lo = 0;
	if ((errc = fat32_path_search(partition_id, path, &dir)) < 0) {
		return errc;
	}
	return (dir.cluster_hi << 16) | dir.cluster_lo;
}

int fat32_dir_first_file(int partition_id, unsigned int cluster) {
	for (int i = 0; i < SECTORSIZE * fat32_superblock->sector_per_cluster; i += 32) {
		struct FAT32DirEntry dir;
		int errc = fat32_read_cluster(partition_id, &dir, cluster, i, sizeof(dir));
		if (errc < 0) {
			return errc;
		}
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

int fat32_dir_read(int partition_id, char* buf, unsigned int cluster,
				   unsigned int entry) {
	for (int i = entry * 32; i < SECTORSIZE * fat32_superblock->sector_per_cluster;
		 i += 32) {
		struct FAT32DirEntry dir;
		int errc = fat32_read_cluster(partition_id, &dir, cluster, i, sizeof(dir));
		if (errc < 0) {
			return errc;
		}
		if (dir.name[0] == 0xe5) {
			continue;
		} else if (dir.name[0] == 0) {
			return 0;
		} else if (dir.attr == ATTR_LONG_NAME) {
			continue;
		} else if (dir.attr == ATTR_VOLUME_ID) {
			continue;
		}
		fat32_get_full_name(&dir, buf);
		return i / 32 + 1;
	}
	return 0;
}
