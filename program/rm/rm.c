/*
 * rm program
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

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		fputs("usage: rm [file]\n", stderr);
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		int ret;
		if ((ret = unlink(argv[i])) < 0) {
			printf("remove %s failed, unlink returned %d\n", argv[i], ret);
			return 0;
		}
	}
	return 0;
}
