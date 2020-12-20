/*
 * Console input and output
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

// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include <common/sleeplock.h>
#include <common/spinlock.h>
#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <core/traps.h>
#include <defs.h>
#include <driver/uart.h>
#include <memlayout.h>
#include <param.h>

#include "font.h"

// comment of the following lines for release build
#define USE_DEBUGCON
#define DEBUGCON_ADDR 0xe9

static void consputc(int);
static void fb_putc(int c);

static int panicked = 0;

static struct {
	struct spinlock lock;
	int locking;
	uint32_t* fb;
	uint32_t width, height;
	enum { CONS_VGA, CONS_FB } mode;
} cons;

static void printint(int xx, int base, int sign) {
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	unsigned int x;

	if (sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);

	if (sign)
		buf[i++] = '-';

	while (--i >= 0)
		consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(const char* fmt, ...) {
	int i, c, locking;
	unsigned int* argp;
	char* s;

	locking = cons.locking;
	if (locking)
		acquire(&cons.lock);

	if (fmt == 0)
		panic("null fmt");

	argp = (unsigned int*)(void*)(&fmt + 1);
	for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
		if (c != '%') {
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if (c == 0)
			break;
		switch (c) {
		case 'd':
			printint(*argp++, 10, 1);
			break;
		case 'x':
		case 'p':
			printint(*argp++, 16, 0);
			break;
		case 's':
			if ((s = (char*)*argp++) == 0)
				s = "(null)";
			for (; *s; s++)
				consputc(*s);
			break;
		case '%':
			consputc('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			consputc('%');
			consputc(c);
			break;
		}
	}

	if (locking)
		release(&cons.lock);
}

void panic(const char* s) {
	int i;
	unsigned int pcs[10];

	cli();
	cons.locking = 0;
	// use lapiccpunum so that we can call panic from mycpu()
	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(&s, pcs);
	for (i = 0; i < 10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1; // freeze other CPU
	for (;;)
		;
}

// PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static unsigned short* crt = (unsigned short*)P2V(0xb8000); // CGA memory

static void cgaputc(int c) {
	static int pos = 0;

	if (c == '\n')
		pos += 80 - pos % 80;
	else if (c == BACKSPACE) {
		if (pos > 0)
			--pos;
	} else
		crt[pos++] = (c & 0xff) | 0x0700; // black on white

	if (pos < 0 || pos > 25 * 80)
		panic("pos under/overflow");

	if ((pos / 80) >= 24) { // Scroll up.
		memmove(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
		pos -= 80;
		memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT + 1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT + 1, pos);
	crt[pos] = ' ' | 0x0700;
}

void consputc(int c) {
	if (panicked) {
		cli();
		for (;;)
			;
	}

	if (c == BACKSPACE) {
		uart_putc('\b');
		uart_putc(' ');
		uart_putc('\b');
	} else {
		uart_putc(c);
	}
	if (cons.mode == CONS_VGA) {
		cgaputc(c);
	} else if (cons.mode == CONS_FB) {
		fb_putc(c);
	}
#ifdef USE_DEBUGCON
	outb(DEBUGCON_ADDR, c);
#endif
}

#define INPUT_BUF 128
struct {
	char buf[INPUT_BUF];
	unsigned int r; // Read index
	unsigned int w; // Write index
	unsigned int e; // Edit index
} input;

#define C(x) ((x) - '@') // Control-x

void consoleintr(int (*getc)(void)) {
	int c;

	acquire(&cons.lock);
	while ((c = getc()) >= 0) {
		switch (c) {
		case C('U'): // Kill line.
			while (input.e != input.w && input.buf[(input.e - 1) % INPUT_BUF] != '\n') {
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		case C('H'):
		case '\x7f': // Backspace
			if (input.e != input.w) {
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		default:
			if (c != 0 && input.e - input.r < INPUT_BUF) {
				c = (c == '\r') ? '\n' : c;
				input.buf[input.e++ % INPUT_BUF] = c;
				consputc(c);
				if (c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF) {
					input.w = input.e;
					wakeup(&input.r);
				}
			}
			break;
		}
	}
	release(&cons.lock);
}

int consoleread(char* dst, int n) {
	unsigned int target;
	int c;

	target = n;
	acquire(&cons.lock);
	while (n > 0) {
		while (input.r == input.w) {
			if (myproc()->killed) {
				release(&cons.lock);
				return -1;
			}
			sleep(&input.r, &cons.lock);
		}
		c = input.buf[input.r++ % INPUT_BUF];
		if (c == C('D')) { // EOF
			if (n < target) {
				// Save ^D for next time, to make sure
				// caller gets a 0-byte result.
				input.r--;
			}
			break;
		}
		*dst++ = c;
		--n;
		if (c == '\n')
			break;
	}
	release(&cons.lock);

	return target - n;
}

int consolewrite(char* buf, int n) {
	acquire(&cons.lock);
	for (int i = 0; i < n; i++)
		consputc(buf[i] & 0xff);
	release(&cons.lock);

	return n;
}

void vgacon_init(void) {
	initlock(&cons.lock, "console");
	cons.locking = 1;
	cons.mode = CONS_VGA;
	memset(P2V(0xb8000), 0, 0x8000);
}

void fbcon_init(phyaddr_t fb_addr, unsigned int width, unsigned int height) {
	initlock(&cons.lock, "console");
	cons.locking = 1;
	cons.fb = map_ram_region(fb_addr, width * height * 4);
	if (!cons.fb) {
		panic("fbcon mmap failed");
	}
	cons.width = width;
	cons.height = height;
	cons.mode = CONS_FB;
	memset(cons.fb, 0, width * height * 4);
}

#define BLACK 0
#define WHITE 0xffffff

static void fbcon_drawchar(int x, int y, char c) {
	for (int i = 0; i < 16; i++) {
		uint32_t* p = cons.fb + (y + i) * cons.width + x;
		if (font16[c * 16 + i] & 1) {
			p[7] = WHITE;
		} else {
			p[7] = BLACK;
		}
		if (font16[c * 16 + i] & 2) {
			p[6] = WHITE;
		} else {
			p[6] = BLACK;
		}
		if (font16[c * 16 + i] & 4) {
			p[5] = WHITE;
		} else {
			p[5] = BLACK;
		}
		if (font16[c * 16 + i] & 8) {
			p[4] = WHITE;
		} else {
			p[4] = BLACK;
		}
		if (font16[c * 16 + i] & 16) {
			p[3] = WHITE;
		} else {
			p[3] = BLACK;
		}
		if (font16[c * 16 + i] & 32) {
			p[2] = WHITE;
		} else {
			p[2] = BLACK;
		}
		if (font16[c * 16 + i] & 64) {
			p[1] = WHITE;
		} else {
			p[1] = BLACK;
		}
		if (font16[c * 16 + i] & 128) {
			p[0] = WHITE;
		} else {
			p[0] = BLACK;
		}
	}
}

static inline void fastmemcpy32(void* dest, const void* src, int cnt) {
	uint32_t* d = dest;
	const uint32_t* s = src;
	for (int i = 0; i < cnt; i++) {
		d[i] = s[i];
	}
}

static void fb_putc(int c) {
	static unsigned int xpos = 0, ypos = 0;
	if (c == '\n') {
		fbcon_drawchar(xpos * 8, ypos * 16, ' ');
		xpos = 0;
		ypos++;
		if (ypos >= cons.height / 16) {
			for (unsigned int i = 0; i < cons.height / 16; i++) {
				fastmemcpy32(cons.fb + i * cons.width * 16,
							 cons.fb + (i + 1) * cons.width * 16, cons.width * 16);
			}
			ypos--;
		}
	} else if (c == BACKSPACE) {
		fbcon_drawchar(xpos * 8, ypos * 16, ' ');
		if (xpos == 0) {
			ypos--;
			xpos = cons.width / 8 - 1;
		} else {
			xpos--;
		}
	} else {
		fbcon_drawchar(xpos * 8, ypos * 16, c);
		xpos++;
	}
	if (xpos * 8 + 8 > cons.width) {
		xpos = 0;
		ypos++;
		if (ypos >= cons.height / 16) {
			for (unsigned int i = 0; i < cons.height / 16; i++) {
				fastmemcpy32(cons.fb + i * cons.width * 16,
							 cons.fb + (i + 1) * cons.width * 16, cons.width * 16);
			}
			ypos--;
		}
	}
	fbcon_drawchar(xpos * 8, ypos * 16, '_');
}
