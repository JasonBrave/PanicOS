/*
 * x86 Message Signaled Interrupt support
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

#include "msi.h"

#define MSI_VECTOR_MAX 190
#define MSI_VECTOR_BASE 65

static struct MSIVector {
	struct {
		unsigned char used : 1;
	};
	void (*handler)(void*);
	void* private;
} msi_vector[MSI_VECTOR_MAX];

void msi_init(void) {
	memset(msi_vector, 0, sizeof(msi_vector));
}

void msi_intr(int vector) {
	int vec = vector - MSI_VECTOR_BASE;
	if (msi_vector[vec].used) {
		msi_vector[vec].handler(msi_vector[vec].private);
	} else {
		cprintf("[pci] spurious MSI interrupt vector %d\n", vector);
	}
}

static void msi_compose_msg(struct MSIMessage* msg, unsigned int vector, unsigned int lapicid) {
	msg->addr = 0xfee00000 | lapicid << 12;
	msg->data = vector;
}

int msi_alloc_vector(struct MSIMessage* msg, void (*handler)(void*), void* private) {
	for (int i = 0; i < MSI_VECTOR_MAX; i++) {
		if (!msi_vector[i].used) {
			msi_vector[i].used = 1;
			msi_vector[i].handler = handler;
			msi_vector[i].private = private;
			msi_compose_msg(msg, i + MSI_VECTOR_BASE, 0);
			return i + MSI_VECTOR_BASE;
		}
	}
	return 0;
}

void msi_free_vector(const struct MSIMessage* msg) {
	unsigned int vector = msg->data & 0xff;
	msi_vector[vector - MSI_VECTOR_BASE].used = 0;
}
