/*
 * Legacy Bootloader
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

// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include "../kernel/common/x86.h"
#include "../kernel/memlayout.h"
#include "../kernel/proc/exec/elf.h"

#define SECTSIZE 512
#define INITRAMFS_SECT 8192

void readseg(unsigned char*, unsigned int, unsigned int);
void load_rootfs(void);

void bootmain(void) {
	struct elfhdr* elf;
	struct proghdr *ph, *eph;
	void (*entry)(void);
	unsigned char* pa;

	elf = (struct elfhdr*)0x10000; // scratch space

	// Read 1st page off disk
	readseg((unsigned char*)elf, 4096, 0);

	// Is this an ELF executable?
	if (elf->magic != ELF_MAGIC)
		return; // let bootasm.S handle error

	// Load each program segment (ignores ph flags).
	ph = (struct proghdr*)((unsigned char*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (; ph < eph; ph++) {
		pa = (unsigned char*)ph->paddr;
		readseg(pa, ph->filesz, ph->off);
		if (ph->memsz > ph->filesz)
			stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
	}

	load_rootfs();

	// Call the entry point from the ELF header.
	// Does not return!
	entry = (void (*)(void))(elf->entry);
	entry();
}

void waitdisk(void) {
	// Wait for disk ready.
	while ((inb(0x1F7) & 0xC0) != 0x40)
		;
}

// Read a single sector at offset into dst.
void readsect(void* dst, unsigned int offset) {
	// Issue command.
	waitdisk();
	outb(0x1F2, 1); // count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20); // cmd 0x20 - read sectors

	// Read data.
	waitdisk();
	insl(0x1F0, dst, SECTSIZE / 4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void readseg(unsigned char* pa, unsigned int count, unsigned int offset) {
	unsigned char* epa;

	epa = pa + count;

	// Round down to sector boundary.
	pa -= offset % SECTSIZE;

	// Translate from bytes to sectors; kernel starts at sector 1.
	offset = (offset / SECTSIZE) + 1;

	// If this is too slow, we could read lots of sectors at a time.
	// We'd write more to memory than asked, but it doesn't matter --
	// we load in increasing order.
	for (; pa < epa; pa += SECTSIZE, offset++)
		readsect(pa, offset);
}

void load_rootfs(void) {
	void* initramfs = (void*)0x400000;
	for (int i = 0; i < 2048; i++) {
		readsect(initramfs, INITRAMFS_SECT + i);
		initramfs += SECTSIZE;
	}
}
