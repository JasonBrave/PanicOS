#include <filesystem/fat32/fat32.h>
#include <filesystem/filesystem.h>

struct FilesystemDriver fat32_driver = {
	.name = "fat32",
	.mount = fat32_mount,
	.probe = fat32_probe,
	.unmount = 0,
	.set_default_attr = fat32_set_default_attr,
	.dir_first_file = fat32_dir_first_file,
	.dir_read = fat32_dir_read,
	.open = fat32_open,
	.create_file = fat32_file_create,
	.update_size = fat32_update_size,
	.read = fat32_read,
	.write = fat32_write,
	.get_file_size = fat32_file_size,
	.get_file_mode = fat32_file_mode,
	.create_directory = fat32_mkdir,
	.remove_file = fat32_file_remove,
};

void fat32_init(void) {
	filesystem_register_driver(&fat32_driver);
}
