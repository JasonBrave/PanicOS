/*
 * fseek function
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
#include <stdio.h>

int fseek(FILE* stream, long int offset, int whence) {
	int lsk;
	if (whence == SEEK_CUR) {
		lsk = FILE_SEEK_CUR;
	} else if (whence == SEEK_END) {
		lsk = FILE_SEEK_END;
	} else if (whence == SEEK_SET) {
		lsk = FILE_SEEK_SET;
	} else {
		return -1;
	}
	if (lseek(stream->fd, offset, lsk) < 0) {
		return -1;
	}
	return 0;
}
