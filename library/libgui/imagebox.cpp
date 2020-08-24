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
#include "imageloader/bmp.hpp"

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

	BMPLoader bmp(filename);
	width = bmp.get_width();
	height = bmp.get_height();
	img = ::operator new(width* height * sizeof(COLOUR));
	bmp.load(img);
}

void GUI::ImageBox::on_mouse_down(int button, int x, int y) {}

void GUI::ImageBox::draw(int handle) {
	if (img) {
		wm_draw_buffer(handle, x, y, width, height, reinterpret_cast<COLOUR*>(img));
	}
}
