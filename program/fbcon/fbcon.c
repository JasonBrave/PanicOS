/*
 * Framebuffer console
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

#include <kcall/display.h>
#include <libwm/keymap.h>
#include <panicos.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned char font16[256 * 16];

#define BLACK 0
#define WHITE 0xffffff

unsigned int xres, yres;
int x_chars, y_chars;
int cur_x = 0, cur_y = 0;
char inputbuf[80]; // input buffer
int inputptr = 0;
uint32_t* fb;
int need_update = 0; // need manual call update
int display_id;
int shift = 0; // holding shift key

static inline void fastmemcpy32(void* dest, const void* src, int cnt) {
	uint32_t* d = dest;
	const uint32_t* s = src;
	for (int i = 0; i < cnt; i++) {
		d[i] = s[i];
	}
}

static inline void fastmemset32(void* dest, uint32_t val, int cnt) {
	uint32_t* d = dest;
	for (int i = 0; i < cnt; i++) {
		d[i] = val;
	}
}

static void fbcon_drawchar(int x, int y, char c) {
	for (int i = 0; i < 16; i++) {
		uint32_t* p = fb + (y + i) * xres + x;
		if (font16[c * 16 + i] & 1) {
			p[7] = WHITE;
		} else {
			p[7] = BLACK;
		}
		if (font16[c * 16 + i] & 2) {
			p[6] = WHITE;
		} else {
			p[6] = BLACK;
		}
		if (font16[c * 16 + i] & 4) {
			p[5] = WHITE;
		} else {
			p[5] = BLACK;
		}
		if (font16[c * 16 + i] & 8) {
			p[4] = WHITE;
		} else {
			p[4] = BLACK;
		}
		if (font16[c * 16 + i] & 16) {
			p[3] = WHITE;
		} else {
			p[3] = BLACK;
		}
		if (font16[c * 16 + i] & 32) {
			p[2] = WHITE;
		} else {
			p[2] = BLACK;
		}
		if (font16[c * 16 + i] & 64) {
			p[1] = WHITE;
		} else {
			p[1] = BLACK;
		}
		if (font16[c * 16 + i] & 128) {
			p[0] = WHITE;
		} else {
			p[0] = BLACK;
		}
	}
}

static void fbcon_putchar(int x, int y, char c) {
	return fbcon_drawchar(x * 8, y * 16, c);
}

int main(int argc, char* argv[]) {
	if (argc == 1) { // fbcon
		display_id = display_find();
		if (display_id < 0) {
			fputs("fbcon: no display device\n", stderr);
			exit(EXIT_FAILURE);
		}
		display_get_preferred(display_id, &xres, &yres);

	} else if (argc == 2) { // fbcon display_id
		display_id = atoi(argv[1]);
		if (display_get_preferred(display_id, &xres, &yres) < 0) {
			fputs("fbcon: display device not found\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (argc == 3) { // fbcon xres yres
		display_id = display_find();
		if (display_id < 0) {
			fputs("fbcon: no display device\n", stderr);
			exit(EXIT_FAILURE);
		}
		xres = atoi(argv[1]);
		yres = atoi(argv[2]);
	} else if (argc == 4) { // fbcon display_id xres yres
		display_id = atoi(argv[1]);
		xres = atoi(argv[2]);
		yres = atoi(argv[3]);
	} else {
		fputs("Usage:\n", stderr);
		fputs(" fbcon - default display and resolution", stderr);
		fputs(" fbcon display_id - custom display and default resolution", stderr);
		fputs(" fbcon xres yres - default display and custom resolution", stderr);
		fputs(" fbcon display_id xres yres - custom display and resolution", stderr);
		exit(EXIT_FAILURE);
	}
	unsigned int flag;
	fb = display_enable(display_id, xres, yres, &flag);
	if (!fb) {
		fputs("fbcon: enable display failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	need_update = (flag & DISPLAY_KCALL_FLAG_NEED_UPDATE) ? 1 : 0;

	x_chars = xres / 8;
	y_chars = yres / 16;

	for (int i = 0; i < 1024 * 768; i++) {
		fb[i] = BLACK;
	}

	// create the Pseudoterminal
	int pty = pty_create();
	if (pty < 0) {
		fputs("fbcon: pty create failed\n", stderr);
		abort();
	}
	// spawn the shell process
	int sh_pid = fork();
	if (sh_pid == 0) {
		pty_switch(pty);
		const char* args[] = {"sh", 0};
		exec("/bin/sh", args);
		abort();
	}

	for (;;) {
		char buf[1024];
		int n = pty_read_output(pty, buf, 1024);
		for (int i = 0; i < n; i++) {
			if (buf[i] == '\n') {
				fbcon_putchar(cur_x, cur_y, ' '); // cover the cursor
				cur_x = 0;
				cur_y++;
				if (cur_y == y_chars) {
					for (int j = 0; j < y_chars - 1; j++) {
						fastmemcpy32(fb + j * xres * 16, fb + (j + 1) * xres * 16,
									 xres * 16);
					}
					fastmemset32(fb + (y_chars - 1) * xres * 16, 0, xres * 16);
					cur_y--;
				}
			} else {
				fbcon_putchar(cur_x, cur_y, buf[i]);
				cur_x++;
				if (cur_x >= x_chars) {
					cur_x = 0;
					cur_y++;
					if (cur_y == y_chars) {
						for (int j = 0; j < y_chars - 1; j++) {
							fastmemcpy32(fb + j * xres * 16, fb + (j + 1) * xres * 16,
										 xres * 16);
						}
						fastmemset32(fb + (y_chars - 1) * xres * 16, 0, xres * 16);
						cur_y--;
					}
				}
			}
		}

		unsigned int kbd;
		kcall("keyboard", (unsigned int)&kbd);
		if (kbd != 0x0 && kbd != 0x100 && !(kbd & 0x100)) {
			int keycode = kbd & 0xff;
			switch (keycode) {
			case 8: // backspace
				if (inputptr) {
					fbcon_putchar(cur_x, cur_y, ' ');
					cur_x--;
					inputptr--;
				}
				break;
			case 13: // enter
				inputbuf[inputptr] = '\n';
				inputptr++;
				pty_write_input(pty, inputbuf, inputptr);
				inputptr = 0;
				fbcon_putchar(cur_x, cur_y, ' ');
				cur_x = 0;
				cur_y++;
				if (cur_y == y_chars) {
					for (int j = 0; j < y_chars - 1; j++) {
						fastmemcpy32(fb + j * xres * 16, fb + (j + 1) * xres * 16,
									 xres * 16);
					}
					fastmemset32(fb + (y_chars - 1) * xres * 16, 0, xres * 16);
					cur_y--;
				}
				break;
			case 16: // shift
				shift = 1;
				break;
			default:
				if (inputptr < 80) {
					if (shift) {
						fbcon_putchar(cur_x, cur_y, keymap_upper[keycode]);
						cur_x++;
						inputbuf[inputptr] = keymap_upper[keycode];
						inputptr++;
					} else {
						fbcon_putchar(cur_x, cur_y, keymap_lower[keycode]);
						cur_x++;
						inputbuf[inputptr] = keymap_lower[keycode];
						inputptr++;
					}
				}
			}
		} else if (kbd == 0x110) { // release shift key
			shift = 0;
		}
		fbcon_putchar(cur_x, cur_y, '_');
		// update framebuffer
		if (need_update) {
			display_update(display_id);
		}
	}
}
