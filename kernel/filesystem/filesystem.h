#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

struct VfsPath {
	int parts;
	char* pathbuf;
};

struct FilesystemDriver {
	const char* name;
	// mounting
	int (*mount)(int partition_id);
	int (*probe)(int partition_id);
	int (*unmount)(int partition_id);
	// directory
	int (*dir_first_file)(int partition_id, unsigned int cluster);
	int (*dir_read)(int partition_id, char* buf, unsigned int cluster, unsigned int entry);
	// file
	int (*open)(int partition_id, struct VfsPath path);
	int (*create_file)(int partition_id, struct VfsPath path);
	int (*update_size)(int partition_id, struct VfsPath path, unsigned int size);
	int (*read)(int partition_id, unsigned int cluster, void* buf, unsigned int offset,
				unsigned int size);
	int (*write)(int partition_id, unsigned int cluster, const void* buf, unsigned int offset,
				 unsigned int size);
	// infomation
	int (*get_file_size)(int partition_id, struct VfsPath path);
	int (*get_file_mode)(int partition_id, struct VfsPath path);
	int (*create_directory)(int partition_id, struct VfsPath path);
	int (*remove_file)(int partition_id, struct VfsPath path);
};

extern const struct FilesystemDriver* filesystem_fat32_driver;

void filesystem_register_driver(const struct FilesystemDriver* fs_driver);
void filesystem_init(void);

#endif
