#ifndef _LIBWM_WM_H
#define _LIBWM_WM_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

typedef struct {
#ifdef __cplusplus
	std::uint8_t b, g, r, x;
#else
	uint8_t b, g, r, x;
#endif
} __attribute__((packed)) COLOUR;

enum WmEventType {
	WM_EVENT_KEY_DOWN,
	WM_EVENT_KEY_UP,
	WM_EVENT_MOUSE_BUTTON_DOWN,
	WM_EVENT_MOUSE_BUTTON_UP,
	WM_EVENT_WINDOW_CLOSE,
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
void wm_print_text_n(int handle, int x, int y, COLOUR colour, const char* text, int n);
int wm_create_window(int width, int height);
void wm_window_set_title(int handle, const char* title);
int wm_wait_event(struct WmEvent* event);
int wm_catch_event(struct WmEvent* event);
void wm_remove_sheet(int handle);
void wm_fill_rect(int handle, int x, int y, int width, int height, COLOUR colour);
void wm_draw_buffer(int handle, int x, int y, int width, int height, COLOUR* buffer);

#ifdef __cplusplus
}
#endif

#endif
