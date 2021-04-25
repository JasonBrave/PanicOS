/*
 * VirtIO generic driver
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

#ifndef _DRIVER_VIRTIO_H
#define _DRIVER_VIRTIO_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

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

struct VirtioQueue {
	struct VirtioDevice* virtio_dev;
	void (*intr_handler)(struct VirtioQueue*);
	int size;
	volatile struct VirtqDesc* desc;
	volatile struct VirtqAvail* avail;
	volatile struct VirtqUsed* used;
	volatile unsigned int* notify;
};

#define VIRTIO_DEVICE_TABLE_SIZE 16

extern struct VirtioDevice virtio_device_table[VIRTIO_DEVICE_TABLE_SIZE];

void virtio_init_queue(struct VirtioDevice* dev, struct VirtioQueue* queue, unsigned int queue_n,
					   void (*intr_handler)(struct VirtioQueue*));
void virtio_queue_notify(struct VirtioDevice* dev, struct VirtioQueue* queue);
void virtio_queue_notify_wait(struct VirtioDevice* dev, struct VirtioQueue* queue);
void virtio_queue_avail_insert(struct VirtioQueue* queue, int desc);
int* virtio_alloc_desc(struct VirtioQueue* queue, int* desc, int num);
void virtio_free_desc(struct VirtioQueue* queue, int desc);
void virtio_register_driver(const struct VirtioDriver* driver);
void virtio_init(void);
void virtio_print_devices(void);

#endif
