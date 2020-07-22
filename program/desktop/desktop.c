/*
 * Desktop environment
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

#include <libwm/wm.h>
#include <panicos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	static COLOUR blue = {0, 0, 255};
	static COLOUR gray = {200, 200, 200};
	// spawn window manager
	if (!wm_init()) {
		int wmpid = fork();
		if (wmpid == 0) {
			char* args[] = {"wm", 0};
			exec("/bin/wm", args);
			abort();
		}
		// wait for window manager to finish initialization
		while (!wm_init()) {
		}
	}

	int toolbar = wm_create_sheet(0, 736, 1024, 32);
	wm_fill_sheet(toolbar, gray);
	wm_print_text(toolbar, 8, 8, blue, "PanicOS");
	int window = wm_create_window(200, 500);
	wm_window_set_title(window, "Files");

	// event loop
	for (;;) {
	}
	fputs("Window Manager exited\n", stderr);
	return 1;
}
