/*
 * Virtio helper functions
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

#ifndef _LIBVIRTIO_H
#define _LIBVIRTIO_H

#include <kernel.h>
#include <klibc.h>

#include "virtio-gpu-regs.h"

struct VirtioDevice {
	volatile struct VirtioPciCommonConfig* cmcfg;
	volatile unsigned int* isr;
	volatile unsigned int* notify;
	volatile struct virtio_gpu_config* devcfg;
};

struct VirtioQueue {
	int size;
	volatile struct VirtqDesc* desc;
	volatile struct VirtqAvail* avail;
	volatile struct VirtqUsed* used;
};

// PCI helper functions
static inline int pci_read_capid(const struct PciAddress* addr, int capptr) {
	return pci_read_config_reg8(addr, capptr);
}

static inline int pci_read_next_cap(const struct PciAddress* addr, int capptr) {
	return pci_read_config_reg8(addr, capptr + 1);
}

static inline void pci_read_cap(const struct PciAddress* addr, int capptr, void* buf,
								int size) {
	uint8_t* p = buf;
	for (int i = 0; i < size; i++) {
		p[i] = pci_read_config_reg8(addr, capptr + i);
	}
}

static inline void virtio_allocate_queue(struct VirtioQueue* queue) {
	queue->desc = kalloc();
	queue->avail = kalloc();
	queue->used = kalloc();
	memset((void*)queue->desc, 0, 4096);
	memset((void*)queue->avail, 0, 4096);
	memset((void*)queue->used, 0, 4096);
}

struct VirtioPciCap {
	uint8_t cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */
	uint8_t cap_next; /* Generic PCI field: next ptr. */
	uint8_t cap_len; /* Generic PCI field: capability length */
	uint8_t cfg_type; /* Identifies the structure. */
	uint8_t bar; /* Where to find it. */
	uint8_t padding[3]; /* Pad to full dword. */
	uint32_t offset; /* Offset within bar. */
	uint32_t length; /* Length of the structure, in bytes. */
};

static inline void virtio_read_cap(const struct PciAddress* addr,
								   struct VirtioDevice* dev) {
	int capptr = pci_read_config_reg8(addr, 0x34);
	int capid;
	do {
		capid = pci_read_capid(addr, capptr);
		if (capid == 9) {
			struct VirtioPciCap vcap;
			pci_read_cap(addr, capptr, &vcap, sizeof(vcap));
			switch (vcap.cfg_type) {
			case VIRTIO_PCI_CAP_COMMON_CFG:
				dev->cmcfg = (void*)pci_read_bar(addr, vcap.bar) + vcap.offset;
				break;
			case VIRTIO_PCI_CAP_NOTIFY_CFG:
				dev->notify = (void*)pci_read_bar(addr, vcap.bar) + vcap.offset;
				break;
			case VIRTIO_PCI_CAP_ISR_CFG:
				dev->isr = (void*)pci_read_bar(addr, vcap.bar) + vcap.offset;
				break;
			case VIRTIO_PCI_CAP_DEVICE_CFG:
				dev->devcfg = (void*)pci_read_bar(addr, vcap.bar) + vcap.offset;
				break;
			}
		}
	} while ((capptr = pci_read_next_cap(addr, capptr)) != 0);
}

static inline void virtio_set_feature(struct VirtioDevice* dev, unsigned int feature) {
	dev->cmcfg->device_status = 0;
	dev->cmcfg->device_status = 1;
	dev->cmcfg->device_status |= 2;
	dev->cmcfg->driver_feature = dev->cmcfg->device_feature & feature;
	dev->cmcfg->device_status |= 8;
	dev->cmcfg->device_status |= 4;
}

static inline void virtio_setup_queue(struct VirtioDevice* dev,
									  struct VirtioQueue* queue, int queue_n) {
	dev->cmcfg->queue_select = queue_n;
	if (dev->cmcfg->queue_size > VIRTIO_QUEUE_SIZE_MAX) {
		dev->cmcfg->queue_size = VIRTIO_QUEUE_SIZE_MAX;
	}
	queue->size = dev->cmcfg->queue_size;
	dev->cmcfg->queue_desc = V2P(queue->desc);
	dev->cmcfg->queue_driver = V2P(queue->avail);
	dev->cmcfg->queue_device = V2P(queue->used);
	dev->cmcfg->queue_enable = 1;
}

static inline int virtio_intr_ack(struct VirtioDevice* dev) {
	return *dev->isr;
}

static inline void virtio_queue_notify_wait(struct VirtioDevice* dev,
											struct VirtioQueue* queue) {
	int prev = queue->used->idx;
	*dev->notify = 0;
	while (prev == queue->used->idx) {
	}
}

static inline void virtio_queue_avail_insert(struct VirtioQueue* queue, int desc) {
	queue->avail->ring[queue->avail->idx % queue->size] = desc;
	queue->avail->idx++;
}

static inline void virtio_queue_notify(struct VirtioDevice* dev) {
	*dev->notify = 0;
}

static inline int* virtio_alloc_desc(struct VirtioQueue* queue, int* desc, int num) {
	for (int i = 0; i < num; i++) {
		desc[i] = -1;
		for (int j = 0; j < queue->size; j++) {
			if (queue->desc[j].addr == 0) {
				queue->desc[j].addr = 0xffffffff;
				desc[i] = j;
				break;
			}
		}
		if (desc[i] == -1) {
			return 0;
		}
	}
	return desc;
}

static inline void virtio_free_desc(struct VirtioQueue* queue, int desc) {
	do {
		queue->desc[desc].addr = 0;
		if (queue->desc[desc].flags & VIRTQ_DESC_F_NEXT) {
			desc = queue->desc[desc].next;
		} else {
			desc = -1;
		}
	} while (desc != -1);
}

#endif
