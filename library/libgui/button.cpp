/*
 * Widget Toolkit Button
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

#include "button.hpp"
#include <libwm/wm.h>

void GUI::Button::on_mouse_down(int button, int x, int y) {
	onclick();
}

void GUI::Button::draw(int handle) {
	wm_fill_rect(handle, x, y, width, height, {159, 150, 150});
	wm_print_text(handle, x + width / 2 - text.length() * 4, y + height / 2 - 8,
				  {0, 0, 0}, text.c_str());
}

GUI::Button::Button(const std::string& s) : text(s) {}
GUI::Button::Button(const char* s) : text(s) {}

void GUI::Button::set_text(const char* s) {
	text = s;
}

void GUI::Button::set_test(const std::string& s) {
	text = s;
}
