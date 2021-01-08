/*
 * Terminal emulator
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

#include <libwm/keymap.h>
#include <libwm/wm.h>
#include <panicos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static COLOUR term_back_colour = {0, 0, 0};
static COLOUR term_font_colour = {255, 255, 255};

int x_chars = 80, y_chars = 25;
char* term_buffer; // terminal buffer
int cur_x = 0, cur_y = 0; // cursor
char inputbuf[80]; // input buffer
int inputptr = 0;

static void print_help_message(void) {
	fputs("Usage: termemu [program] [x_chars y_chars]\n", stderr);
	exit(1);
}

int main(int argc, char* argv[]) {
	const char* prog = "/bin/sh";
	if (argc == 2) {
		if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
			print_help_message();
		}
		prog = argv[1];
	} else if (argc == 4) {
		prog = argv[1];
		x_chars = atoi(argv[2]);
		y_chars = atoi(argv[3]);
	} else if (argc == 3 || argc > 4) {
		print_help_message();
	}
	// draw the window
	if (!wm_init()) {
		fputs("Cannot connect to Window Manager\n", stderr);
		exit(EXIT_FAILURE);
	}
	int term_handle = wm_create_window(x_chars * 8, y_chars * 16);
	wm_window_set_title(term_handle, "Terminal");
	wm_fill_sheet(term_handle, term_back_colour);
	// create the Pseudoterminal
	int pty = pty_create();
	if (pty < 0) {
		fputs("termemu: pty create failed\n", stderr);
		abort();
	}
	// allocate the terminal buffer
	term_buffer = malloc(x_chars * y_chars);
	// spawn the shell process
	int sh_pid = fork();
	if (sh_pid == 0) {
		pty_switch(pty);
		const char* args[] = {"sh", 0};
		exec(prog, args);
		abort();
	}

	int upper = 0;
	for (;;) {
		int sh_exit_status;
		if (proc_status(sh_pid, &sh_exit_status) == PROC_EXITED) {
			pty_close(pty);
			wm_remove_sheet(term_handle);
			exit(0);
		}

		int need_update = 0;
		char buf[1024];
		int n = pty_read_output(pty, buf, 1024);

		if (n) {
			need_update = 1;
			for (int i = 0; i < n; i++) {
				if (buf[i] == '\n') {
					term_buffer[cur_y * x_chars + cur_x] = ' ';
					cur_x = 0;
					cur_y++;
					if (cur_y == y_chars) {
						for (int j = 0; j < y_chars - 1; j++) {
							memcpy(term_buffer + j * x_chars, term_buffer + (j + 1) * x_chars,
								   x_chars);
						}
						memset(term_buffer + (y_chars - 1) * x_chars, ' ', x_chars);
						cur_y--;
					}
				} else {
					term_buffer[cur_y * x_chars + cur_x] = buf[i];
					cur_x++;
					if (cur_x >= x_chars) {
						cur_x = 0;
						cur_y++;
						if (cur_y == y_chars) {
							for (int j = 0; j < y_chars - 1; j++) {
								memcpy(term_buffer + j * x_chars, term_buffer + (j + 1) * x_chars,
									   x_chars);
							}
							memset(term_buffer + (y_chars - 1) * x_chars, ' ', x_chars);
							cur_y--;
						}
					}
				}
			}
		}

		struct WmEvent event;
		int event_cached = wm_catch_event(&event);
		if (event_cached && event.event_type == WM_EVENT_KEY_DOWN) {
			need_update = 1;
			if (event.keycode == 8) { // backspace
				if (inputptr) {
					term_buffer[cur_y * x_chars + cur_x] = ' ';
					cur_x--;
					inputptr--;
				}
			} else if (event.keycode == 27) { // ESC
				kill(sh_pid);
				pty_close(pty);
				wm_remove_sheet(term_handle);
				exit(0);
			} else if (event.keycode == 13) { // enter
				inputbuf[inputptr] = '\n';
				inputptr++;
				pty_write_input(pty, inputbuf, inputptr);
				inputptr = 0;
				term_buffer[cur_y * x_chars + cur_x] = ' ';
				cur_x = 0;
				cur_y++;
				if (cur_y == y_chars) {
					for (int j = 0; j < y_chars - 1; j++) {
						memcpy(term_buffer + j * x_chars, term_buffer + (j + 1) * x_chars, x_chars);
					}
					memset(term_buffer + (y_chars - 1) * x_chars, ' ', x_chars);
					cur_y--;
				}
			} else if (event.keycode == 16) { // shift
				upper = 1;
			} else {
				if (upper) {
					term_buffer[cur_y * x_chars + cur_x] = keymap_upper[event.keycode];
					cur_x++;
					inputbuf[inputptr] = keymap_upper[event.keycode];
					inputptr++;
				} else {
					term_buffer[cur_y * x_chars + cur_x] = keymap_lower[event.keycode];
					cur_x++;
					inputbuf[inputptr] = keymap_lower[event.keycode];
					inputptr++;
				}
			}
		}
		if (event_cached && event.event_type == WM_EVENT_KEY_UP) {
			if (event.keycode == 16) { // shift
				upper = 0;
			}
		} else if (event_cached && event.event_type == WM_EVENT_WINDOW_CLOSE) { // window closed
			kill(sh_pid);
			pty_close(pty);
			wm_remove_sheet(term_handle);
			exit(0);
		}

		term_buffer[cur_y * x_chars + cur_x] = '_';

		if (need_update) {
			wm_fill_sheet(term_handle, term_back_colour);
			for (int i = 0; i < y_chars; i++) {
				wm_print_text_n(term_handle, 0, i * 16, term_font_colour, term_buffer + i * x_chars,
								x_chars);
			}
		}
	}
}
