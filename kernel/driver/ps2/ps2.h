#ifndef _DRIVER_PS2_H
#define _DRIVER_PS2_H

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

enum PS2Command {
	PS2_CMD_DISABLE_SECOND = 0xa7,
	PS2_CMD_ENABLE_SECOND = 0xa8,
	PS2_CMD_READ_CFG = 0x20,
	PS2_CMD_WRITE_CFG = 0x60,
	PS2_CMD_WRITE_SECOND = 0xd4,
};

enum PS2MouseCommand {
	PS2_MOUSE_CMD_MOUSE_ID = 0xf2,
	PS2_MOUSE_CMD_SET_SAMPLE_RATE = 0xf3,
	PS2_MOUSE_CMD_ENABLE_REPORT = 0xf4,
	PS2_MOUSE_CMD_DISABLE_REPORT = 0xf5,
};

enum PS2MouseType {
	PS2_MOUSE_WITH_WHEEL,
	PS2_MOUSE_WITHOUT_WHEEL,
};

// mouse.c
void ps2_mouse_init(void);
void ps2_mouse_intr(void);

// keyboard.c
void ps2_keyboard_init(void);
void ps2_keyboard_intr(void);

#endif
