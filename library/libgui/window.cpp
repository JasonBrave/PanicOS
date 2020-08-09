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

#include "window.hpp"
#include <cstdlib>
#include <libwm/wm.h>

GUI::Window::Window(int width, int height) {
	handle = wm_create_window(width, height);
}

GUI::Window::~Window() {
	wm_remove_sheet(handle);
}

void GUI::Window::render() {
	for (const auto& t : control_cont) {
		t->draw(handle);
	}
}

void GUI::Window::set_title(const char* title) {
	wm_window_set_title(handle, title);
}

void GUI::Window::add_control(ControlBase& control) {
	control_cont.push_back(&control);
}

void GUI::Window::on_mouse_down(int button, int x, int y) {
	for (const auto& t : control_cont) {
		if (t->x <= x && x < t->x + t->width && t->y <= y && y < t->y + t->height) {
			t->on_mouse_down(button, x - t->x, y - t->y);
		}
	}
}

void GUI::Window::on_key_down(int key) {}
void GUI::Window::on_key_up(int key) {}
void GUI::Window::on_mouse_up(int button, int x, int y) {}

namespace {
	GUI::Window* runwin;

	void app_cleanup() {
		runwin->~Window();
	}
} // namespace

void GUI::start_application(GUI::Window& win) {
	runwin = &win;
	std::atexit(app_cleanup);
	std::at_quick_exit(app_cleanup);

	win.render();

	WmEvent ev;
	while (wm_wait_event(&ev) && ev.handle == win.get_handle()) {
		switch (ev.event_type) {
		case WM_EVENT_KEY_DOWN:
			win.on_key_down(ev.keycode);
			break;
		case WM_EVENT_KEY_UP:
			win.on_key_up(ev.keycode);
			break;
		case WM_EVENT_MOUSE_BUTTON_DOWN:
			win.on_mouse_down(ev.keycode, ev.x, ev.y);
			break;
		case WM_EVENT_MOUSE_BUTTON_UP:
			win.on_mouse_up(ev.keycode, ev.x, ev.y);
			break;
		case WM_EVENT_WINDOW_CLOSE:
			std::exit(0);
			break;
		}
		win.render();
	}
}
