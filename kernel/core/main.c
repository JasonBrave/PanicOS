/*
 * main function of the kernel
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

#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <core/traps.h>
#include <defs.h>
#include <driver/pci/pci.h>
#include <filesystem/initramfs/initramfs.h>
#include <memlayout.h>
#include <param.h>

static void startothers(void);
static void mpmain(void) __attribute__((noreturn));
extern pde_t* kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int main(void) {
	kinit1(end, P2V(4 * 1024 * 1024)); // phys page allocator
	kvmalloc(); // kernel page table
	mpinit(); // detect other processors
	lapicinit(); // interrupt controller
	seginit(); // segment descriptors
	picinit(); // disable pic
	ioapicinit(); // another interrupt controller
	consoleinit(); // console hardware
	uartinit(); // serial port
	pinit(); // process table
	tvinit(); // trap vectors
	startothers(); // start other processors
	kinit2(P2V(8 * 1024 * 1024), P2V(PHYSTOP)); // must come after startothers()
	// greeting
	cprintf(" _   _       _       ___  ____  \n");
	cprintf("| | | | ___ | | ___ / _ \\/ ___| \n");
	cprintf("| |_| |/ _ \\| |/ _ \\ | | \\___ \\ \n");
	cprintf("|  _  | (_) | |  __/ |_| |___) |\n");
	cprintf("|_| |_|\\___/|_|\\___|\\___/|____/\n");
	cprintf("Welcome to HoleOS pre-alpha version, this is free software licensed "
			"under GNU General Public License v3+\n");
	pci_init();
	initramfs_init();
	userinit(); // first user process
	mpmain(); // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
static void mpenter(void) {
	switchkvm();
	seginit();
	lapicinit();
	mpmain();
}

// Common CPU setup code.
static void mpmain(void) {
	cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
	idtinit(); // load idt register
	xchg(&(mycpu()->started), 1); // tell startothers() we're up
	scheduler(); // start running processes
}

pde_t entrypgdir[]; // For entry.S

// Start the non-boot (AP) processors.
static void startothers(void) {
	extern unsigned char _binary_entryother_start[], _binary_entryother_size[];
	unsigned char* code;
	struct cpu* c;
	char* stack;

	// Write entry code to unused memory at 0x7000.
	// The linker has placed the image of entryother.S in
	// _binary_entryother_start.
	code = P2V(0x7000);
	memmove(code, _binary_entryother_start, (unsigned int)_binary_entryother_size);

	for (c = cpus; c < cpus + ncpu; c++) {
		if (c == mycpu()) // We've started already.
			continue;

		// Tell entryother.S what stack to use, where to enter, and what
		// pgdir to use. We cannot use kpgdir yet, because the AP processor
		// is running in low  memory, so we use entrypgdir for the APs too.
		stack = kalloc();
		*(void**)(code - 4) = stack + KSTACKSIZE;
		*(void (**)(void))(code - 8) = mpenter;
		*(int**)(code - 12) = (void*)V2P(entrypgdir);

		lapicstartap(c->apicid, V2P(code));

		// wait for cpu to finish mpmain()
		while (c->started == 0)
			;
	}
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.

__attribute__((__aligned__(PGSIZE))) pde_t entrypgdir[NPDENTRIES] = {
	// Map VA's [0, 4MB) to PA's [0, 4MB)
	[0] = (0) | PTE_P | PTE_W | PTE_PS,
	// Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
	[KERNBASE >> PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};
