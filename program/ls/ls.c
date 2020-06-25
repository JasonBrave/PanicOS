/*
 * ls program
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <holeos.h>
#include <stat.h>

#define DIRSIZ 14

struct dirent {
	unsigned short inum;
	char name[DIRSIZ];
};

char* fmtname(char* path) {
	static char buf[DIRSIZ + 1];
	char* p;

	// Find first character after last slash.
	for (p = path + strlen(path); p >= path && *p != '/'; p--)
		;
	p++;

	// Return blank-padded name.
	if (strlen(p) >= DIRSIZ)
		return p;
	memmove(buf, p, strlen(p));
	memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
	return buf;
}

void ls(char* path) {
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if ((fd = open(path, 0)) < 0) {
		printf(2, "ls: cannot open %s\n", path);
		return;
	}

	if (fstat(fd, &st) < 0) {
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch (st.type) {
	case T_FILE:
		printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
		break;

	case T_DIR:
		if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
			printf(1, "ls: path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf + strlen(buf);
		*p++ = '/';
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if (stat(buf, &st) < 0) {
				printf(1, "ls: cannot stat %s\n", buf);
				continue;
			}
			printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
		}
		break;
	}
	close(fd);
}

int main(int argc, char* argv[]) {
	int i;

	if (argc < 2) {
		ls(".");
		proc_exit();
	}
	for (i = 1; i < argc; i++)
		ls(argv[i]);
	proc_exit();
}
