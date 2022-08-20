/*
 * Filesystem driver support
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

#include <defs.h>
#include <filesystem/fat32/fat32.h>
#include <filesystem/filesystem.h>
#include <filesystem/vfs/vfs.h>

const struct FilesystemDriver* filesystem_fat32_driver;

void filesystem_register_driver(const struct FilesystemDriver* fs_driver) {
	if (strncmp(fs_driver->name, "fat32", 10) == 0) {
		filesystem_fat32_driver = fs_driver;
	}
}

void filesystem_init(void) {
	fat32_init();
	vfs_init();
}
