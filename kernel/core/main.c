/*
 * main function of the kernel
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

#include <arch/x86/lapic.h>
#include <arch/x86/msi.h>
#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <core/traps.h>
#include <defs.h>
#include <driver/ahci/ahci.h>
#include <driver/ata/ata.h>
#include <driver/bochs-display/bochs-display.h>
#include <driver/usb/usb.h>
#include <driver/virtio/virtio.h>
#include <filesystem/filesystem.h>
#include <hal/hal.h>
#include <memlayout.h>
#include <param.h>
#include <proc/kcall.h>
#include <proc/pty.h>

#include "multiboot.h"

static void startothers(void);
static void mpmain(void) __attribute__((noreturn));
extern pde_t* kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

extern void platform_init(void);

struct BootGraphicsMode boot_graphics_mode;

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
void kmain(uint32_t mb_sig, uint32_t mb_addr) {
	kinit1(end, P2V(4 * 1024 * 1024)); // phys page allocator
	kvmalloc(); // kernel page table
	cprintf("PanicOS i686 alpha built on " __DATE__ " " __TIME__ " gcc " __VERSION__ "\n");
	if (mb_sig == 0x2BADB002 && mb_addr < 0x100000) {
		cprintf("[multiboot] Multiboot bootloader detected, info at %x\n", mb_addr);
		struct multiboot_info* mbinfo = P2V(mb_addr);
		if ((mbinfo->flags & (1 << 6)) && mbinfo->mmap_addr < 0x100000) { // memory map
			unsigned long long memory_size = 0;
			cprintf("[multiboot] Memory map addr %x length %d\n", mbinfo->mmap_addr,
					mbinfo->mmap_length);
			struct multiboot_mmap_entry* mmap = P2V(mbinfo->mmap_addr);
			for (unsigned int off = 0; off < mbinfo->mmap_length;
				 off += (mmap->size + sizeof(mmap->size))) {
				mmap = P2V(mbinfo->mmap_addr + off);
				cprintf("[multiboot] MMAP %llx - %llx type %d\n", mmap->addr,
						mmap->addr + mmap->len - 1, mmap->type);
				if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
					memory_size += mmap->len;
				}
			}
			cprintf("[multiboot] Memory size %d MiB\n", memory_size / (1024 * 1024));
		}
		if (mbinfo->flags & (1 << 12)) { // video mode
			switch (mbinfo->framebuffer_type) {
			case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
				cprintf("[multiboot] VGA text mode\n");
				boot_graphics_mode.mode = BOOT_GRAPHICS_MODE_VGA_TEXT;
				break;
			case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
				cprintf("[multiboot] framebuffer addr %llx pitch %d width %d "
						"height %d bpp %d\n",
						mbinfo->framebuffer_addr, mbinfo->framebuffer_pitch,
						mbinfo->framebuffer_width, mbinfo->framebuffer_height,
						mbinfo->framebuffer_bpp);
				if (mbinfo->framebuffer_addr >= (unsigned long long)4 * 1024 * 1024 * 1024) {
					cprintf("[multiboot] Warning mbinfo->framebuffer_addr >= 4GB");
					boot_graphics_mode.mode = BOOT_GRAPHICS_MODE_HEADLESS;
				} else {
					boot_graphics_mode.mode = BOOT_GRAPHICS_MODE_FRAMEBUFFER;
					boot_graphics_mode.width = mbinfo->framebuffer_width;
					boot_graphics_mode.height = mbinfo->framebuffer_height;
					boot_graphics_mode.fb_addr = mbinfo->framebuffer_addr;
				}
				if (mbinfo->framebuffer_bpp != 32) {
					cprintf("[multiboot] Warning mbinfo->framebuffer_bpp != 32");
				}
				if (mbinfo->framebuffer_pitch != mbinfo->framebuffer_width * 4) {
					cprintf("[multiboot] Warning mbinfo->framebuffer_pitch != "
							"mbinfo->framebuffer_width * 4");
				}
				break;
			default:
				cprintf("[multiboot] unknown multiboot graphics mode %d, assuming VGA "
						"mode\n",
						mbinfo->framebuffer_type);
				boot_graphics_mode.mode = BOOT_GRAPHICS_MODE_VGA_TEXT;
			}
		} else { // graphics mode not given
			cprintf("[multiboot] graphics mode not given, assuming VGA "
					"mode\n");
			boot_graphics_mode.mode = BOOT_GRAPHICS_MODE_VGA_TEXT;
		}
	}
	mpinit(); // detect other processors
	lapicinit(); // interrupt controller
	seginit(); // segment descriptors
	msi_init();
	if (boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
		fbcon_init(boot_graphics_mode.fb_addr, boot_graphics_mode.width, boot_graphics_mode.height);
	} else {
		vgacon_init();
	}
	pinit(); // process table
	tvinit(); // trap vectors
	cprintf("[cpu] starting other cpus\n");
	startothers(); // start other processors
	kinit2(P2V(4 * 1024 * 1024), P2V(PHYSTOP));
	// greeting
	cprintf(" ____             _       ___  ____  \n");
	cprintf("|  _ \\ __ _ _ __ (_) ___ / _ \\/ ___| \n");
	cprintf("| |_) / _` | '_ \\| |/ __| | | \\___ \\ \n");
	cprintf("|  __/ (_| | | | | | (__| |_| |___) |\n");
	cprintf("|_|   \\__,_|_| |_|_|\\___|\\___/|____/ \n");
	cprintf("Welcome to PanicOS pre-alpha version, this is free software licensed "
			"under GNU General Public License v3+\n");
	// subsystems and HAL
	kcall_init();
	hal_display_init();
	hal_block_init();
	hal_hid_init();
	hal_power_init();
	pty_init();
	// device initialization
	platform_init(); // platform devices (platform dependent) and PCI
	virtio_init(); // virtio "bus" and devices
	usb_init(); // usb bus and devices
	// kernel built-in drivers
	ata_init(); // parallel ata and ata subsystem
	ahci_init(); // AHCI driver
	bochs_display_init(); // qemu virtual display adapter
	// filesystem and user-mode
	filesystem_init();
	module_init(); // kernel modules
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
	cprintf("[cpu] starting %d\n", cpuid());
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
