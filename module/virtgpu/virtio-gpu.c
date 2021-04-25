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
#include <memory.h>
#include <virtio.h>

#include "virtio-gpu-regs.h"
#include "virtio-gpu.h"

static void virtio_gpu_controlq_intr(struct VirtioQueue* queue) {
	struct VirtioGPUDevice* dev = queue->virtio_dev->private;
	acquire(&dev->lock);
	volatile struct virtio_gpu_config* gpucfg = dev->virtio_dev->devcfg;
	if (gpucfg->events_read) {
		cprintf("[virtio-gpu] event %x\n", gpucfg->events_read);
		gpucfg->events_clear = gpucfg->events_read;
	}
	static unsigned short last = 0;
	for (; last != queue->used->idx; last++) {
	}
	release(&dev->lock);
}

static void virtio_gpu_cursorq_intr(struct VirtioQueue* queue) {
	return;
}

unsigned int virtio_gpu_alloc_resource_id(struct VirtioGPUDevice* dev) {
	unsigned int id = dev->res_id;
	dev->res_id++;
	return id;
}

void virtio_gpu_init(struct VirtioDevice* virtio_dev, unsigned int features) {
	// alloc dev
	struct VirtioGPUDevice* dev = kalloc();
	memset(dev, 0, sizeof(struct VirtioGPUDevice));
	virtio_dev->private = dev;
	dev->virtio_dev = virtio_dev;
	dev->res_id = 1; // assign resource_id from 1
	// initialize lock
	initlock(&dev->lock, "virtio-gpu");
	acquire(&dev->lock);
	// initialize queues
	virtio_init_queue(dev->virtio_dev, &dev->controlq, 0, virtio_gpu_controlq_intr);
	virtio_init_queue(dev->virtio_dev, &dev->cursorq, 1, virtio_gpu_cursorq_intr);

	cprintf("[virtio-gpu] Feature 3D%s EDID%s UUID%s\n", BOOL2SIGN(features & VIRTIO_GPU_F_VIRGL),
			BOOL2SIGN(features & VIRTIO_GPU_F_EDID),
			BOOL2SIGN(features & VIRTIO_GPU_F_RESOURCE_UUID));
	if (features & VIRTIO_GPU_F_EDID) {
		dev->features.edid = 1;
	}
	if (features & VIRTIO_GPU_F_VIRGL) {
		dev->features.virgl = 1;
	}
	release(&dev->lock);
	virtio_gpu_display_dev_init(dev);
}

static struct VirtioDriver virtio_gpu_virtio_driver;

void module_init(void) {
	virtio_gpu_virtio_driver.name = "virtio-gpu";
	virtio_gpu_virtio_driver.legacy_device_id = 0; // no legacy variant
	virtio_gpu_virtio_driver.device_id = 16;
	virtio_gpu_virtio_driver.features = VIRTIO_GPU_F_EDID;
	virtio_gpu_virtio_driver.init = virtio_gpu_init;
	virtio_gpu_virtio_driver.uninit = 0;
	virtio_gpu_display_global_init();
	virtio_register_driver(&virtio_gpu_virtio_driver);
}
