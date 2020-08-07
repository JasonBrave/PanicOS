/*
 * Widget Toolkit controls base class
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

#ifndef _LIBGUI_CONTROL_HPP
#define _LIBGUI_CONTROL_HPP

namespace GUI {

	class ControlBase {
	public:
		int x, y;
		int width, height;

		virtual void on_mouse_down(int button, int x, int y) = 0;
		virtual void draw(int handle) = 0;
	};

} // namespace GUI

#endif
