/*
 * BMP File loader
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

#include <cstdint>
#include <cstring>
#include <libwm/wm.h>

#include "bmp.hpp"

#define ROUNDUP4(sz) (((sz) + 4 - 1) & ~(4 - 1))

struct BMPHeader {
	std::uint8_t sig[2];
	std::uint32_t size;
	std::int16_t res;
	std::int16_t res2;
	std::uint32_t offset;
} __attribute__((packed));

struct BMPInfoHeader {
	std::uint32_t header_size;
	std::uint32_t width;
	std::uint32_t height;
	std::uint16_t color_plane;
	std::uint16_t bits_per_pixel;
	std::uint32_t compress;
	std::uint32_t image_size;
	std::uint32_t horizontal_res;
	std::uint32_t vertical_res;
	std::uint32_t num_colors;
	std::uint32_t num_imp_colors;
} __attribute__((packed));

struct BMPPixel24 {
	std::uint8_t b, g, r;
} __attribute__((packed));

GUI::BMPLoader::BMPLoader(const char* filename) {
	bmpfile = std::fopen(filename, "rb");
	if (!bmpfile) {
		return;
	}

	BMPHeader bmp_header;
	fread(&bmp_header, sizeof(bmp_header), 1, bmpfile);

	BMPInfoHeader bmp_info_header;
	fread(&bmp_info_header, sizeof(bmp_info_header), 1, bmpfile);
	width = bmp_info_header.width;
	height = bmp_info_header.height;
	bpp = bmp_info_header.bits_per_pixel;
	img_offset = bmp_header.offset;
}

GUI::BMPLoader::~BMPLoader() {
	std::fclose(bmpfile);
}

void GUI::BMPLoader::pix24_to_colour(void* dest, const void* src, int num) {
	COLOUR* d = reinterpret_cast<COLOUR*>(dest);
	const BMPPixel24* s = reinterpret_cast<const BMPPixel24*>(src);
	for (int i = 0; i < num; i++) {
		d[i].r = s[i].r;
		d[i].g = s[i].g;
		d[i].b = s[i].b;
	}
}

void GUI::BMPLoader::load(void* dest) {
	void* buf = ::operator new(height* ROUNDUP4(width * (bpp / 8)));
	fseek(bmpfile, img_offset, SEEK_SET);
	fread(buf, height * ROUNDUP4(width * (bpp / 8)), 1, bmpfile);

	if (bpp == 24) {
		for (int i = 0; i < height; i++) {
			pix24_to_colour(reinterpret_cast<std::uint8_t*>(dest) +
								i * width * sizeof(COLOUR),
							reinterpret_cast<std::uint8_t*>(buf) +
								(height - i - 1) * ROUNDUP4(width * 3),
							width);
		}
	}
	::operator delete(buf);
}

int GUI::BMPLoader::get_bpp(void) const {
	return bpp;
}

int GUI::BMPLoader::get_width(void) const {
	return width;
}

int GUI::BMPLoader::get_height(void) const {
	return height;
}
