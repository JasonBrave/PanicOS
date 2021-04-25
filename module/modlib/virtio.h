/*
 * Kernel module virtio helper functions
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

#ifndef _MODLIB_VIRTIO_H
#define _MODLIB_VIRTIO_H

#include <kernsrv.h>

#define VIRTIO_MAX_NUM_VIRTQUEUE 8

struct VirtioQueue;

struct VirtioDevice {
	unsigned int device_id;
	const struct VirtioDriver* driver;
	void* private;
	struct spinlock lock;
	union {
		struct PCIDevice* pcidev;
	};
	struct {
		unsigned int transport_is_pci : 1;
		unsigned int is_legacy : 1;
	};
	struct VirtioQueue* virtqueue[VIRTIO_MAX_NUM_VIRTQUEUE];
	// registers
	volatile struct VirtioPciCommonConfig* cmcfg;
	volatile unsigned int* isr;
	volatile void* devcfg;
	volatile unsigned int* notify_begin;
	unsigned int notify_off_multiplier;
};

struct VirtioDriver {
	const char* name;
	unsigned int legacy_device_id, device_id;
	unsigned int features;
	void (*init)(struct VirtioDevice*, unsigned int features);
	void (*uninit)(struct VirtioDevice*);
};

#define VIRTIO_QUEUE_SIZE_MAX 256

struct VirtqDesc {
	/* Address (guest-physical). */
	uint64_t addr;
	/* Length. */
	uint32_t len;

/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT 1
/* This marks a buffer as device write-only (otherwise device read-only). */
#define VIRTQ_DESC_F_WRITE 2
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT 4
	/* The flags as indicated above. */
	uint16_t flags;
	/* Next field if flags & NEXT */
	uint16_t next;
};

struct VirtqAvail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[VIRTIO_QUEUE_SIZE_MAX];
	uint16_t used_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct VirtqUsed {
#define VIRTQ_USED_F_NO_NOTIFY 1
	uint16_t flags;
	uint16_t idx;
	struct virtq_used_elem {
		/* Index of start of used descriptor chain. */
		uint32_t id;
		/* Total length of the descriptor chain which was used (written to) */
		uint32_t len;
	} ring[VIRTIO_QUEUE_SIZE_MAX];
	uint16_t avail_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct VirtioQueue {
	struct VirtioDevice* virtio_dev;
	void (*intr_handler)(struct VirtioQueue*);
	int size;
	volatile struct VirtqDesc* desc;
	volatile struct VirtqAvail* avail;
	volatile struct VirtqUsed* used;
	volatile unsigned int* notify;
};

static inline void virtio_register_driver(const struct VirtioDriver* driver) {
	return kernsrv->virtio_register_driver(driver);
}

static inline void virtio_init_queue(struct VirtioDevice* dev, struct VirtioQueue* queue,
									 unsigned int queue_n,
									 void (*intr_handler)(struct VirtioQueue*)) {
	return kernsrv->virtio_init_queue(dev, queue, queue_n, intr_handler);
}

static inline void virtio_queue_notify(struct VirtioDevice* dev, struct VirtioQueue* queue) {
	*queue->notify = 0;
}

static inline void virtio_queue_notify_wait(struct VirtioDevice* dev, struct VirtioQueue* queue) {
	int prev = queue->used->idx;
	*queue->notify = 0;
	while (prev == queue->used->idx) {
	}
}

static inline void virtio_queue_avail_insert(struct VirtioQueue* queue, int desc) {
	queue->avail->ring[queue->avail->idx % queue->size] = desc;
	queue->avail->idx++;
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
