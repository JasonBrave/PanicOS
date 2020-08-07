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

#ifndef _LIBGUI_LABEL_HPP
#define _LIBGUI_LABEL_HPP

#include <libgui/control.hpp>
#include <string>

namespace GUI {

	class Label : public GUI::ControlBase {
		std::string label_str;

		void draw(int handle) override;
		void on_mouse_down(int button, int x, int y) override;

	public:
		constexpr Label() = default;
		constexpr Label(const char* text) : label_str(text) {}
		constexpr Label(std::string text) : label_str(text) {}
		void set_text(const char* text);
		void set_text(const std::string& text);
	};

} // namespace GUI

#endif
