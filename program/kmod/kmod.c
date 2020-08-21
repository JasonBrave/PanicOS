/*
 * Kernel module manager
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

#include <panicos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void load_module(const char* name) {
	char module_path[512];
	strcpy(module_path, "/boot/module/");
	strcat(module_path, name);
	strcat(module_path, ".mod");
	int ret = module_load(module_path);
	if (ret < 0) {
		printf("load %s failed with %d\n", name, ret);
		exit(0);
	}
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		puts("kmod mod_name(s)");
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		load_module(argv[i]);
	}
	return 0;
}
