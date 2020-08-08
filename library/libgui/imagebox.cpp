/*
 * Widget Toolkit Image Box
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
#include <cstdio>
#include <cstring>
#include <libwm/wm.h>

#include "imagebox.hpp"

#define ROUNDUP4(sz) (((sz) + 4 - 1) & ~(4 - 1))

struct BmpHeader {
	std::uint8_t sig[2];
	std::uint32_t size;
	std::int16_t res;
	std::int16_t res2;
	std::uint32_t offset;
} __attribute__((packed));

struct BmpInfoHeader {
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

GUI::ImageBox::ImageBox(const char* filename) {
	load_image(filename);
}

GUI::ImageBox::~ImageBox() {
	::operator delete(img);
}

void GUI::ImageBox::load_image(const char* filename) {
	if (img) {
		::operator delete(img);
	}

	std::FILE* bmp = std::fopen(filename, "rb");
	if (!bmp) {
		return;
	}

	BmpHeader bmp_header;
	fread(&bmp_header, sizeof(bmp_header), 1, bmp);

	BmpInfoHeader bmp_info_header;
	fread(&bmp_info_header, sizeof(bmp_info_header), 1, bmp);
	width = bmp_info_header.width;
	height = bmp_info_header.height;

	void* buf =
		::operator new(height* ROUNDUP4(width * (bmp_info_header.bits_per_pixel / 8)));
	fseek(bmp, bmp_header.offset, SEEK_SET);
	fread(buf, height * ROUNDUP4(width * (bmp_info_header.bits_per_pixel / 8)), 1, bmp);

	img = ::operator new(width* height * sizeof(COLOUR));
	for (int i = 0; i < height; i++) {
		std::memcpy(reinterpret_cast<std::uint8_t*>(img) + i * width * 3,
					reinterpret_cast<std::uint8_t*>(buf) +
						(height - i - 1) * ROUNDUP4(width * sizeof(COLOUR)),
					width * 3);
	}
	::operator delete(buf);

	std::fclose(bmp);
}

void GUI::ImageBox::on_mouse_down(int button, int x, int y) {}

void GUI::ImageBox::draw(int handle) {
	if (img) {
		wm_draw_buffer(handle, x, y, width, height, reinterpret_cast<COLOUR*>(img));
	}
}
