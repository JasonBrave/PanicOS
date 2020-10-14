/*
 * VirtIO generic device driver
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
#include <driver/pci/pci.h>
#include <memlayout.h>

#include "virtio-blk-regs.h"
#include "virtio-blk.h"

void virtio_allocate_queue(struct VirtioQueue* queue) {
	queue->desc = kalloc();
	queue->avail = kalloc();
	queue->used = kalloc();
	memset((void*)queue->desc, 0, 4096);
	memset((void*)queue->avail, 0, 4096);
	memset((void*)queue->used, 0, 4096);
}

void virtio_read_cap(const struct PciAddress* addr, struct VirtioDevice* dev) {
	int capptr = pci_read_config_reg8(addr, 0x34);
	unsigned int cmcfg_off = 0, notify_off = 0, isr_off = 0, devcfg_off = 0;
	unsigned int mmio_bar = 0;
	do {
		if (pci_read_config_reg8(addr, capptr + VIRTIO_PCI_CAP_OFF_CAP_VENDOR) == 9) {
			switch (pci_read_config_reg8(addr, capptr + VIRTIO_PCI_CAP_OFF_TYPE)) {
			case VIRTIO_PCI_CAP_COMMON_CFG:
				mmio_bar = pci_read_config_reg8(addr, capptr + VIRTIO_PCI_CAP_OFF_BAR);
				cmcfg_off =
					pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			case VIRTIO_PCI_CAP_NOTIFY_CFG:
				notify_off =
					pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			case VIRTIO_PCI_CAP_ISR_CFG:
				isr_off =
					pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			case VIRTIO_PCI_CAP_DEVICE_CFG:
				devcfg_off =
					pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			}
		}
	} while ((capptr = pci_read_config_reg8(addr, capptr + 1)) != 0);
	volatile void* mmio_region = mmio_map_region(pci_read_bar(addr, mmio_bar),
												 pci_read_bar_size(addr, mmio_bar));
	dev->cmcfg = mmio_region + cmcfg_off;
	dev->notify = mmio_region + notify_off;
	dev->isr = mmio_region + isr_off;
	dev->devcfg = mmio_region + devcfg_off;
	cprintf("[virtio-blk] BAR %d cmcfg %x notify %x isr %x devcfg %x\n", mmio_bar,
			cmcfg_off, notify_off, isr_off, devcfg_off);
}

void virtio_set_feature(struct VirtioDevice* dev, unsigned int feature) {
	dev->cmcfg->device_status = 0;
	dev->cmcfg->device_status = 1;
	dev->cmcfg->device_status |= 2;
	dev->cmcfg->driver_feature = dev->cmcfg->device_feature & feature;
	dev->cmcfg->device_status |= 8;
	dev->cmcfg->device_status |= 4;
}

void virtio_setup_queue(struct VirtioDevice* dev, struct VirtioQueue* queue,
						int queue_n) {
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

int virtio_intr_ack(struct VirtioDevice* dev) {
	return *dev->isr;
}

void virtio_queue_notify_wait(struct VirtioDevice* dev, struct VirtioQueue* queue) {
	int prev = queue->used->idx;
	*dev->notify = 0;
	while (prev == queue->used->idx) {
	}
}

void virtio_queue_avail_insert(struct VirtioQueue* queue, int desc) {
	queue->avail->ring[queue->avail->idx % queue->size] = desc;
	queue->avail->idx++;
}

void virtio_queue_notify(struct VirtioDevice* dev) {
	*dev->notify = 0;
}

int* virtio_alloc_desc(struct VirtioQueue* queue, int* desc, int num) {
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

void virtio_free_desc(struct VirtioQueue* queue, int desc) {
	do {
		queue->desc[desc].addr = 0;
		if (queue->desc[desc].flags & VIRTQ_DESC_F_NEXT) {
			desc = queue->desc[desc].next;
		} else {
			desc = -1;
		}
	} while (desc != -1);
}
