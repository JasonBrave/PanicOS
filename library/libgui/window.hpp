/*
 * Widget Toolkit Window
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

#include <libgui/control.hpp>
#include <libgui/wmbase.hpp>
#include <vector>

namespace GUI {

	class Window : private WmBase {
		int handle;
		std::vector<ControlBase*> control_cont;

	public:
		Window() = delete;
		Window(int width, int height);
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		virtual ~Window();

		void render();
		void set_title(const char* title);
		void add_control(ControlBase& control);

		constexpr int get_handle() {
			return handle;
		}

		// events
		virtual void on_key_down(int key);
		virtual void on_key_up(int key);
		virtual void on_mouse_down(int button, int x, int y);
		virtual void on_mouse_up(int button, int x, int y);
	};

	void start_application(Window& win);

} // namespace GUI
