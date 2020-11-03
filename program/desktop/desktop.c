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

#include <dirent.h>
#include <kcall/display.h>
#include <libwm/wm.h>
#include <panicos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int xres, yres;

int main(int argc, char* argv[]) {
	const char* wm_args[] = {"wm", 0, 0, 0, 0};
	if (argc == 1) { // desktop
		int display_id = display_find();
		if (display_id < 0) {
			fputs("desktop: no display device\n", stderr);
			exit(EXIT_FAILURE);
		}
		display_get_preferred(display_id, &xres, &yres);
	} else if (argc == 2) { // desktop display_id
		int display_id = atoi(argv[1]);
		if (display_get_preferred(display_id, &xres, &yres) < 0) {
			fputs("desktop: display device not found\n", stderr);
			exit(EXIT_FAILURE);
		}
		wm_args[1] = argv[1];
	} else if (argc == 3) { // desktop xres yres
		int display_id = display_find();
		if (display_id < 0) {
			fputs("desktop: no display device\n", stderr);
			exit(EXIT_FAILURE);
		}
		xres = atoi(argv[1]);
		yres = atoi(argv[2]);
		wm_args[1] = argv[1];
		wm_args[2] = argv[2];
	} else if (argc == 4) { // desktop display_id xres yres
		xres = atoi(argv[2]);
		yres = atoi(argv[3]);
		wm_args[1] = argv[1];
		wm_args[2] = argv[2];
		wm_args[3] = argv[3];
	} else {
		fputs("Usage:\n", stderr);
		fputs(" desktop - default display and resolution", stderr);
		fputs(" desktop display_id - custom display and default resolution", stderr);
		fputs(" desktop xres yres - default display and custom resolution", stderr);
		fputs(" desktop display_id xres yres - custom display and resolution", stderr);
		exit(EXIT_FAILURE);
	}

	static COLOUR blue = {0, 0, 255, 0};
	static COLOUR gray = {200, 200, 200, 0};
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

	DIR* dir = opendir("/bin");
	struct dirent* diren;
	int filename_y = 2;
	while ((diren = readdir(dir)) != NULL) {
		char fullname[32] = "/bin/";
		strcat(fullname, diren->d_name);
		if (file_get_mode(fullname) & 0040000) {
			continue;
		}
		wm_print_text(window, 2, filename_y, blue, diren->d_name);
		filename_y += 20;
	}
	closedir(dir);

	// event loop
	struct WmEvent event;
	int event_catched = 0;
	while (1) {
		event_catched = wm_wait_event(&event);

		if (event_catched && event.handle == window &&
			event.event_type == WM_EVENT_MOUSE_BUTTON_DOWN) {
			DIR* fmdir = opendir("/bin");
			struct dirent* file;
			int cnt = 0;
			while ((file = readdir(fmdir)) != NULL) {
				char fullname[32] = "/bin/";
				strcat(fullname, file->d_name);
				if (file_get_mode(fullname) & 0040000) {
					continue;
				}
				if (event.y / 20 == cnt) {
					int childpid = fork();
					if (childpid == 0) {
						const char* args[] = {file->d_name, 0};
						exec(fullname, args);
						abort();
					}
					break;
				}
				cnt++;
			}
			closedir(fmdir);
		}
	}
	fputs("Window Manager exited\n", stderr);
	return 1;
}
