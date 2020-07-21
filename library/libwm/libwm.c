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
	while (!message_receive(&ret_handle) ||
		   ret_handle.msgtype != WM_MESSAGE_RETURN_HANDLE) {
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

int wm_create_window(int width, int height) {
	struct MessageCreateWindow msg;
	msg.msgtype = WM_MESSAGE_CREATE_WINDOW;
	msg.width = width;
	msg.height = height;
	message_send(wm_pid, sizeof(msg), &msg);
	struct MessageReturnHandle ret_handle;
	while (!message_receive(&ret_handle) ||
		   ret_handle.msgtype != WM_MESSAGE_RETURN_HANDLE) {
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
