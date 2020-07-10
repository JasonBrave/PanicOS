/*
 * VirtIO generic device driver
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

#include <defs.h>
#include <driver/pci/pci.h>
#include <memlayout.h>

#include "virtio-blk-regs.h"
#include "virtio-blk.h"

#define VIRTIO_VID 0x1af4
#define VIRTIO_LEGACY_DID 0x0fff
#define VIRTIO_MODERN_DID 0x1040

void virtio_enum_device(int device_id,
						void (*devinitfunc)(const struct PciAddress* addr)) {
	struct PciAddress addr;
	if (pci_find_device(&addr, VIRTIO_VID, VIRTIO_LEGACY_DID + device_id)) { // legacy
		devinitfunc(&addr);
		while (pci_next_device(&addr, VIRTIO_VID, VIRTIO_LEGACY_DID + device_id)) {
			devinitfunc(&addr);
		}
	}
	if (pci_find_device(&addr, VIRTIO_VID, VIRTIO_MODERN_DID + device_id)) { // modern
		devinitfunc(&addr);
		while (pci_next_device(&addr, VIRTIO_VID, VIRTIO_MODERN_DID + device_id)) {
			devinitfunc(&addr);
		}
	}
}

void virtio_allocate_queue(struct VirtioQueue* queue) {
	queue->desc = (void*)kalloc();
	queue->avail = (void*)kalloc();
	queue->used = (void*)kalloc();
	memset((void*)queue->desc, 0, 4096);
	memset((void*)queue->avail, 0, 4096);
	memset((void*)queue->used, 0, 4096);
}

void virtio_read_cap(const struct PciAddress* addr, struct VirtioDevice* dev) {
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

void virtio_setup_queue(struct VirtioDevice* dev, struct VirtioQueue* queue) {
	dev->cmcfg->device_status = 0;
	dev->cmcfg->device_status = 1;
	dev->cmcfg->device_status |= 2;
	dev->cmcfg->driver_feature = dev->cmcfg->device_feature;
	dev->cmcfg->device_status |= 8;
	dev->cmcfg->device_status |= 4;

	dev->cmcfg->queue_select = 0;
	dev->cmcfg->queue_enable = 0;
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
