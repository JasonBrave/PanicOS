/*
 * strerror function
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

#include <errorcode.h>

static char* error_string[] = {[-ERROR_INVAILD] = "Invaild arguments",
							   [-ERROR_NOT_EXIST] = "File does not exist",
							   [-ERROR_EXIST] = "File already exists",
							   [-ERROR_NOT_FILE] = "Not a file",
							   [-ERROR_NOT_DIRECTORY] = "Not a directory",
							   [-ERROR_READ_FAIL] = "Disk read fail",
							   [-ERROR_OUT_OF_SPACE] = "Filesystem out of space",
							   [-ERROR_WRITE_FAIL] = "Disk write fail",
							   [-ERROR_NO_PERM] = "Permission denied"

};

char* strerror(int errnum) {
	return error_string[-errnum];
}
