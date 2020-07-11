/*
 * cat program
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

#include <stdio.h>
#include <string.h>

void print_file(FILE* file) {
	char buf[256];
	while (fgets(buf, 256, file) != NULL) {
		fputs(buf, stdout);
	}
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		// no command line arguments
		print_file(stdin);
	} else {
		for (int i = 1; i < argc; i++) {
			// display file one by one
			if (strcmp(argv[i], "-") == 0) {
				print_file(stdin);
			} else {
				FILE* file = fopen(argv[i], "r");
				if (file == NULL) {
					fputs("File not found\n", stderr);
					return 1;
				}
				print_file(file);
				fclose(file);
			}
		}
	}
}
