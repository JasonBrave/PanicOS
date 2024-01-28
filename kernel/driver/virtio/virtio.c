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

#include <arch/x86/msi.h>
#include <defs.h>
#include <driver/pci/pci.h>
#include <memlayout.h>

#include "virtio-blk.h"
#include "virtio-regs.h"
#include "virtio.h"

#define VIRTIO_PCI_USE_MSIX

struct VirtioDevice virtio_device_table[VIRTIO_DEVICE_TABLE_SIZE];

static void virtio_read_cap(const struct PciAddress *addr, struct VirtioDevice *dev) {
	int capptr = pci_read_config_reg8(addr, 0x34);
	unsigned int cmcfg_off = 0, notify_off = 0, isr_off = 0, devcfg_off = 0;
	unsigned int mmio_bar = 0;
	do {
		if (pci_read_config_reg8(addr, capptr + VIRTIO_PCI_CAP_OFF_CAP_VENDOR) == 9) {
			switch (pci_read_config_reg8(addr, capptr + VIRTIO_PCI_CAP_OFF_TYPE)) {
			case VIRTIO_PCI_CAP_COMMON_CFG:
				mmio_bar = pci_read_config_reg8(addr, capptr + VIRTIO_PCI_CAP_OFF_BAR);
				cmcfg_off = pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			case VIRTIO_PCI_CAP_NOTIFY_CFG:
				notify_off = pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				dev->notify_off_multiplier = pci_read_config_reg32(addr, capptr + 16);
				break;
			case VIRTIO_PCI_CAP_ISR_CFG:
				isr_off = pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			case VIRTIO_PCI_CAP_DEVICE_CFG:
				devcfg_off = pci_read_config_reg32(addr, capptr + VIRTIO_PCI_CAP_OFF_OFFSET);
				break;
			}
		}
	} while ((capptr = pci_read_config_reg8(addr, capptr + 1)) != 0);
	volatile void *mmio_region
		= map_mmio_region(pci_read_bar(addr, mmio_bar), pci_read_bar_size(addr, mmio_bar));
	dev->cmcfg = mmio_region + cmcfg_off;
	dev->notify_begin = mmio_region + notify_off;
	dev->isr = mmio_region + isr_off;
	dev->devcfg = mmio_region + devcfg_off;
	cprintf("[virtio] BAR %d cmcfg %x notify %x isr %x devcfg %x\n", mmio_bar, cmcfg_off,
			notify_off, isr_off, devcfg_off);
}

static unsigned int virtio_set_feature(struct VirtioDevice *dev, unsigned int feature) {
	dev->cmcfg->device_status |= VIRTIO_STATUS_DRIVER;
	dev->cmcfg->driver_feature = dev->cmcfg->device_feature & feature;
	unsigned int ack_feature = dev->cmcfg->driver_feature;
	dev->cmcfg->device_status |= VIRTIO_STATUS_FEATURES_OK;
	dev->cmcfg->device_status |= VIRTIO_STATUS_DRIVER_OK;
	return ack_feature;
}

/*
For virtio MSI-X interrupts
the device consume num_queues vectors
vector queue_n is used for queues
*/
#ifdef VIRTIO_PCI_USE_MSIX
static void virtio_pci_queue_msix_handler(void *private) {
	struct VirtioQueue *queue = private;
	queue->intr_handler(queue);
	return;
}
#endif

static void virtio_add_virtq_to_dev(struct VirtioDevice *dev, struct VirtioQueue *queue) {
	for (int i = 0; i < VIRTIO_MAX_NUM_VIRTQUEUE; i++) {
		if (!dev->virtqueue[i]) {
			dev->virtqueue[i] = queue;
			return;
		}
	}
	panic("virtio only support max 8 virtqueue");
}

void virtio_init_queue(struct VirtioDevice *dev, struct VirtioQueue *queue, unsigned int queue_n,
					   void (*intr_handler)(struct VirtioQueue *)) {
	queue->virtio_dev = dev;
	virtio_add_virtq_to_dev(dev, queue);
	// allocate queue memory
	queue->desc = kalloc();
	queue->avail = kalloc();
	queue->used = kalloc();
	memset((void *)queue->desc, 0, 4096);
	memset((void *)queue->avail, 0, 4096);
	memset((void *)queue->used, 0, 4096);
	// select current queue
	dev->cmcfg->queue_select = queue_n;
	if (dev->cmcfg->queue_size > VIRTIO_QUEUE_SIZE_MAX) {
		dev->cmcfg->queue_size = VIRTIO_QUEUE_SIZE_MAX;
	}
	// set VirtioQueue structures
	queue->size = dev->cmcfg->queue_size;
	queue->notify = dev->notify_begin + dev->cmcfg->queue_notify_off * dev->notify_off_multiplier;
	queue->intr_handler = intr_handler;
// config MSI-X
#ifdef VIRTIO_PCI_USE_MSIX
	struct MSIMessage msix_msg;
	if (msi_alloc_vector(&msix_msg, virtio_pci_queue_msix_handler, queue) == 0) {
		cprintf("[virtio] MSI-X out MSI vector\n");
	}
	pci_msix_set_message(dev->pcidev, queue_n, &msix_msg);
	pci_msix_unmask(dev->pcidev, queue_n);
	dev->cmcfg->queue_msix_vector = queue_n; // MSI-X vector
#endif
	dev->cmcfg->queue_desc = V2P(queue->desc);
	dev->cmcfg->queue_driver = V2P(queue->avail);
	dev->cmcfg->queue_device = V2P(queue->used);
	dev->cmcfg->queue_enable = 1;
}

void virtio_queue_notify(struct VirtioDevice *dev, struct VirtioQueue *queue) {
	*queue->notify = 0;
}

void virtio_queue_notify_wait(struct VirtioDevice *dev, struct VirtioQueue *queue) {
	int prev = queue->used->idx;
	*queue->notify = 0;
	while (prev == queue->used->idx) {
	}
}

void virtio_queue_avail_insert(struct VirtioQueue *queue, int desc) {
	queue->avail->ring[queue->avail->idx % queue->size] = desc;
	queue->avail->idx++;
}

int *virtio_alloc_desc(struct VirtioQueue *queue, int *desc, int num) {
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

void virtio_free_desc(struct VirtioQueue *queue, int desc) {
	do {
		queue->desc[desc].addr = 0;
		if (queue->desc[desc].flags & VIRTQ_DESC_F_NEXT) {
			desc = queue->desc[desc].next;
		} else {
			desc = -1;
		}
	} while (desc != -1);
}

#ifndef VIRTIO_PCI_USE_MSIX
static int virtio_intr_ack(struct VirtioDevice *dev) {
	return *dev->isr;
}

static void virtio_pci_intr_handler(struct PCIDevice *pcidev) {
	struct VirtioDevice *dev = pcidev->private;
	if (virtio_intr_ack(dev) & 1) { // is queue interrupt?
		for (int i = 0; i < VIRTIO_MAX_NUM_VIRTQUEUE; i++) {
			if (dev->virtqueue[i]) {
				dev->virtqueue[i]->intr_handler(dev->virtqueue[i]);
			}
		}
	}
	return;
}
#endif

static unsigned int virtio_generic_init(struct VirtioDevice *dev, unsigned int features) {
	pci_enable_bus_mastering(&dev->pcidev->addr);
	// interrupt initialization
#ifdef VIRTIO_PCI_USE_MSIX
	pci_msix_enable(dev->pcidev);
#else
	pci_register_intr_handler(dev->pcidev, virtio_pci_intr_handler);
#endif
	// initialize lock
	initlock(&dev->lock, "virtio-blk");
	acquire(&dev->lock);
	virtio_read_cap(&dev->pcidev->addr, dev);
	// reset device
	dev->cmcfg->device_status = 0;
	dev->cmcfg->device_status = VIRTIO_STATUS_ACKNOWLEDGE;
	unsigned int ack_feature = virtio_set_feature(dev, features);
	// do not send interrupt on configuration change
	dev->cmcfg->msix_config = VIRTIO_MSI_NO_VECTOR;

	release(&dev->lock);
	return ack_feature;
}

void virtio_register_driver(const struct VirtioDriver *driver) {
	for (int i = 0; i < VIRTIO_DEVICE_TABLE_SIZE; i++) {
		if (!virtio_device_table[i].driver && virtio_device_table[i].is_legacy
			&& virtio_device_table[i].device_id == driver->legacy_device_id) {
			virtio_device_table[i].driver = driver;
			unsigned int feature = virtio_generic_init(&virtio_device_table[i], driver->features);
			driver->init(&virtio_device_table[i], feature);
		} else if (!virtio_device_table[i].driver && !virtio_device_table[i].is_legacy
				   && virtio_device_table[i].device_id == driver->device_id) {
			virtio_device_table[i].driver = driver;
			unsigned int feature = virtio_generic_init(&virtio_device_table[i], driver->features);
			driver->init(&virtio_device_table[i], feature);
		}
	}
}

static struct VirtioDevice *virtio_alloc_device(void) {
	for (int i = 0; i < VIRTIO_DEVICE_TABLE_SIZE; i++) {
		if (!virtio_device_table[i].device_id) {
			memset(&virtio_device_table[i], 0, sizeof(struct VirtioDevice));
			return &virtio_device_table[i];
		}
	}
	panic("too many virtio device");
}

static void virtio_pci_init(struct PCIDevice *pcidev) {
	struct VirtioDevice *dev = virtio_alloc_device();
	dev->transport_is_pci = 1;
	dev->pcidev = pcidev;
	pcidev->private = dev;
	uint16_t device_id = pci_read_config_reg16(&pcidev->addr, 2);
	if (device_id <= 0x1040) {
		dev->is_legacy = 1;
		dev->device_id = device_id; // legacy device use pci device id
		cprintf("[virtio] VirtIO Legacy device id %x\n", dev->device_id);
	} else {
		dev->device_id = device_id - 0x1040;
		cprintf("[virtio] VirtIO Modern device id %d\n", dev->device_id);
	}
}

static struct PCIDeviceID virtio_pci_device_id[257];

const static struct PCIDriver virtio_pci_driver = {
	.name = "virtio-pci",
	.match_table = virtio_pci_device_id,
	.init = virtio_pci_init,
};

void virtio_init(void) {
	memset(virtio_device_table, 0, sizeof(virtio_device_table));
	// virtio PCI transport
	for (int i = 0; i < 256; i++) {
		virtio_pci_device_id[i].vendor_id = 0x1af4;
		virtio_pci_device_id[i].device_id = 0x1000 + i;
	}
	virtio_pci_device_id[256].vendor_id = 0;
	virtio_pci_device_id[256].device_id = 0;
	pci_register_driver(&virtio_pci_driver);
	// virtio device types
	virtio_blk_init(); // virtio block device
}

void virtio_print_devices(void) {
	cprintf("VirtIO devices:\n");
	for (int i = 0; i < VIRTIO_DEVICE_TABLE_SIZE; i++) {
		if (virtio_device_table[i].device_id && virtio_device_table[i].driver) {
			if (virtio_device_table[i].is_legacy) {
				cprintf("VirtIO PCI %x:%x.%x Legacy device id %x driver %s\n",
						virtio_device_table[i].pcidev->addr.bus,
						virtio_device_table[i].pcidev->addr.device,
						virtio_device_table[i].pcidev->addr.function,
						virtio_device_table[i].device_id, virtio_device_table[i].driver->name);
			} else {
				cprintf("VirtIO PCI %x:%x.%x Modern device id %d driver %s\n",
						virtio_device_table[i].pcidev->addr.bus,
						virtio_device_table[i].pcidev->addr.device,
						virtio_device_table[i].pcidev->addr.function,
						virtio_device_table[i].device_id, virtio_device_table[i].driver->name);
			}
		} else if (virtio_device_table[i].device_id) {
			if (virtio_device_table[i].is_legacy) {
				cprintf("VirtIO PCI %x:%x.%x Legacy device id %x driver <none>\n",
						virtio_device_table[i].pcidev->addr.bus,
						virtio_device_table[i].pcidev->addr.device,
						virtio_device_table[i].pcidev->addr.function,
						virtio_device_table[i].device_id);
			} else {
				cprintf("VirtIO PCI %x:%x.%x Modern device id %d driver <none>\n",
						virtio_device_table[i].pcidev->addr.bus,
						virtio_device_table[i].pcidev->addr.device,
						virtio_device_table[i].pcidev->addr.function,
						virtio_device_table[i].device_id);
			}
		}
	}
}
