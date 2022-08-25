#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

struct VfsPath {
	int parts;
	char* pathbuf;
};

struct FilesystemDriver {
	const char* name;
	// mounting
	int (*mount)(int partition_id, void** private);
	int (*probe)(int partition_id);
	int (*unmount)(void* private);
	void (*set_default_attr)(void* private, unsigned int uid, unsigned int gid, unsigned int mode);
	// directory
	int (*dir_first_file)(void* private, unsigned int cluster);
	int (*dir_read)(void* private, char* buf, unsigned int cluster, unsigned int entry);
	// file
	int (*open)(void* private, struct VfsPath path);
	int (*create_file)(void* private, struct VfsPath path);
	int (*update_size)(void* private, struct VfsPath path, unsigned int size);
	int (*read)(void* private, unsigned int cluster, void* buf, unsigned int offset,
				unsigned int size);
	int (*write)(void* private, unsigned int cluster, const void* buf, unsigned int offset,
				 unsigned int size);
	// infomation
	int (*get_file_size)(void* private, struct VfsPath path);
	int (*get_file_mode)(void* private, struct VfsPath path);
	int (*create_directory)(void* private, struct VfsPath path);
	int (*remove_file)(void* private, struct VfsPath path);
};

extern const struct FilesystemDriver* filesystem_fat32_driver;

void filesystem_register_driver(const struct FilesystemDriver* fs_driver);
void filesystem_init(void);

#endif
