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

static int atoi(const char* nptr) {
	int n = 0, neg = 0;
	while (*nptr) {
		if ((*nptr >= '0') && (*nptr <= '9')) {
			n = n * 10 + (*nptr - '0');
		} else if (*nptr == '-') {
			neg = 1;
		} else if (*nptr == '+') {
			neg = 0;
		}
		nptr++;
	}
	return neg ? -n : n;
}

struct DisplayControl {
	int xres;
	int yres;
	void* framebuffer;
};

void* wm_modeswitch(int xres, int yres) {
	struct DisplayControl dc = {.xres = xres, .yres = yres};
	if (kcall("display", (unsigned int)&dc) < 0) {
		fputs("wm: no display found\n", stderr);
		exit(EXIT_FAILURE);
	}
	return dc.framebuffer;
}

typedef struct {
	uint8_t b, g, r;
} __attribute__((packed)) COLOUR;

int xres, yres;

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
	COLOUR* fb = wm_modeswitch(xres, yres);

	for (int x = 0; x < xres; x++) {
		for (int y = 0; y < yres; y++) {
			fb[y * xres + x].r = 40;
			fb[y * xres + x].g = 200;
			fb[y * xres + x].b = 255;
		}
	}

	for (;;) {
	}
	return 0;
}
