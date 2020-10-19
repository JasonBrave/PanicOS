/*
 * Virtio GPU device driver
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

#include <kernel.h>
#include <klibc.h>

#include "libvirtio.h"
#include "virtio-gpu-regs.h"
#include "virtio-gpu.h"

static void virtio_gpu_intr(struct PCIDevice* pcidev) {
	struct VirtioGPUDevice* dev = pcidev->private;
	acquire(&dev->lock);
	virtio_intr_ack(&dev->virtio_dev);
	if (dev->virtio_dev.devcfg->events_read) {
		cprintf("[virtio-gpu] event %x\n", dev->virtio_dev.devcfg->events_read);
		dev->virtio_dev.devcfg->events_clear = dev->virtio_dev.devcfg->events_read;
	}
	static unsigned short last = 0;
	for (; last != dev->controlq.used->idx; last++) {
	}
	release(&dev->lock);
}

unsigned int virtio_gpu_alloc_resource_id(struct VirtioGPUDevice* dev) {
	unsigned int id = dev->res_id;
	dev->res_id++;
	return id;
}

void virtio_gpu_init(struct PCIDevice* pcidev) {
	// enable bus mastering
	pci_enable_bus_mastering(&pcidev->addr);
	// alloc dev
	struct VirtioGPUDevice* dev = kalloc();
	memset(dev, 0, sizeof(struct VirtioGPUDevice));
	pcidev->private = dev;
	dev->res_id = 1; // assign resource_id from 1
	// register PCI interrupt handler
	pci_register_intr_handler(pcidev, virtio_gpu_intr);
	// initialize lock
	initlock(&dev->lock, "virtio-gpu");
	acquire(&dev->lock);
	// allocate memory for queues
	virtio_read_cap(&pcidev->addr, &dev->virtio_dev);
	virtio_allocate_queue(&dev->controlq);
	virtio_allocate_queue(&dev->cursorq);
	unsigned int feature = virtio_set_feature(&dev->virtio_dev, VIRTIO_GPU_F_EDID);
	cprintf("[virtio-gpu] Feature 3D%s EDID%s UUID%s\n",
			BOOL2SIGN(feature & VIRTIO_GPU_F_VIRGL),
			BOOL2SIGN(feature & VIRTIO_GPU_F_EDID),
			BOOL2SIGN(feature & VIRTIO_GPU_F_RESOURCE_UUID));
	if (feature & VIRTIO_GPU_F_EDID) {
		dev->features.edid = 1;
	}
	if (feature & VIRTIO_GPU_F_VIRGL) {
		dev->features.virgl = 1;
	}
	virtio_setup_queue(&dev->virtio_dev, &dev->controlq, 0);
	virtio_setup_queue(&dev->virtio_dev, &dev->cursorq, 1);
	release(&dev->lock);
	virtio_gpu_display_dev_init(dev);
}

const struct PCIDeviceID virtio_gpu_device_id[] = {{0x1af4, 0x1050}, {}};

struct PCIDriver virtio_gpu_pci_driver;

void module_init(void) {
	virtio_gpu_pci_driver.name = "virtio-gpu";
	virtio_gpu_pci_driver.match_table = virtio_gpu_device_id;
	virtio_gpu_pci_driver.init = virtio_gpu_init;
	virtio_gpu_display_global_init();
	pci_register_driver(&virtio_gpu_pci_driver);
}
