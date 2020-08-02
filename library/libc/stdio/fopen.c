/*
 * fopen function
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

#include <errno.h>
#include <panicos.h>
#include <stdio.h>
#include <stdlib.h>

FILE* fopen(const char* restrict filename, const char* restrict mode) {
	int omode;
	if (mode[0] == 'w') {
		if (mode[1] == 'x') { // wx
			omode = O_WRITE | O_CREATE;
		} else if (mode[1] == 'b') {
			if (mode[2] == 'x') { // wbx
				omode = O_WRITE | O_CREATE;
			} else if (mode[2] == '+') {
				if (mode[3] == 'x') { // wb+x
					omode = O_WRITE | O_READ | O_CREATE;
				} else { // wb+
					omode = O_WRITE | O_READ | O_CREATE | O_TRUNC;
				}
			} else { // wb
				omode = O_WRITE | O_CREATE | O_TRUNC;
			}
		} else if (mode[1] == '+') {
			if (mode[2] == 'x') { // w+x
				omode = O_WRITE | O_READ | O_CREATE;
			} else if (mode[2] == 'b') {
				if (mode[3] == 'x') { // w+bx
					omode = O_WRITE | O_READ | O_CREATE;
				} else { // w+b
					omode = O_WRITE | O_READ | O_CREATE | O_TRUNC;
				}
			} else { // w+
				omode = O_WRITE | O_READ | O_CREATE | O_TRUNC;
			}
		} else { // w
			omode = O_WRITE | O_CREATE | O_TRUNC;
		}
	} else if (mode[0] == 'r') {
		if (mode[1] == 'b') {
			if (mode[2] == '+') { // rb+
				omode = O_READ | O_WRITE;
			} else { // rb
				omode = O_READ;
			}
		} else if (mode[1] == '+') { // r+ r+b
			omode = O_READ | O_WRITE;
		} else { // r
			omode = O_READ;
		}
	} else if (mode[0] == 'a') {
		if (mode[1] == 'b') {
			if (mode[2] == '+') { // ab+
				omode = O_WRITE | O_READ | O_APPEND | O_CREATE;
			} else { // ab
				omode = O_WRITE | O_APPEND | O_CREATE;
			}
		} else if (mode[1] == '+') { // a+ a+b
			omode = O_WRITE | O_READ | O_APPEND | O_CREATE;
		} else { // a
			omode = O_WRITE | O_APPEND | O_CREATE;
		}
	} else {
		return NULL;
	}

	FILE* file;
	if ((file = malloc(sizeof(FILE))) == NULL) {
		return NULL;
	}
	file->fd = open(filename, omode);
	if (file->fd < 0) {
		errno = file->fd;
		free(file);
		return NULL;
	}
	return file;
}
