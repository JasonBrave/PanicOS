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

#ifndef __riscv
#include <arch/x86/lapic.h>
#include <common/x86.h>
#include <driver/x86/uart.h>
#endif

#include <common/sleeplock.h>
#include <common/spinlock.h>
#include <core/proc.h>
#include <core/traps.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

#include <stdarg.h>

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
	uint32_t *fb;
	uint32_t width, height;
	enum {
		CONS_VGA,
		CONS_FB
	} mode;
} cons;

static void printint(unsigned long long xx, int base, int sgn, int cpt) {
	const char *digits = cpt ? "0123456789ABCDEF" : "0123456789abcdef";
	char buf[64];
	int i, neg;
	unsigned long long x;

	neg = 0;
	if (sgn && xx < 0) {
		neg = 1;
		x = -xx;
	} else {
		x = xx;
	}

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);
	if (neg)
		buf[i++] = '-';

	while (--i >= 0)
		consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(const char *restrict fmt, ...) {
	int c, i, state;

	state = 0;
	va_list args;
	va_start(args, fmt);

	if (cons.locking)
		acquire(&cons.lock);

	for (i = 0; fmt[i]; i++) {
		c = fmt[i] & 0xff;
		if (state == 0) {
			if (c == '%') {
				state = '%';
			} else {
				consputc(c);
			}
		} else if (state == '%') {
			state = 0;
			if (c == '%') {
				consputc(c);
			} else if (c == 'c') {
				consputc(va_arg(args, int));
			} else if (c == 's') {
				const char *s = va_arg(args, const char *);
				if (s == 0)
					s = "(null)";
				while (*s != 0) {
					consputc(*s);
					s++;
				}
			} else if (c == 'd' || c == 'i') {
				printint(va_arg(args, int), 10, 1, 0);
			} else if (c == 'x') {
				printint(va_arg(args, unsigned int), 16, 0, 0);
			} else if (c == 'X') {
				printint(va_arg(args, unsigned int), 16, 0, 1);
			} else if (c == 'u') {
				printint(va_arg(args, unsigned int), 10, 0, 0);
			} else if (c == 'p') {
				printint((unsigned int)va_arg(args, void *), 16, 0, 0);
			} else if (c == 'l') {
				state = 'l';
			} else {
				// Unknown % sequence.  Print it to draw attention.
				consputc('%');
				consputc(c);
			}
		} else if (state == 'l') {
			state = 0;
			if (c == 'd' || c == 'i') {
				printint(va_arg(args, long), 10, 1, 0);
			} else if (c == 'x') {
				printint(va_arg(args, unsigned long), 16, 0, 0);
			} else if (c == 'X') {
				printint(va_arg(args, unsigned long), 16, 0, 1);
			} else if (c == 'u') {
				printint(va_arg(args, unsigned long), 10, 0, 0);
			} else if (c == 'l') {
				state = 'L';
			}
		} else if (state == 'L') {
			state = 0;
			if (c == 'd' || c == 'i') {
				printint(va_arg(args, long long), 10, 1, 0);
			} else if (c == 'x') {
				printint(va_arg(args, unsigned long long), 16, 0, 0);
			} else if (c == 'X') {
				printint(va_arg(args, unsigned long long), 16, 0, 1);
			} else if (c == 'u') {
				printint(va_arg(args, unsigned long long), 10, 0, 0);
			}
		}
	}
	if (cons.locking)
		release(&cons.lock);
}

void panic(const char *s) {
#ifndef __riscv
	int i;
	unsigned int pcs[10];
	cli();
#endif
	cons.locking = 0;
// use lapiccpunum so that we can call panic from mycpu()
#ifndef __riscv
	cprintf("lapicid %d: panic: ", lapicid());
#else
	cprintf("panic: ");
#endif
	cprintf(s);
	cprintf("\n");
#ifndef __riscv
	getcallerpcs(&s, pcs);
	for (i = 0; i < 10; i++)
		cprintf(" %p", pcs[i]);

#endif

	panicked = 1; // freeze other CPU
	for (;;)
		;
}

#define BACKSPACE 0x100

#ifndef __riscv

// PAGEBREAK: 50
#define CRTPORT 0x3d4
static unsigned short *crt = (unsigned short *)P2V(0xb8000); // CGA memory

static void cgaputc(int c) {
	static int pos = 0;

	if (c == '\n') {
		pos += 80 - pos % 80;
	} else if (c == BACKSPACE) {
		if (pos > 0) {
			--pos;
		}
	} else {
		crt[pos++] = (c & 0xff) | 0x0700; // black on white
	}

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

#endif

void consputc(int c) {
	if (panicked) {
#ifndef __riscv
		cli();
#endif
		for (;;)
			;
	}

#ifndef __riscv
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
#endif

#ifdef USE_DEBUGCON
#ifdef __riscv
	volatile uint8_t *uart_tx = (uint8_t *)0x10000000;
	*uart_tx = c;
#else
	outb(DEBUGCON_ADDR, c);
#endif
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
#ifndef __riscv
					wakeup(&input.r);
#endif
				}
			}
			break;
		}
	}
	release(&cons.lock);
}

int consoleread(char *dst, int n) {
	unsigned int target;
	int c;

	target = n;
	acquire(&cons.lock);
	while (n > 0) {
		while (input.r == input.w) {
#ifndef __riscv
			if (myproc()->killed) {
				release(&cons.lock);
				return -1;
			}
			sleep(&input.r, &cons.lock);
#endif
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

int consolewrite(char *buf, int n) {
	acquire(&cons.lock);
	for (int i = 0; i < n; i++)
		consputc(buf[i] & 0xff);
	release(&cons.lock);

	return n;
}

#ifndef __riscv

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
		uint32_t *p = cons.fb + (y + i) * cons.width + x;
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

static inline void fastmemcpy32(void *dest, const void *src, int cnt) {
	uint32_t *d = dest;
	const uint32_t *s = src;
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
				fastmemcpy32(cons.fb + i * cons.width * 16, cons.fb + (i + 1) * cons.width * 16,
							 cons.width * 16);
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
				fastmemcpy32(cons.fb + i * cons.width * 16, cons.fb + (i + 1) * cons.width * 16,
							 cons.width * 16);
			}
			ypos--;
		}
	}
	fbcon_drawchar(xpos * 8, ypos * 16, '_');
}

#endif
