/*
 * malloc function
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

#include <panicos.h>
#include <stdlib.h>

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

typedef union header {
	struct {
		union header* ptr;
		unsigned int size;
	} s;
	Align x;
} Header;

static Header base;
Header* freep;

static Header* morecore(size_t nu) {
	char* p;
	Header* hp;

	if (nu < 4096)
		nu = 4096;
	p = sbrk(nu * sizeof(Header));
	if (p == (char*)-1)
		return 0;
	hp = (Header*)p;
	hp->s.size = nu;
	free((void*)(hp + 1));
	return freep;
}

void* malloc(size_t size) {
	Header *p, *prevp;
	size_t nunits;

	nunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;
	if ((prevp = freep) == 0) {
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}
	for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
		if (p->s.size >= nunits) {
			if (p->s.size == nunits)
				prevp->s.ptr = p->s.ptr;
			else {
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freep = prevp;
			return (void*)(p + 1);
		}
		if (p == freep)
			if ((p = morecore(nunits)) == 0)
				return 0;
	}
}
