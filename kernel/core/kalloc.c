/*
 * Physical memory page allocator
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
#include <defs.h>
#include <memlayout.h>
#include <param.h>

extern char end[]; // first address after kernel loaded from ELF file defined by the
				   // kernel linker script in kernel.ld

struct run {
	struct run* next;
	unsigned int num_pages; // size in 4K pages
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
	kmem.freelist = 0;

	kmem.freelist = (void*)PGROUNDUP((unsigned int)vstart);
	kmem.freelist->next = 0;
	kmem.freelist->num_pages = ((unsigned int)vend - PGROUNDUP((unsigned int)vstart)) / PGSIZE;
}

void kinit2(void* vstart, void* vend) {
	kmem.freelist->next = (void*)PGROUNDUP((unsigned int)vstart);
	kmem.freelist->next->next = 0;
	kmem.freelist->next->num_pages =
		((unsigned int)vend - PGROUNDUP((unsigned int)vstart)) / PGSIZE;

	kmem.use_lock = 1;
}

void* pgalloc(unsigned int num_pages) {
	if (kmem.use_lock)
		acquire(&kmem.lock);

	if (!kmem.freelist)
		panic("out of memory");

	void* p = 0;
	if (kmem.freelist->num_pages >= num_pages) {
		p = kmem.freelist;
		if (kmem.freelist->num_pages > num_pages) {
			struct run* newfl = (void*)kmem.freelist + num_pages * PGSIZE;
			newfl->next = kmem.freelist->next;
			newfl->num_pages = kmem.freelist->num_pages - num_pages;
			kmem.freelist = newfl;
		} else if (kmem.freelist->num_pages == num_pages) {
			kmem.freelist = kmem.freelist->next;
		}
	} else {
		struct run* r = kmem.freelist;
		while (r->next) {
			if (r->next->num_pages >= num_pages) {
				p = r->next;
				if (r->next->num_pages > num_pages) {
					struct run* newrun = (void*)r->next + num_pages * PGSIZE;
					newrun->next = r->next->next;
					newrun->num_pages = r->next->num_pages - num_pages;
					r->next = newrun;
				} else if (r->next->num_pages == num_pages) {
					r->next = r->next->next;
				}
				break;
			}
			r = r->next;
		}
	}

	if (!p)
		panic("out of memory");

	if (kmem.use_lock)
		release(&kmem.lock);
	return p;
}

void pgfree(void* ptr, unsigned int num_pages) {
	if (kmem.use_lock)
		acquire(&kmem.lock);

	memset(ptr, 1, num_pages * PGSIZE);

	void* orighead = kmem.freelist;
	kmem.freelist = ptr;
	kmem.freelist->num_pages = num_pages;
	kmem.freelist->next = orighead;

	if (kmem.use_lock)
		release(&kmem.lock);
}

void print_memory_usage(void) {
	if (kmem.use_lock)
		acquire(&kmem.lock);

	unsigned int pages = 0, clusters = 0;
	struct run* r = kmem.freelist;
	while (r) {
		pages += r->num_pages;
		r = r->next;
		clusters++;
	}

	if (kmem.use_lock)
		release(&kmem.lock);

	cprintf("Free memory %d clusters %d pages %d MiB\n", clusters, pages, pages / 256);
}
