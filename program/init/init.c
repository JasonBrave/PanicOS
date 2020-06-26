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

#include <fcntl.h>
#include <holeos.h>
#include <stat.h>

char* argv[] = {"sh", 0};

int main(void) {
	int pid, wpid;
	for (;;) {
		printf(1, "init: starting sh\n");
		pid = fork();
		if (pid < 0) {
			printf(1, "init: fork failed\n");
			proc_exit();
		}
		if (pid == 0) {
			exec("sh", argv);
			printf(1, "init: exec sh failed\n");
			proc_exit();
		}
		while ((wpid = wait()) >= 0 && wpid != pid)
			printf(1, "zombie!\n");
	}
}
