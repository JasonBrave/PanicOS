#ifndef _LIBWM_WM_H
#define _LIBWM_WM_H

#include <stdint.h>

typedef struct {
	uint8_t b, g, r;
} __attribute__((packed)) COLOUR;

enum WmEventType {
	WM_EVENT_KEY_DOWN,
	WM_EVENT_KEY_UP,
	WM_EVENT_MOUSE_BUTTON_DOWN,
	WM_EVENT_MOUSE_BUTTON_UP,
};

enum WmMouseButton {
	WM_MOUSE_LEFT,
	WM_MOUSE_RIGHT,
	WM_MOUSE_MIDDLE,
};

struct WmEvent {
	enum WmEventType event_type;
	int handle;
	int keycode;
	int x, y;
};

int wm_init(void);
int wm_create_sheet(int x, int y, int width, int height);
void wm_fill_sheet(int handle, COLOUR colour);
void wm_print_text(int handle, int x, int y, COLOUR colour, const char* text);
int wm_create_window(int width, int height);
void wm_window_set_title(int handle, const char* title);
int wm_wait_event(struct WmEvent* event);
int wm_catch_event(struct WmEvent* event);
void wm_remove_sheet(int handle);
void wm_fill_rect(int handle, int x, int y, int width, int height, COLOUR colour);
void wm_draw_buffer(int handle, int x, int y, int width, int height, COLOUR* buffer);

#endif
