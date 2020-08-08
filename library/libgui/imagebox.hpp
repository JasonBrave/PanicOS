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

#ifndef _LIBGUI_IMAGEBOX_HPP
#define _LIBGUI_IMAGEBOX_HPP

#include <libgui/control.hpp>

namespace GUI {

	class ImageBox : public GUI::ControlBase {
	private:
		int width, height;
		void* img = nullptr;

	public:
		ImageBox() = default;
		ImageBox(const char* filename);
		~ImageBox();
		void load_image(const char* filename);

		void on_mouse_down(int button, int x, int y) override;
		void draw(int handle) override;
	};

} // namespace GUI

#endif
