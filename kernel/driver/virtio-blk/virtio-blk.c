/*
 * VirtIO block device driver
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

#define VIRTIO_BLK_VID 0x1af4
#define VIRTIO_BLK_LEGACY_DID 0x1001
#define VIRTIO_BLK_MODERN_DID 0x1042

struct VirtioBlockDevice virtio_blk_dev[VIRTIO_BLK_NUM_MAX];

static struct VirtioBlockDevice* virtio_blk_find_by_pci(const struct PciAddress* addr) {
	int id = -1;
	for (int i = 0; i < VIRTIO_BLK_NUM_MAX; i++) {
		if ((addr->bus == virtio_blk_dev[i].addr.bus) &&
			(addr->device == virtio_blk_dev[i].addr.device) &&
			(addr->function == virtio_blk_dev[i].addr.function)) {
			id = i;
		}
	}
	if (id == -1) {
		return 0;
	}
	return &virtio_blk_dev[id];
}

void virtio_blk_intr(const struct PciAddress* addr) {
	cprintf("[virtio-blk] INTR\n");

	struct VirtioBlockDevice* dev = virtio_blk_find_by_pci(addr);
	if (!dev) {
		panic("invaild virtio block interrupt");
	}

	acquire(&dev->lock);
	if (!(*dev->isr & 1)) {
		panic("virtio isr");
	}
	release(&dev->lock);
}

static void virtio_blk_init_queue(struct VirtioBlockDevice* dev) {
	dev->cmcfg->device_status = 0;
	dev->cmcfg->device_status = 1;
	dev->cmcfg->device_status |= 2;
	dev->cmcfg->driver_feature = dev->cmcfg->device_feature;
	dev->cmcfg->device_status |= 8;
	dev->cmcfg->device_status |= 4;

	dev->cmcfg->queue_select = 0;
	dev->cmcfg->queue_enable = 0;
	dev->cmcfg->queue_size = 8;
	dev->cmcfg->queue_desc = V2P(dev->virtq_desc);
	dev->cmcfg->queue_driver = V2P(dev->virtq_avail);
	dev->cmcfg->queue_device = V2P(dev->virtq_used);
	dev->cmcfg->queue_enable = 1;
}

static struct VirtioBlockDevice* virtio_blk_alloc_dev(void) {
	int id = -1;
	for (int i = 0; i < VIRTIO_BLK_NUM_MAX; i++) {
		if (!virtio_blk_dev[i].cmcfg) {
			id = i;
			break;
		}
	}
	if (id == -1) {
		panic("too many virtio block device");
	}
	return &virtio_blk_dev[id];
}

void virtio_blk_dev_init(const struct PciAddress* addr, int device_id) {
	struct VirtioBlockDevice* dev = virtio_blk_alloc_dev();

	// copy PCI address
	dev->addr = *addr;
	// allocate memory for queues
	dev->virtq_desc = (void*)kalloc();
	dev->virtq_avail = (void*)kalloc();
	dev->virtq_used = (void*)kalloc();
	memset((void*)dev->virtq_desc, 0, 4096);
	memset((void*)dev->virtq_avail, 0, 4096);
	memset((void*)dev->virtq_used, 0, 4096);
	// enable bus mastering
	uint16_t pcicmd = pci_read_config_reg16(addr, 4);
	pcicmd |= 4;
	pci_write_config_reg16(addr, 4, pcicmd);
	// register PCI interrupt handler
	pci_register_intr_handler(addr, virtio_blk_intr);
	/// read PCI BARs
	void* bar4 = (void*)pci_read_bar(addr, 4);
	if (!bar4) {
		panic("legacy virtio devices are not supported");
	}
	dev->cmcfg = bar4;
	dev->isr = bar4 + 0x1000;
	dev->notify = bar4 + 0x3000;
	dev->devcfg = bar4 + 0x2000;
	// initialize lock and setup queues
	initlock(&dev->lock, "virtio-blk");
	acquire(&dev->lock);
	virtio_blk_init_queue(dev);
	release(&dev->lock);
	// print a message
	if (device_id == VIRTIO_BLK_LEGACY_DID) {
		cprintf("[virtio-blk] Transitional Virtio Block device %d:%d.%d capacity %d\n",
				addr->bus, addr->device, addr->function, dev->devcfg->capacity);
	} else if (device_id == VIRTIO_BLK_MODERN_DID) {
		cprintf("[virtio-blk] Modern Virtio Block device %d:%d.%d capacity %d\n",
				addr->bus, addr->device, addr->function, dev->devcfg->capacity);
	}
}

void virtio_blk_init(void) {
	struct PciAddress addr;
	if (pci_find_device(&addr, VIRTIO_BLK_VID, VIRTIO_BLK_LEGACY_DID)) { // legacy
		virtio_blk_dev_init(&addr, VIRTIO_BLK_LEGACY_DID);
		while (pci_next_device(&addr, VIRTIO_BLK_VID, VIRTIO_BLK_LEGACY_DID)) {
			virtio_blk_dev_init(&addr, VIRTIO_BLK_LEGACY_DID);
		}
	}
	if (pci_find_device(&addr, VIRTIO_BLK_VID, VIRTIO_BLK_MODERN_DID)) { // modern
		virtio_blk_dev_init(&addr, VIRTIO_BLK_MODERN_DID);
		while (pci_next_device(&addr, VIRTIO_BLK_VID, VIRTIO_BLK_MODERN_DID)) {
			virtio_blk_dev_init(&addr, VIRTIO_BLK_MODERN_DID);
		}
	}
}
