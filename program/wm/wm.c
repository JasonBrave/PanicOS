/*
 * Window manager
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

#include <kcall/display.h>
#include <libwm/protocol.h>
#include <panicos.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURSOR_WIDTH 5
#define CURSOR_HEIGHT 5

extern unsigned char font16[256 * 16];

#define DISPLAY_OP_ENABLE 0
#define DISPLAY_OP_UPDATE 1
#define DISPLAY_OP_FIND 2

#define DISPLAY_FLAG_NEED_UPDATE (1 << 0)

struct DisplayControl {
	int op;
	int display_id;
	int xres;
	int yres;
	int flag;
	void* framebuffer;
};

typedef struct {
	uint8_t b, g, r, x;
} __attribute__((packed)) COLOUR;

struct Window {
	int width, height;
	COLOUR* buffer;
	char title[64];
};

struct Sheet {
	int x, y, width, height;
	int owner_pid;
	COLOUR* buffer;
	struct Window* window;
	struct Sheet* next;
};

// common used colour
COLOUR light_blue = {.r = 40, .g = 200, .b = 255};
COLOUR dark_blue = {.r = 0, .g = 0, .b = 255};
COLOUR red = {.r = 255, .g = 0, .b = 0};
COLOUR green = {.r = 0, .g = 255, .b = 0};
COLOUR gray = {.r = 200, .g = 200, .b = 200};
COLOUR white = {.r = 255, .g = 255, .b = 255};

COLOUR* fb; // framebuffer
unsigned int xres, yres;
int cur_x = 200, cur_y = 200;
struct Sheet* sheet_list = NULL; // sheets linked list
int need_update = 0; // need manual call update
int display_id;

static inline void fastmemcpy32(void* dest, const void* src, int cnt) {
	uint32_t* d = dest;
	const uint32_t* s = src;
	for (int i = 0; i < cnt; i++) {
		d[i] = s[i];
	}
}

static inline void fastmemset32(void* dest, uint32_t val, int cnt) {
	uint32_t* d = dest;
	for (int i = 0; i < cnt; i++) {
		d[i] = val;
	}
}

void wm_print_char(char c, COLOUR* buf, int x, int y, int width, COLOUR colour) {
	for (int i = 0; i < 16; i++) {
		COLOUR* p = buf + (y + i) * width + x;
		if (font16[c * 16 + i] & 1) {
			p[7] = colour;
		}
		if (font16[c * 16 + i] & 2) {
			p[6] = colour;
		}
		if (font16[c * 16 + i] & 4) {
			p[5] = colour;
		}
		if (font16[c * 16 + i] & 8) {
			p[4] = colour;
		}
		if (font16[c * 16 + i] & 16) {
			p[3] = colour;
		}
		if (font16[c * 16 + i] & 32) {
			p[2] = colour;
		}
		if (font16[c * 16 + i] & 64) {
			p[1] = colour;
		}
		if (font16[c * 16 + i] & 128) {
			p[0] = colour;
		}
	}
}

void wm_print_string(COLOUR* buf, const char* str, int x, int y, int width, COLOUR colour) {
	for (int i = 0; (str[i] != '\0') && (str[i] != '\n'); i++) {
		wm_print_char(str[i], buf, x + i * 8, y, width, colour);
	}
}

void wm_fill_buffer(COLOUR* buf, int x, int y, int w, int h, int width, COLOUR colour) {
	for (int i = 0; i < h; i++) {
		fastmemset32(buf + (y + i) * width + x, *(uint32_t*)&colour, w);
	}
}

void wm_copy_buffer(COLOUR* dest, int dest_x, int dest_y, int dest_width, const COLOUR* src,
					int src_x, int src_y, int src_width, int width, int height) {
	for (int i = 0; i < height; i++) {
		fastmemcpy32(dest + (dest_y + i) * dest_width + dest_x,
					 src + (src_y + i) * src_width + src_x, width);
	}
}

void wm_render_window(struct Sheet* sht, COLOUR title_colour) {
	wm_fill_buffer(sht->buffer, 0, 0, sht->width, 32, sht->width, title_colour);
	wm_print_string(sht->buffer, sht->window->title, 8, 8, sht->width, white);
	wm_fill_buffer(sht->buffer, sht->width - 28, 4, 24, 24, sht->width, red);
	wm_print_string(sht->buffer, "X", sht->width - 20, 8, sht->width, white);
	wm_copy_buffer(sht->buffer, 0, 32, sht->width, sht->window->buffer, 0, 0, sht->window->width,
				   sht->window->width, sht->window->height);
}

struct Sheet* wm_create_sheet(int x, int y, int width, int height) {
	// allocate a Sheet object
	struct Sheet* sht = malloc(sizeof(struct Sheet));
	sht->next = NULL;
	sht->x = x;
	sht->y = y;
	sht->width = width;
	sht->height = height;
	sht->window = NULL;
	sht->owner_pid = 0;
	sht->buffer = malloc(sizeof(COLOUR) * width * height);
	// add to linked list
	if (sheet_list == NULL) {
		sheet_list = sht;
	} else {
		struct Sheet* l = sheet_list;
		while (l->next) {
			l = l->next;
		}
		if (l->window) {
			wm_render_window(l, gray);
		}
		l->next = sht;
	}
	return sht;
}

struct Sheet* wm_create_window(int width, int height, int x, int y) {
	struct Sheet* sht = wm_create_sheet(x, y, width, height + 32);
	sht->window = malloc(sizeof(struct Window));
	sht->window->width = width;
	sht->window->height = height;
	sht->window->title[0] = '\0'; // empty title
	sht->window->buffer = malloc(sizeof(COLOUR) * width * height);
	wm_fill_buffer(sht->window->buffer, 0, 0, width, height, width, white);
	wm_render_window(sht, dark_blue);
	return sht;
}

void wm_window_set_title(struct Sheet* sht, const char* title) {
	strcpy(sht->window->title, title);
	if (!sht->next) {
		wm_render_window(sht, dark_blue);
	} else {
		wm_render_window(sht, gray);
	}
}

void wm_draw_sheet(struct Sheet* sht) {
	wm_copy_buffer(fb, sht->x, sht->y, xres, sht->buffer, 0, 0, sht->width, sht->width,
				   sht->height);
}

void sheet_remove(struct Sheet* to_remove) {
	free(to_remove->buffer);
	if (to_remove == sheet_list) {
		sheet_list = sheet_list->next;
	} else {
		struct Sheet* sht = sheet_list;
		while (sht->next != to_remove) {
			sht = sht->next;
			if (!sht) {
				return;
			}
		}
		sht->next = sht->next->next;
	}
	// cover old sheet
	wm_fill_buffer(fb, to_remove->x, to_remove->y, to_remove->width, to_remove->height, xres,
				   light_blue);
	free(to_remove);
	struct Sheet* todraw = sheet_list;
	while (todraw) {
		wm_draw_sheet(todraw);
		todraw = todraw->next;
	}
}

void window_close(struct Sheet* to_remove) {
	free(to_remove->window->buffer);
	free(to_remove->window);
	sheet_remove(to_remove);
}

void window_move_top(struct Sheet* to_move) {
	if (to_move == sheet_list) {
		sheet_list = sheet_list->next;
	} else {
		struct Sheet* lst = sheet_list;
		while (lst->next != to_move) {
			lst = lst->next;
			if (!lst) {
				return;
			}
		}
		lst->next = lst->next->next;
	}

	// attach to end of linked lsit
	struct Sheet* sht = sheet_list;
	while (sht && sht->next) {
		sht = sht->next;
	}
	if (sht->window) {
		wm_render_window(sht, gray);
	}
	sht->next = to_move;
	wm_render_window(to_move, dark_blue);
	to_move->next = NULL;
	// draw buffers
	struct Sheet* todraw = sheet_list;
	while (todraw) {
		wm_draw_sheet(todraw);
		todraw = todraw->next;
	}
}

struct Sheet* win_moving = NULL;
unsigned int prev_x = 0, prev_y = 0;

void window_onclick(struct Sheet* sht, unsigned int btn) {
	if (btn & 1 && cur_y < sht->y + 32 && !win_moving) {
		if (cur_x >= sht->x + sht->width - 28 && cur_x < sht->x + sht->width - 4 &&
			cur_y >= sht->y + 4 && cur_y < sht->y + 28) {
			// close button event
			if (sht->owner_pid) {
				struct MessageWindowCloseEvent msg;
				msg.msgtype = WM_MESSAGE_WINDOW_CLOSE_EVENT;
				msg.sheet_id = (int)sht;
				message_send(sht->owner_pid, sizeof(msg), &msg);
			}
		} else {
			// start to move the window
			win_moving = sht;
			prev_x = cur_x - sht->x;
			prev_y = cur_y - sht->y;
			// move window to top
			if (sht->next) {
				window_move_top(sht);
			}
		}
	} else if (win_moving && !(btn & 1)) {
		win_moving = NULL;
	}
}

void mouse_cursor_event(void) {
	struct Sheet* sht = win_moving;
	if (win_moving && cur_y < sht->y + 32) {
		// cover old window
		wm_fill_buffer(fb, sht->x, sht->y, sht->width, sht->height, xres, light_blue);
		sht->x = cur_x - prev_x;
		sht->y = cur_y - prev_y;
		// draw buffers
		struct Sheet* todraw = sheet_list;
		while (todraw) {
			wm_draw_sheet(todraw);
			todraw = todraw->next;
		}
	}
}

void mouse_button_event(unsigned char btn) {
	struct Sheet* sht = sheet_list;
	while (sht) {
		if (cur_x >= sht->x && cur_x < sht->x + sht->width && cur_y >= sht->y &&
			cur_y < sht->y + sht->height) {
			if (sht->window) {
				window_onclick(sht, btn);
			}
			// mouse button event message
			struct MessageMouseButtonEvent msg;
			msg.msgtype = WM_MESSAGE_MOUSE_BUTTON_EVENT;
			msg.sheet_id = (int)sht;
			msg.x = cur_x - sht->x;
			if (sht->window) {
				if (cur_y < sht->y + 32) {
					return;
				}
				msg.y = cur_y - sht->y - 32;
			} else {
				msg.y = cur_y - sht->y;
			}
			msg.button = btn;
			message_send(sht->owner_pid, sizeof(msg), &msg);

			return;
		}
		sht = sht->next;
	}
}

void keyboard_event(unsigned int keycode) {
	struct MessageKeyboardEvent event = {.msgtype = WM_MESSAGE_KEYBOARD_EVENT};
	event.keycode = keycode & 0xff;
	if (keycode & 0x100) {
		event.event = 1;
	} else {
		event.event = 0;
	}
	event.sheet_id = 0;
	message_send(getppid(), sizeof(event), &event);
	struct Sheet *wnd = NULL, *sht = sheet_list;
	while (sht) {
		if (sht->window) {
			wnd = sht;
		}
		sht = sht->next;
	}
	if (wnd) {
		event.sheet_id = (int)wnd;
		message_send(wnd->owner_pid, sizeof(event), &event);
	}
}

void message_received(int pid, void* msg) {
	if (*(int*)msg == WM_MESSAGE_CREATE_SHEET) { // create_sheet
		struct MessageCreateSheet* message = msg;
		struct Sheet* sheet_handle =
			wm_create_sheet(message->x, message->y, message->width, message->height);
		sheet_handle->owner_pid = pid;
		struct MessageReturnHandle return_handle = {.msgtype = WM_MESSAGE_RETURN_HANDLE,
													.handle = (int)sheet_handle};
		message_send(pid, sizeof(return_handle), &return_handle);
	} else if (*(int*)msg == WM_MESSAGE_FILL_SHEET) { // fill_sheet
		struct MessageFillSheet* message = msg;
		COLOUR colour = {.r = message->r, .g = message->g, .b = message->b};
		struct Sheet* sheet = (struct Sheet*)message->sheet_id;
		if (!sheet->window) {
			wm_fill_buffer(sheet->buffer, 0, 0, sheet->width, sheet->height, sheet->width, colour);
		} else {
			wm_fill_buffer(sheet->window->buffer, 0, 0, sheet->window->width, sheet->window->height,
						   sheet->window->width, colour);
			if (!sheet->next) {
				wm_render_window(sheet, dark_blue);
			} else {
				wm_render_window(sheet, gray);
			}
		}
	} else if (*(int*)msg == WM_MESSAGE_PRINT_TEXT) { // print_text
		struct MessagePrintText* message = msg;
		COLOUR colour = {.r = message->r, .g = message->g, .b = message->b};
		struct Sheet* sheet = (struct Sheet*)message->sheet_id;
		if (!sheet->window) {
			wm_print_string(sheet->buffer, message->text, message->x, message->y, sheet->width,
							colour);
		} else {
			wm_print_string(sheet->window->buffer, message->text, message->x, message->y,
							sheet->window->width, colour);
			if (!sheet->next) {
				wm_render_window(sheet, dark_blue);
			} else {
				wm_render_window(sheet, gray);
			}
		}
	} else if (*(int*)msg == WM_MESSAGE_CREATE_WINDOW) { // create_window
		struct MessageCreateWindow* message = msg;
		struct Sheet* sheet_handle = wm_create_window(message->width, message->height, 200, 200);
		sheet_handle->owner_pid = pid;
		struct MessageReturnHandle return_handle = {.msgtype = WM_MESSAGE_RETURN_HANDLE,
													.handle = (int)sheet_handle};
		message_send(pid, sizeof(return_handle), &return_handle);
	} else if (*(int*)msg == WM_MESSAGE_WINDOW_SET_TITLE) { // window_set_title
		struct MessageWindowSetTitle* message = msg;
		wm_window_set_title((struct Sheet*)message->sheet_id, message->title);
	} else if (*(int*)msg == WM_MESSAGE_REMOVE_SHEET) { // window_remove_sheet
		struct MessageWindowSetTitle* message = msg;
		struct Sheet* sheet = (struct Sheet*)message->sheet_id;
		if (sheet->window) {
			window_close(sheet);
		} else {
			sheet_remove(sheet);
		}

	} else if (*(int*)msg == WM_MESSAGE_DRAW_RECT) { // window_draw_rect
		struct MessageDrawRect* message = msg;
		struct Sheet* sheet = (struct Sheet*)message->sheet_id;
		COLOUR colour = {.r = message->r, .g = message->g, .b = message->b};
		if (!sheet->window) {
			wm_fill_buffer(sheet->buffer, message->x, message->y, message->w, message->h,
						   sheet->width, colour);
		} else {
			wm_fill_buffer(sheet->window->buffer, message->x, message->y, message->w, message->h,
						   sheet->window->width, colour);
			if (!sheet->next) {
				wm_render_window(sheet, dark_blue);
			} else {
				wm_render_window(sheet, gray);
			}
		}
	} else if (*(int*)msg == WM_MESSAGE_DRAW_BUFFER) {
		struct MessageDrawBuffer* message = msg;
		struct Sheet* sheet = (struct Sheet*)message->sheet_id;
		if (!sheet->window) {
			wm_copy_buffer(sheet->buffer, message->x, message->y, sheet->width,
						   (void*)message->buffer, 0, 0, message->w, message->w, message->h);
		} else {
			wm_copy_buffer(sheet->window->buffer, message->x, message->y, sheet->window->width,
						   (void*)message->buffer, 0, 0, message->w, message->w, message->h);
			if (!sheet->next) {
				wm_render_window(sheet, dark_blue);
			} else {
				wm_render_window(sheet, gray);
			}
		}
	} else {
		printf("Unknown message %d from pid %d\n", *(int*)msg, pid);
	}
	struct Sheet* todraw = sheet_list;
	while (todraw) {
		wm_draw_sheet(todraw);
		todraw = todraw->next;
	}
}

int main(int argc, char* argv[]) {
	if (argc == 1) { // wm
		display_id = display_find();
		if (display_id < 0) {
			fputs("wm: no display device\n", stderr);
			exit(EXIT_FAILURE);
		}
		display_get_preferred(display_id, &xres, &yres);

	} else if (argc == 2) { // wm display_id
		display_id = atoi(argv[1]);
		if (display_get_preferred(display_id, &xres, &yres) < 0) {
			fputs("wm: display device not found\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (argc == 3) { // wm xres yres
		display_id = display_find();
		if (display_id < 0) {
			fputs("wm: no display device\n", stderr);
			exit(EXIT_FAILURE);
		}
		xres = atoi(argv[1]);
		yres = atoi(argv[2]);
	} else if (argc == 4) { // wm display_id xres yres
		display_id = atoi(argv[1]);
		xres = atoi(argv[2]);
		yres = atoi(argv[3]);
	} else {
		fputs("Usage:\n", stderr);
		fputs(" wm - default display and resolution\n", stderr);
		fputs(" wm display_id - custom display and default resolution\n", stderr);
		fputs(" wm xres yres - default display and custom resolution\n", stderr);
		fputs(" wm display_id xres yres - custom display and resolution\n", stderr);
		exit(EXIT_FAILURE);
	}
	unsigned int flag;
	char display_name[64];
	if (display_get_name(display_id, display_name) != 0) {
		fputs("wm: display get name failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	printf("wm: enable display %d %s %dx%d @ %dbpp\n", display_id, display_name, xres, yres, 32);
	fb = display_enable(display_id, xres, yres, &flag);
	if (!fb) {
		fputs("wm: enable display failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	need_update = (flag & DISPLAY_KCALL_FLAG_NEED_UPDATE) ? 1 : 0;

	wm_fill_buffer(fb, 0, 0, xres, yres, xres, light_blue);

	char* msg = malloc(1024 * 4096); // 4MiB byte buffer
	// main loop
	for (;;) {
		int pid;
		if ((pid = message_receive(msg)) != 0) {
			message_received(pid, msg);
		}
		// keyboard
		unsigned int kbd;
		kcall("keyboard", (unsigned int)&kbd);
		if (kbd != 0x0 && kbd != 0x100) {
			keyboard_event(kbd);
		}
		// mouse
		int m;
		kcall("mouse", (unsigned int)&m);
		if (!m) {
			// update framebuffer
			if (need_update) {
				display_update(display_id);
			}
			continue;
		}
		// remove old cursor
		wm_fill_buffer(fb, cur_x, cur_y, CURSOR_WIDTH, CURSOR_HEIGHT, xres, light_blue);
		// draw sheets
		struct Sheet* sht = sheet_list;
		while (sht) {
			wm_draw_sheet(sht);
			sht = sht->next;
		}
		// mouse cursor
		char movex = (char)((m >> 16) & 0xff);
		char movey = (char)((m >> 8) & 0xff);
		cur_x += movex;
		cur_y -= movey;
		if (movex || movey) {
			mouse_cursor_event();
		}
		// draw cursor
		wm_fill_buffer(fb, cur_x, cur_y, CURSOR_WIDTH, CURSOR_HEIGHT, xres, red);
		// mouse buttons
		int btn = (m >> 24) & 7;
		static int prevbtn = 0;
		if (prevbtn != btn) {
			mouse_button_event(btn);
			prevbtn = btn;
		}
		// update framebuffer
		if (need_update) {
			display_update(display_id);
		}
	}

	return 0;
}
