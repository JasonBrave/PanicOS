/*
 * Path string operation
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

#include <core/proc.h>
#include <defs.h>

#include "vfs.h"

int vfs_path_split(const char *path, char *buf) {
	int count = 0;
	int x = 0, y;
	for (;;) {
		if (path[x] == '\0') {
			break;
		}
		for (y = x; ((path[y] != '/') && (path[y] != '\0')); y++) {
		}
		if (y - x == 0) {
			// empty directory name
			x = y + 1;
			continue;
		}
		strncpy(buf + count * 128, path + x, y - x);
		*(buf + count * 128 + y - x) = '\0';
		count++;
		if (path[y] == '\0') {
			// end of path string
			break;
		}
		x = y + 1;
	}
	return count;
}

int vfs_path_compare(int lhs_parts, const char *lhs_buf, int rhs_parts, const char *rhs_buf) {
	if (lhs_parts != rhs_parts) {
		return 0;
	}
	for (int i = 0; i < lhs_parts; i++) {
		if (strncmp(lhs_buf + i * 128, rhs_buf + i * 128, 128) != 0) {
			return 0;
		}
	}
	return 1;
}

void vfs_path_tostring(struct VfsPath path, char *buf) {
	int next = 1;
	buf[0] = '/';
	for (int i = 0; i < path.parts; i++) {
		safestrcpy(buf + next, path.pathbuf + i * 128, 128);
		next += strlen(path.pathbuf + i * 128);
		buf[next] = '/';
		next++;
	}
	buf[next] = '\0';
}

void vfs_get_absolute_path(struct VfsPath *path) {
	char *newpath = kalloc();
#ifndef __riscv
	memmove(newpath, myproc()->cwd.pathbuf, myproc()->cwd.parts * 128);
	memmove(newpath + myproc()->cwd.parts * 128, path->pathbuf, path->parts * 128);
#endif
	kfree(path->pathbuf);
	path->pathbuf = newpath;
#ifndef __riscv
	path->parts += myproc()->cwd.parts;
#endif
}
