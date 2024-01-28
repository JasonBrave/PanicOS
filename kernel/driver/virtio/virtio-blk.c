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
#include <hal/hal.h>
#include <memlayout.h>

#include "virtio-blk-regs.h"
#include "virtio-blk.h"
#include "virtio-regs.h"
#include "virtio.h"

static void virtio_blk_req(struct VirtioBlockDevice *dev, int type, unsigned int sect,
						   unsigned int count, phyaddr_t dest, uint8_t *status) {
	volatile struct {
		uint32_t type;
		uint32_t reserved;
		uint64_t sector;
	} __attribute__((packed)) buf0;

	buf0.type = type;
	buf0.reserved = 0;
	buf0.sector = sect;

	int desc[3];
	if (!virtio_alloc_desc(&dev->requestq, desc, 3)) {
		panic("virtio-blk out of desc");
	}

	dev->requestq.desc[desc[0]].addr = V2P(&buf0);
	dev->requestq.desc[desc[0]].len = 16;
	dev->requestq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->requestq.desc[desc[0]].next = desc[1];

	dev->requestq.desc[desc[1]].addr = dest;
	dev->requestq.desc[desc[1]].len = 512 * count;
	dev->requestq.desc[desc[1]].flags = VIRTQ_DESC_F_NEXT;
	if (type == VIRTIO_BLK_T_IN) {
		dev->requestq.desc[desc[1]].flags |= VIRTQ_DESC_F_WRITE;
	}
	dev->requestq.desc[desc[1]].next = desc[2];

	dev->requestq.desc[desc[2]].addr = V2P(status);
	dev->requestq.desc[desc[2]].len = 1;
	dev->requestq.desc[desc[2]].flags = VIRTQ_DESC_F_WRITE;
	dev->requestq.desc[desc[2]].next = 0;

	virtio_queue_avail_insert(&dev->requestq, desc[0]);

// do not sleep at boot time
#ifndef __riscv
	if (myproc()) {
		virtio_queue_notify(dev->virtio_dev, &dev->requestq);
		sleep(P2V(dest), &dev->lock);
	} else {
#endif
		virtio_queue_notify_wait(dev->virtio_dev, &dev->requestq);
#ifndef __riscv
	}
#endif
}

int virtio_blk_read(void *private, unsigned int begin, int count, void *buf) {
	// check buf for DMA
	if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP || (phyaddr_t)buf % PGSIZE)
		panic("virtio dma");
	if (count == 0 || count > 8)
		panic("virtio count");
	phyaddr_t dest = V2P(buf);
	struct VirtioBlockDevice *dev = private;
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

int virtio_blk_write(void *private, unsigned int begin, int count, const void *buf) {
	// check buf for DMA
	if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP || (phyaddr_t)buf % PGSIZE)
		panic("virtio dma");
	if (count == 0 || count > 8)
		panic("virtio count");
	phyaddr_t dest = V2P(buf);
	struct VirtioBlockDevice *dev = private;
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

const struct BlockDeviceDriver virtio_blk_block_driver = {
	.block_read = virtio_blk_read,
	.block_write = virtio_blk_write,
};

static struct VirtioBlockDevice *virtio_blk_alloc_dev(void) {
	struct VirtioBlockDevice *dev = kalloc();
	memset(dev, 0, sizeof(struct VirtioBlockDevice));
	return dev;
}

static void virtio_blk_requestq_intr(struct VirtioQueue *queue) {
	struct VirtioBlockDevice *dev = queue->virtio_dev->private;

	acquire(&dev->lock);
	static unsigned short last = 0;
	for (; last != queue->used->idx; last++) {
		unsigned int id = queue->used->ring[last % queue->size].id;
		volatile struct VirtqDesc *desc = &queue->desc[id];
		if (desc->flags & VIRTQ_DESC_F_NEXT) {
			desc = &queue->desc[desc->next];
#ifndef __riscv
			wakeup(P2V((phyaddr_t)desc->addr));
#endif
		}
		virtio_free_desc(queue, id);
	}
	release(&dev->lock);
}

static void virtio_blk_dev_init(struct VirtioDevice *virtio_dev, unsigned int features) {
	struct VirtioBlockDevice *dev = virtio_blk_alloc_dev();
	virtio_dev->private = dev;
	dev->virtio_dev = virtio_dev;
	volatile struct VirtioBlockConfig *blkcfg = dev->virtio_dev->devcfg;
	// initialize lock
	initlock(&dev->lock, "virtio-blk");
	acquire(&dev->lock);
	virtio_init_queue(dev->virtio_dev, &dev->requestq, 0, virtio_blk_requestq_intr);
	// print a message
	cprintf("[virtio-blk] Virtio Block device capacity %lld "
			"blk_size %d\n",

			blkcfg->capacity, blkcfg->blk_size);
	release(&dev->lock);
	hal_block_register_device("virtio-blk", dev, &virtio_blk_block_driver);
}

const struct VirtioDriver virtio_blk_virtio_driver = {
	.name = "virtio-blk",
	.legacy_device_id = 0x1001,
	.device_id = 2,
	.features = VIRTIO_BLK_F_RO | VIRTIO_BLK_F_BLK_SIZE,
	.init = virtio_blk_dev_init,
};

void virtio_blk_init(void) {
	virtio_register_driver(&virtio_blk_virtio_driver);
}
