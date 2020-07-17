/*
 * Window manager
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

#include <panicos.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_XRES 1024
#define DEFAULT_YRES 768
#define CURSOR_WIDTH 5
#define CURSOR_HEIGHT 5

struct DisplayControl {
	int xres;
	int yres;
	void* framebuffer;
};

typedef struct {
	uint8_t b, g, r;
} __attribute__((packed)) COLOUR;

COLOUR* fb;
int xres, yres;
int cur_x = 200, cur_y = 200;

void* wm_modeswitch(int xres, int yres) {
	struct DisplayControl dc = {.xres = xres, .yres = yres};
	if (kcall("display", (unsigned int)&dc) < 0) {
		fputs("wm: no display found\n", stderr);
		exit(EXIT_FAILURE);
	}
	return dc.framebuffer;
}

void wm_fill(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			fb[(y + i) * xres + (x + j)].r = r;
			fb[(y + i) * xres + (x + j)].g = g;
			fb[(y + i) * xres + (x + j)].b = b;
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		xres = DEFAULT_XRES;
		yres = DEFAULT_YRES;
	} else if (argc == 3) {
		xres = atoi(argv[1]);
		yres = atoi(argv[2]);
	} else {
		fputs("Usage: wm [xres yres]\n", stderr);
		exit(EXIT_FAILURE);
	}
	fb = wm_modeswitch(xres, yres);

	for (int x = 0; x < xres; x++) {
		for (int y = 0; y < yres; y++) {
			fb[y * xres + x].r = 40;
			fb[y * xres + x].g = 200;
			fb[y * xres + x].b = 255;
		}
	}

	// main loop
	for (;;) {
		int m;
		kcall("mouse", (unsigned int)&m);
		if (!m) {
			continue;
		}
		// cursor and mouse move
		wm_fill(cur_x, cur_y, CURSOR_WIDTH, CURSOR_HEIGHT, 40, 200, 255);
		cur_x += (char)((m >> 16) & 0xff);
		cur_y -= (char)((m >> 8) & 0xff);
		wm_fill(cur_x, cur_y, CURSOR_WIDTH, CURSOR_HEIGHT, 255, 0, 0);
		// mouse buttons
		int btn = (m >> 24) & 0xff;
		if (btn & 1) {
			wm_fill(0, 0, 50, 50, 255, 0, 0);
		} else {
			wm_fill(0, 0, 50, 50, 0, 255, 0);
		}
		if (btn & 4) {
			wm_fill(50, 0, 50, 50, 255, 0, 0);
		} else {
			wm_fill(50, 0, 50, 50, 0, 255, 0);
		}
		if (btn & 2) {
			wm_fill(100, 0, 50, 50, 255, 0, 0);
		} else {
			wm_fill(100, 0, 50, 50, 0, 255, 0);
		}
		// scroll wheel
		char wheel = m & 0xff;
		if (wheel) {
			static int ws = 400;
			wm_fill(0, ws, 20, 20, 40, 200, 255);
			ws += (wheel * 5);
			wm_fill(0, ws, 20, 20, 255, 0, 0);
		}
	}

	return 0;
}
