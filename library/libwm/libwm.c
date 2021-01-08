/*
 * Window manager control library
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

#include <panicos.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "wm.h"

static int wm_pid;

int wm_init(void) {
	wm_pid = proc_search("wm");
	if (!wm_pid) {
		return 0;
	}
	return 1;
}

int wm_create_sheet(int x, int y, int width, int height) {
	struct MessageCreateSheet msg;
	msg.msgtype = WM_MESSAGE_CREATE_SHEET;
	msg.x = x;
	msg.y = y;
	msg.width = width;
	msg.height = height;
	message_send(wm_pid, sizeof(msg), &msg);
	struct MessageReturnHandle ret_handle;
	while (!message_receive(&ret_handle) || ret_handle.msgtype != WM_MESSAGE_RETURN_HANDLE) {
	}
	return ret_handle.handle;
}

void wm_fill_sheet(int handle, COLOUR colour) {
	struct MessageFillSheet msg;
	msg.msgtype = WM_MESSAGE_FILL_SHEET;
	msg.sheet_id = handle;
	msg.r = colour.r;
	msg.g = colour.g;
	msg.b = colour.b;
	message_send(wm_pid, sizeof(msg), &msg);
}

void wm_print_text(int handle, int x, int y, COLOUR colour, const char* text) {
	struct MessagePrintText msg;
	msg.msgtype = WM_MESSAGE_PRINT_TEXT;
	msg.sheet_id = handle;
	msg.x = x;
	msg.y = y;
	msg.r = colour.r;
	msg.g = colour.g;
	msg.b = colour.b;
	strcpy(msg.text, text);
	message_send(wm_pid, sizeof(msg), &msg);
}

void wm_print_text_n(int handle, int x, int y, COLOUR colour, const char* text, int n) {
	struct MessagePrintText msg;
	msg.msgtype = WM_MESSAGE_PRINT_TEXT;
	msg.sheet_id = handle;
	msg.x = x;
	msg.y = y;
	msg.r = colour.r;
	msg.g = colour.g;
	msg.b = colour.b;
	memcpy(msg.text, text, n);
	msg.text[n] = '\0';
	message_send(wm_pid, sizeof(msg), &msg);
}

int wm_create_window(int width, int height) {
	struct MessageCreateWindow msg;
	msg.msgtype = WM_MESSAGE_CREATE_WINDOW;
	msg.width = width;
	msg.height = height;
	message_send(wm_pid, sizeof(msg), &msg);
	struct MessageReturnHandle ret_handle;
	while (!message_receive(&ret_handle) || ret_handle.msgtype != WM_MESSAGE_RETURN_HANDLE) {
	}
	return ret_handle.handle;
}

void wm_window_set_title(int handle, const char* title) {
	struct MessageWindowSetTitle msg;
	msg.msgtype = WM_MESSAGE_WINDOW_SET_TITLE;
	msg.sheet_id = handle;
	strcpy(msg.title, title);
	message_send(wm_pid, sizeof(msg), &msg);
}

int wm_wait_event(struct WmEvent* event) {
	char buf[256];
	if (!message_wait(buf)) {
		return 0;
	}
	if (*(int*)buf == WM_MESSAGE_KEYBOARD_EVENT) {
		struct MessageKeyboardEvent* msg = (struct MessageKeyboardEvent*)buf;
		if (msg->event) {
			event->event_type = WM_EVENT_KEY_UP;
		} else {
			event->event_type = WM_EVENT_KEY_DOWN;
		}
		event->handle = msg->sheet_id;
		event->keycode = msg->keycode;
		return 1;
	} else if (*(int*)buf == WM_MESSAGE_MOUSE_BUTTON_EVENT) {
		static int prevbtn = 0;
		struct MessageMouseButtonEvent* msg = (struct MessageMouseButtonEvent*)buf;
		if (!(prevbtn & 1) && (msg->button & 1)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_DOWN;
			event->keycode = WM_MOUSE_LEFT;
		} else if ((prevbtn & 1) && !(msg->button & 1)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_UP;
			event->keycode = WM_MOUSE_LEFT;
		} else if (!(prevbtn & 2) && (msg->button & 2)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_DOWN;
			event->keycode = WM_MOUSE_RIGHT;
		} else if ((prevbtn & 2) && !(msg->button & 2)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_UP;
			event->keycode = WM_MOUSE_RIGHT;
		} else if (!(prevbtn & 4) && (msg->button & 4)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_DOWN;
			event->keycode = WM_MOUSE_MIDDLE;
		} else if ((prevbtn & 4) && !(msg->button & 4)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_UP;
			event->keycode = WM_MOUSE_MIDDLE;
		}
		prevbtn = msg->button;
		event->handle = msg->sheet_id;
		event->x = msg->x;
		event->y = msg->y;
		return 1;
	} else if (*(int*)buf == WM_MESSAGE_WINDOW_CLOSE_EVENT) {
		struct MessageWindowCloseEvent* msg = (struct MessageWindowCloseEvent*)buf;
		event->event_type = WM_EVENT_WINDOW_CLOSE;
		event->handle = msg->sheet_id;
		return 1;
	} else {
		return 0;
	}
}

int wm_catch_event(struct WmEvent* event) {
	char buf[256];
	if (!message_receive(buf)) {
		return 0;
	}
	if (*(int*)buf == WM_MESSAGE_KEYBOARD_EVENT) {
		struct MessageKeyboardEvent* msg = (struct MessageKeyboardEvent*)buf;
		if (msg->event) {
			event->event_type = WM_EVENT_KEY_UP;
		} else {
			event->event_type = WM_EVENT_KEY_DOWN;
		}
		event->handle = msg->sheet_id;
		event->keycode = msg->keycode;
		return 1;
	} else if (*(int*)buf == WM_MESSAGE_MOUSE_BUTTON_EVENT) {
		static int prevbtn = 0;
		struct MessageMouseButtonEvent* msg = (struct MessageMouseButtonEvent*)buf;
		if (!(prevbtn & 1) && (msg->button & 1)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_DOWN;
			event->keycode = WM_MOUSE_LEFT;
		} else if ((prevbtn & 1) && !(msg->button & 1)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_UP;
			event->keycode = WM_MOUSE_LEFT;
		} else if (!(prevbtn & 2) && (msg->button & 2)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_DOWN;
			event->keycode = WM_MOUSE_RIGHT;
		} else if ((prevbtn & 2) && !(msg->button & 2)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_UP;
			event->keycode = WM_MOUSE_RIGHT;
		} else if (!(prevbtn & 4) && (msg->button & 4)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_DOWN;
			event->keycode = WM_MOUSE_MIDDLE;
		} else if ((prevbtn & 4) && !(msg->button & 4)) {
			event->event_type = WM_EVENT_MOUSE_BUTTON_UP;
			event->keycode = WM_MOUSE_MIDDLE;
		}
		prevbtn = msg->button;
		event->handle = msg->sheet_id;
		event->x = msg->x;
		event->y = msg->y;
		return 1;
	} else if (*(int*)buf == WM_MESSAGE_WINDOW_CLOSE_EVENT) {
		struct MessageWindowCloseEvent* msg = (struct MessageWindowCloseEvent*)buf;
		event->event_type = WM_EVENT_WINDOW_CLOSE;
		event->handle = msg->sheet_id;
		return 1;
	} else {
		return 0;
	}
}

void wm_remove_sheet(int handle) {
	struct MessageRemoveSheet msg;
	msg.msgtype = WM_MESSAGE_REMOVE_SHEET;
	msg.sheet_id = handle;
	message_send(wm_pid, sizeof(msg), &msg);
}

void wm_fill_rect(int handle, int x, int y, int width, int height, COLOUR colour) {
	struct MessageDrawRect msg;
	msg.msgtype = WM_MESSAGE_DRAW_RECT;
	msg.sheet_id = handle;
	msg.x = x;
	msg.y = y;
	msg.w = width;
	msg.h = height;
	msg.r = colour.r;
	msg.g = colour.g;
	msg.b = colour.b;
	message_send(wm_pid, sizeof(msg), &msg);
}

void wm_draw_buffer(int handle, int x, int y, int width, int height, COLOUR* buffer) {
	struct MessageDrawBuffer* msg = malloc(sizeof(struct MessageDrawBuffer));
	msg->msgtype = WM_MESSAGE_DRAW_BUFFER;
	msg->sheet_id = handle;
	msg->x = x;
	msg->y = y;
	msg->w = width;
	msg->h = height;
	memcpy(msg->buffer, buffer, width * height * sizeof(COLOUR));
	message_send(wm_pid, sizeof(struct MessageDrawBuffer), msg);
	free(msg);
}
