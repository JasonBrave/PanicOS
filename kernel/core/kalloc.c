/*
 * Physical memory allocator
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

#include <common/spinlock.h>
#include <core/mmu.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

void freerange(void* vstart, void* vend);
extern char end[]; // first address after kernel loaded from ELF file
				   // defined by the kernel linker script in kernel.ld

struct run {
	struct run* next;
};

struct {
	struct spinlock lock;
	int use_lock;
	struct run* freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void kinit1(void* vstart, void* vend) {
	initlock(&kmem.lock, "kmem");
	kmem.use_lock = 0;
	freerange(vstart, vend);
}

void kinit2(void* vstart, void* vend) {
	freerange(vstart, vend);
	kmem.use_lock = 1;
}

void freerange(void* vstart, void* vend) {
	char* p;
	p = (char*)PGROUNDUP((unsigned int)vstart);
	for (; p + PGSIZE <= (char*)vend; p += PGSIZE)
		kfree(p);
}
// PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void* v) {
	struct run* r;

	if ((unsigned int)v % PGSIZE || (char*)v < end || V2P(v) >= PHYSTOP)
		panic("kfree");

	// Fill with junk to catch dangling refs.
	memset(v, 1, PGSIZE);

	if (kmem.use_lock)
		acquire(&kmem.lock);
	r = (struct run*)v;
	r->next = kmem.freelist;
	kmem.freelist = r;
	if (kmem.use_lock)
		release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void) {
	struct run* r;

	if (kmem.use_lock)
		acquire(&kmem.lock);
	r = kmem.freelist;
	if (r)
		kmem.freelist = r->next;
	if (kmem.use_lock)
		release(&kmem.lock);
	return r;
}
