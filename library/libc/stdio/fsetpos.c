/*
 * fsetpos function
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

int fsetpos(FILE* stream, const fpos_t* pos) {
	int ret = lseek(stream->fd, *pos, FILE_SEEK_SET);
	if (ret < 0) {
		return ret;
	}
	return 0;
}
