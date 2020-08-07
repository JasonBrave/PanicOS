/*
 * Widget Toolkit event class
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

#ifndef _LIBGUI_EVENT_HPP
#define _LIBGUI_EVENT_HPP

namespace GUI {

	class Window;

	template <typename... Args>
	struct Event {
		typedef void (GUI::Window::*handler_class_t)(Args...);
		typedef void (*handler_func_t)(Args...);

		handler_class_t hand_class = nullptr;
		handler_func_t hand_func = nullptr;
		GUI::Window* wnd = nullptr;

		template <typename T>
		constexpr void connect(GUI::Window* win, void (T::*handler)(Args...)) {
			wnd = win;
			hand_class = reinterpret_cast<handler_class_t>(handler);
		}

		constexpr void connect(void (*handler)(Args...)) {
			wnd = nullptr;
			hand_func = handler;
		}

		constexpr void disconnect() {
			wnd = nullptr;
			hand_class = nullptr;
			hand_func = nullptr;
		}

		constexpr void raise(Args... args) {
			if (wnd) {
				(wnd->*hand_class)(args...);
			} else {
				hand_func(args...);
			}
		}

		constexpr void operator()(Args... args) {
			raise(args...);
		}
	};

} // namespace GUI

#endif
