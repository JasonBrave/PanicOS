/*
 * VirtIO block device driver
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

#include <common/errorcode.h>
#include <defs.h>
#include <driver/pci/pci.h>
#include <memlayout.h>

#include "virtio-blk-regs.h"
#include "virtio-blk.h"

struct VirtioBlockDevice virtio_blk_dev[VIRTIO_BLK_NUM_MAX];

static void virtio_blk_intr(struct PCIDevice* pcidev) {
	struct VirtioBlockDevice* dev = pcidev->private;
	if (!dev) {
		panic("invaild virtio block interrupt");
	}

	acquire(&dev->lock);
	if (!(virtio_intr_ack(&dev->virtio_dev) & 1)) {
		panic("virtio isr");
	}
	static unsigned short last = 0;
	for (; last != dev->virtio_queue.used->idx; last++) {
		unsigned int id =
			dev->virtio_queue.used->ring[last % dev->virtio_queue.size].id;
		volatile struct VirtqDesc* desc = &dev->virtio_queue.desc[id];
		if (desc->flags & VIRTQ_DESC_F_NEXT) {
			desc = &dev->virtio_queue.desc[desc->next];
			wakeup(P2V((phyaddr_t)desc->addr));
		}
		virtio_free_desc(&dev->virtio_queue, id);
	}
	release(&dev->lock);
}

static void virtio_blk_req(struct VirtioBlockDevice* dev, int type, unsigned int sect,
						   unsigned int count, phyaddr_t dest, uint8_t* status) {
	volatile struct {
		uint32_t type;
		uint32_t reserved;
		uint64_t sector;
	} __attribute__((packed)) buf0;

	buf0.type = type;
	buf0.reserved = 0;
	buf0.sector = sect;

	int desc[3];
	if (!virtio_alloc_desc(&dev->virtio_queue, desc, 3)) {
		panic("virtio-blk out of desc");
	}

	dev->virtio_queue.desc[desc[0]].addr = V2P(&buf0);
	dev->virtio_queue.desc[desc[0]].len = 16;
	dev->virtio_queue.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->virtio_queue.desc[desc[0]].next = desc[1];

	dev->virtio_queue.desc[desc[1]].addr = dest;
	dev->virtio_queue.desc[desc[1]].len = 512 * count;
	dev->virtio_queue.desc[desc[1]].flags = VIRTQ_DESC_F_NEXT;
	if (type == VIRTIO_BLK_T_IN) {
		dev->virtio_queue.desc[desc[1]].flags |= VIRTQ_DESC_F_WRITE;
	}
	dev->virtio_queue.desc[desc[1]].next = desc[2];

	dev->virtio_queue.desc[desc[2]].addr = V2P(status);
	dev->virtio_queue.desc[desc[2]].len = 1;
	dev->virtio_queue.desc[desc[2]].flags = VIRTQ_DESC_F_WRITE;
	dev->virtio_queue.desc[desc[2]].next = 0;

	virtio_queue_avail_insert(&dev->virtio_queue, desc[0]);

	// do not sleep at boot time
	if (myproc()) {
		virtio_queue_notify(&dev->virtio_dev);
		sleep(P2V(dest), &dev->lock);
	} else {
		virtio_queue_notify_wait(&dev->virtio_dev, &dev->virtio_queue);
	}
}

int virtio_blk_read(int id, unsigned int begin, int count, void* buf) {
	// check buf for DMA
	if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP ||
		(phyaddr_t)buf % PGSIZE)
		panic("virtio dma");
	if (count == 0 || count > 8)
		panic("virtio count");
	phyaddr_t dest = V2P(buf);
	struct VirtioBlockDevice* dev = &virtio_blk_dev[id];
	uint8_t status;
	// start request
	acquire(&dev->lock);
	virtio_blk_req(dev, VIRTIO_BLK_T_IN, begin, count, dest, &status);
	release(&dev->lock);
	if (status) {
		return ERROR_READ_FAIL;
	}
	return 0;
}

int virtio_blk_write(int id, unsigned int begin, int count, const void* buf) {
	// check buf for DMA
	if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP ||
		(phyaddr_t)buf % PGSIZE)
		panic("virtio dma");
	if (count == 0 || count > 8)
		panic("virtio count");
	phyaddr_t dest = V2P(buf);
	struct VirtioBlockDevice* dev = &virtio_blk_dev[id];
	uint8_t status;
	// start request
	acquire(&dev->lock);
	virtio_blk_req(dev, VIRTIO_BLK_T_OUT, begin, count, dest, &status);
	release(&dev->lock);
	if (status) {
		return ERROR_READ_FAIL;
	}
	return 0;
}

static struct VirtioBlockDevice* virtio_blk_alloc_dev(void) {
	int id = -1;
	for (int i = 0; i < VIRTIO_BLK_NUM_MAX; i++) {
		if (!virtio_blk_dev[i].virtio_dev.cmcfg) {
			id = i;
			break;
		}
	}
	if (id == -1) {
		panic("too many virtio block device");
	}
	return &virtio_blk_dev[id];
}

static void virtio_blk_dev_init(struct PCIDevice* pcidev) {
	const struct PciAddress* addr = &pcidev->addr;
	// enable bus mastering
	uint16_t pcicmd = pci_read_config_reg16(addr, 4);
	pcicmd |= 4;
	pci_write_config_reg16(addr, 4, pcicmd);
	// allocate device
	struct VirtioBlockDevice* dev = virtio_blk_alloc_dev();
	pcidev->private = dev;
	// register PCI interrupt handler
	pci_register_intr_handler(pcidev, virtio_blk_intr);
	// initialize lock
	initlock(&dev->lock, "virtio-blk");
	acquire(&dev->lock);
	// allocate memory for queues
	virtio_read_cap(addr, &dev->virtio_dev);
	virtio_allocate_queue(&dev->virtio_queue);
	virtio_setup_queue(&dev->virtio_dev, &dev->virtio_queue,
					   VIRTIO_BLK_F_RO | VIRTIO_BLK_F_BLK_SIZE);
	// print a message
	cprintf("[virtio-blk] Virtio Block device %d:%d.%d capacity %d "
			"blk_size %d\n",
			addr->bus, addr->device, addr->function,
			(unsigned int)dev->virtio_dev.devcfg->capacity,
			dev->virtio_dev.devcfg->blk_size);
	release(&dev->lock);
}

const struct PCIDeviceID virtio_blk_device_id[] = {
	{0x1af4, 0x1001}, // Legacy
	{0x1af4, 0x1042}, // Modern
	{},
};

const struct PCIDriver virtio_blk_pci_driver = {
	.name = "virtio-blk",
	.match_table = virtio_blk_device_id,
	.init = virtio_blk_dev_init,
};

void virtio_blk_init(void) {
	memset(virtio_blk_dev, 0, sizeof(virtio_blk_dev));
	pci_register_driver(&virtio_blk_pci_driver);
}
