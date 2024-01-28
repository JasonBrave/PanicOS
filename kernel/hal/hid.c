/*
 * Human interface device hardware abstraction layer
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

#include <defs.h>
#include <driver/pci/pci.h>
#include <driver/usb/usb.h>
#include <driver/virtio/virtio.h>
#include <proc/kcall.h>

#define MOUSE_QUEUE_SIZE 16
static unsigned int mouse_queue[MOUSE_QUEUE_SIZE];
static int mouse_queue_begin = 0, mouse_queue_end = 0;
static struct spinlock mouse_queue_lock;

static int mouse_kcall_handler(unsigned int p) {
	unsigned int *m = (void *)p;
	acquire(&mouse_queue_lock);
	if (mouse_queue_begin == mouse_queue_end) {
		*m = 0;
		release(&mouse_queue_lock);
		return 0;
	}
	*m = mouse_queue[mouse_queue_end];
	mouse_queue_end++;
	if (mouse_queue_end == MOUSE_QUEUE_SIZE) {
		mouse_queue_end = 0;
	}
	release(&mouse_queue_lock);
	return 0;
}

void hal_mouse_update(unsigned int data) {
	acquire(&mouse_queue_lock);
	mouse_queue[mouse_queue_begin] = data;
	mouse_queue_begin++;
	if (mouse_queue_begin == MOUSE_QUEUE_SIZE) {
		mouse_queue_begin = 0;
	}
	release(&mouse_queue_lock);
}

#define KEYBOARD_QUEUE_SIZE 16
static unsigned int keyboard_queue[KEYBOARD_QUEUE_SIZE];
static int keyboard_queue_begin = 0, keyboard_queue_end = 0;
static struct spinlock keyboard_queue_lock;
int hal_kbd_send_legacy = 1;

static int keyboard_kcall_handler(unsigned int p) {
	hal_kbd_send_legacy = 0;
	unsigned int *m = (void *)p;
	acquire(&keyboard_queue_lock);
	if (keyboard_queue_begin == keyboard_queue_end) {
		*m = 0;
		release(&keyboard_queue_lock);
		return 0;
	}
	*m = keyboard_queue[keyboard_queue_end];
	keyboard_queue_end++;
	if (keyboard_queue_end == KEYBOARD_QUEUE_SIZE) {
		keyboard_queue_end = 0;
	}
	release(&keyboard_queue_lock);
	return 0;
}

void hal_keyboard_update(unsigned int data) {
	if (data == 144) { // numlock
#ifndef __riscv
		procdump();
#endif
		print_memory_usage();
		pci_print_devices();
		usb_print_devices();
		virtio_print_devices();
#ifndef __riscv
		module_print();
#endif
		return;
	}
	acquire(&keyboard_queue_lock);
	keyboard_queue[keyboard_queue_begin] = data;
	keyboard_queue_begin++;
	if (keyboard_queue_begin == KEYBOARD_QUEUE_SIZE) {
		keyboard_queue_begin = 0;
	}
	release(&keyboard_queue_lock);
}

void hal_hid_init(void) {
	memset(mouse_queue, 0, sizeof(mouse_queue));
	kcall_set("mouse", mouse_kcall_handler);
	initlock(&mouse_queue_lock, "mouse");

	memset(keyboard_queue, 0, sizeof(keyboard_queue));
	kcall_set("keyboard", keyboard_kcall_handler);
	initlock(&keyboard_queue_lock, "keyboard");
}
