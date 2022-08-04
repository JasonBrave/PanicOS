/*
 * ls program
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

#include <dirent.h>
#include <panicos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	char* dir_target;
	char cwd[64];
	getcwd(cwd);
	if (argc == 1) {
		dir_target = cwd;
	} else {
		dir_target = argv[1];
	}
	DIR* dir = opendir(dir_target);
	if (!dir) {
		printf("opendir failed\n");
		return 1;
	}
	struct dirent* dirent;
	while ((dirent = readdir(dir)) != NULL) {
		printf("%s ", dirent->d_name);
	}
	closedir(dir);
	putchar('\n');
}
