/*
 * x86 instruction header
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _X86_H
#define _X86_H

static inline unsigned char inb(unsigned short port) {
	unsigned char data;

	__asm__ volatile("in %1,%0" : "=a"(data) : "d"(port));
	return data;
}

static inline void insl(int port, void* addr, int cnt) {
	__asm__ volatile("cld; rep insl"
					 : "=D"(addr), "=c"(cnt)
					 : "d"(port), "0"(addr), "1"(cnt)
					 : "memory", "cc");
}

static inline void outb(unsigned short port, unsigned char data) {
	__asm__ volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outw(unsigned short port, unsigned short data) {
	__asm__ volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outsl(int port, const void* addr, int cnt) {
	__asm__ volatile("cld; rep outsl"
					 : "=S"(addr), "=c"(cnt)
					 : "d"(port), "0"(addr), "1"(cnt)
					 : "cc");
}

static inline void stosb(void* addr, int data, int cnt) {
	__asm__ volatile("cld; rep stosb"
					 : "=D"(addr), "=c"(cnt)
					 : "0"(addr), "1"(cnt), "a"(data)
					 : "memory", "cc");
}

static inline void stosl(void* addr, int data, int cnt) {
	__asm__ volatile("cld; rep stosl"
					 : "=D"(addr), "=c"(cnt)
					 : "0"(addr), "1"(cnt), "a"(data)
					 : "memory", "cc");
}

struct segdesc;

static inline void lgdt(struct segdesc* p, int size) {
	volatile unsigned short pd[3];

	pd[0] = size - 1;
	pd[1] = (unsigned int)p;
	pd[2] = (unsigned int)p >> 16;

	__asm__ volatile("lgdt (%0)" : : "r"(pd));
}

struct gatedesc;

static inline void lidt(struct gatedesc* p, int size) {
	volatile unsigned short pd[3];

	pd[0] = size - 1;
	pd[1] = (unsigned int)p;
	pd[2] = (unsigned int)p >> 16;

	__asm__ volatile("lidt (%0)" : : "r"(pd));
}

static inline void ltr(unsigned short sel) {
	__asm__ volatile("ltr %0" : : "r"(sel));
}

static inline unsigned int readeflags(void) {
	unsigned int eflags;
	__asm__ volatile("pushfl; popl %0" : "=r"(eflags));
	return eflags;
}

static inline void loadgs(unsigned short v) {
	__asm__ volatile("movw %0, %%gs" : : "r"(v));
}

static inline void cli(void) {
	__asm__ volatile("cli");
}

static inline void sti(void) {
	__asm__ volatile("sti");
}

static inline unsigned int xchg(volatile unsigned int* addr, unsigned int newval) {
	unsigned int result;

	// The + in "+m" denotes a read-modify-write operand.
	__asm__ volatile("lock; xchgl %0, %1"
					 : "+m"(*addr), "=a"(result)
					 : "1"(newval)
					 : "cc");
	return result;
}

static inline unsigned int rcr2(void) {
	unsigned int val;
	__asm__ volatile("movl %%cr2,%0" : "=r"(val));
	return val;
}

static inline void lcr3(unsigned int val) {
	__asm__ volatile("movl %0,%%cr3" : : "r"(val));
}

// PAGEBREAK: 36
// Layout of the trap frame built on the stack by the
// hardware and by trap__asm__.S, and passed to trap().
struct trapframe {
	// registers as pushed by pusha
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int oesp; // useless & ignored
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;

	// rest of trap frame
	unsigned short gs;
	unsigned short padding1;
	unsigned short fs;
	unsigned short padding2;
	unsigned short es;
	unsigned short padding3;
	unsigned short ds;
	unsigned short padding4;
	unsigned int trapno;

	// below here defined by x86 hardware
	unsigned int err;
	unsigned int eip;
	unsigned short cs;
	unsigned short padding5;
	unsigned int eflags;

	// below here only when crossing rings, such as from user to kernel
	unsigned int esp;
	unsigned short ss;
	unsigned short padding6;
};

#endif