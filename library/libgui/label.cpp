/*
 * Widget Toolkit Label
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

#include "label.hpp"
#include <libwm/wm.h>

void GUI::Label::draw(int handle) {
	COLOUR c{0, 0, 0};
	wm_print_text(handle, x, y, c, label_str.c_str());
}

void GUI::Label::on_mouse_down(int button, int x, int y) {}

void GUI::Label::set_text(const char* text) {
	label_str = text;
}

void GUI::Label::set_text(const std::string& text) {
	label_str = text;
}
