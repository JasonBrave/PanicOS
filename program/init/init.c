/*
 * Init process
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

char* argv[] = {"sh", 0};

int main(void) {
	int pid, wpid;
	for (;;) {
		puts("init: starting sh");
		pid = fork();
		if (pid < 0) {
			fputs("init: fork failed\n", stderr);
			return 1;
		}
		if (pid == 0) {
			exec("sh", argv);
			fputs("init: exec sh failed\n", stderr);
			return 1;
		}
		while ((wpid = wait()) >= 0 && wpid != pid)
			fputs("init: zombie!\n", stderr);
	}
}
