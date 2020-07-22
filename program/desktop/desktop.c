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

#define DEFAULT_XRES 1024
#define DEFAULT_YRES 768

int xres, yres;

int main(int argc, char* argv[]) {
	char* wm_args[] = {"wm", 0, 0, 0};
	if (argc == 1) {
		xres = DEFAULT_XRES;
		yres = DEFAULT_YRES;
		wm_args[1] = 0;
	} else if (argc == 3) {
		xres = atoi(argv[1]);
		yres = atoi(argv[2]);
		wm_args[1] = argv[1];
		wm_args[2] = argv[2];
	} else {
		fputs("Usage: desktop [xres yres]\n", stderr);
		exit(EXIT_FAILURE);
	}

	static COLOUR blue = {0, 0, 255};
	static COLOUR gray = {200, 200, 200};
	// spawn window manager
	if (!wm_init()) {
		int wmpid = fork();
		if (wmpid == 0) {
			exec("/bin/wm", wm_args);
			abort();
		}
		// wait for window manager to finish initialization
		while (!wm_init()) {
		}
	}

	int toolbar = wm_create_sheet(0, yres - 32, xres, 32);
	wm_fill_sheet(toolbar, gray);
	wm_print_text(toolbar, 8, 8, blue, "PanicOS");
	int window = wm_create_window(200, 500);
	wm_window_set_title(window, "Files");

	// event loop
	struct WmEvent event;
	int event_catched = 0;
	for (;;) {
		event_catched = wm_wait_event(&event);
		if (event_catched && event.handle == window &&
			event.event_type == WM_EVENT_MOUSE_BUTTON_DOWN) {
			puts("clicked");
		} else if (event_catched) {
			puts("other event");
		}
	}
	fputs("Window Manager exited\n", stderr);
	return 1;
}
