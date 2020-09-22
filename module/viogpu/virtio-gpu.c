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

struct VirtioGPUDevice {
	struct VirtioDevice virtio_dev;
	struct VirtioQueue controlq;
	struct VirtioQueue cursorq;
	struct spinlock lock;
	int xres, yres;
	phyaddr_t framebuffer;
};

static void virtio_gpu_intr(struct PCIDevice* pcidev) {
	struct VirtioGPUDevice* dev = pcidev->private;
	virtio_intr_ack(&dev->virtio_dev);
}

static int virtio_gpu_get_display_info(struct VirtioGPUDevice* dev) {
	acquire(&dev->lock);

	struct virtio_gpu_ctrl_hdr* req = kalloc();
	volatile struct virtio_gpu_resp_display_info* resp = kalloc();

	req->type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
	req->flags = 0;
	req->fence_id = 0;
	req->ctx_id = 0;

	int desc[2];
	if (!virtio_alloc_desc(&dev->controlq, desc, 2)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_ctrl_hdr);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(resp);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_resp_display_info);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[1]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(&dev->virtio_dev, &dev->controlq);

	if (resp->hdr.type != VIRTIO_GPU_RESP_OK_DISPLAY_INFO) {
		cprintf("[virtio-gpu] get display info failed with 0x%x\n", resp->hdr.type);
		kfree(req);
		kfree((void*)resp);
		virtio_free_desc(&dev->controlq, desc[0]);
		release(&dev->lock);
		return -1;
	}

	cprintf("[virtio-gpu] display width %d height %d\n", resp->pmodes[0].r.width,
			resp->pmodes[0].r.height);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
	return 0;
}

static void virtio_gpu_res_create_2d(struct VirtioGPUDevice* dev, int w, int h) {
	acquire(&dev->lock);

	struct virtio_gpu_resource_create_2d* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->resource_id = 1;
	req->format = VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM;
	req->width = w;
	req->height = h;

	int desc[2];
	if (!virtio_alloc_desc(&dev->controlq, desc, 2)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_resource_create_2d);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(resp);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_ctrl_hdr);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[1]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(&dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

static void virtio_gpu_attach_banking(struct VirtioGPUDevice* dev, phyaddr_t fb) {
	acquire(&dev->lock);

	struct virtio_gpu_resource_attach_backing* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();
	struct virtio_gpu_mem_entry* mement = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->resource_id = 1;
	req->nr_entries = 1;

	mement->addr = fb;
	mement->length = 32 * 1024 * 1024;

	int desc[3];
	if (!virtio_alloc_desc(&dev->controlq, desc, 3)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_resource_attach_backing);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(mement);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_mem_entry);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[1]].next = desc[2];

	dev->controlq.desc[desc[2]].addr = V2P(resp);
	dev->controlq.desc[desc[2]].len = sizeof(struct virtio_gpu_ctrl_hdr);
	dev->controlq.desc[desc[2]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[2]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(&dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree(mement);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

static void virtio_gpu_set_scanout(struct VirtioGPUDevice* dev, int w, int h) {
	acquire(&dev->lock);

	struct virtio_gpu_set_scanout* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_SET_SCANOUT;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->r.x = 0;
	req->r.y = 0;
	req->r.width = w;
	req->r.height = h;
	req->scanout_id = 0;
	req->resource_id = 1;

	int desc[2];
	if (!virtio_alloc_desc(&dev->controlq, desc, 2)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_set_scanout);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(resp);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_ctrl_hdr);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[1]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(&dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

static void virtio_gpu_xfer_to_host_2d(struct VirtioGPUDevice* dev, int w, int h) {
	acquire(&dev->lock);

	struct virtio_gpu_transfer_to_host_2d* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->r.x = 0;
	req->r.y = 0;
	req->r.width = w;
	req->r.height = h;
	req->offset = 0;
	req->resource_id = 1;

	int desc[2];
	if (!virtio_alloc_desc(&dev->controlq, desc, 2)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_transfer_to_host_2d);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(resp);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_ctrl_hdr);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[1]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(&dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

static void virtio_gpu_flush(struct VirtioGPUDevice* dev, int w, int h) {
	acquire(&dev->lock);

	struct virtio_gpu_resource_flush* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->r.x = 0;
	req->r.y = 0;
	req->r.width = w;
	req->r.height = h;
	req->resource_id = 1;

	int desc[2];
	if (!virtio_alloc_desc(&dev->controlq, desc, 2)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_resource_flush);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(resp);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_ctrl_hdr);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[1]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(&dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

static void virtio_gpu_update(void* private) {
	struct VirtioGPUDevice* dev = private;
	virtio_gpu_xfer_to_host_2d(dev, dev->xres, dev->yres);
	virtio_gpu_flush(dev, dev->xres, dev->yres);
}

static phyaddr_t virtio_gpu_display_enable(void* private, int xres, int yres) {
	struct VirtioGPUDevice* dev = private;
	dev->framebuffer = V2P(pgalloc(4096));
	virtio_gpu_res_create_2d(private, xres, yres);
	virtio_gpu_attach_banking(private, dev->framebuffer);
	virtio_gpu_set_scanout(private, xres, yres);
	dev->xres = xres;
	dev->yres = yres;
	return dev->framebuffer;
}

static void virtio_gpu_display_disable(void* private) {
	struct VirtioGPUDevice* dev = private;
	pgfree(P2V(dev->framebuffer), 4096);
}

struct FramebufferDriver virtio_gpu_fb_driver;

void virtio_gpu_init(struct PCIDevice* pcidev) {
	// enable bus mastering
	pci_enable_bus_mastering(&pcidev->addr);
	// alloc dev
	struct VirtioGPUDevice* dev = kalloc();
	memset(dev, 0, sizeof(struct VirtioGPUDevice));
	pcidev->private = dev;
	// register PCI interrupt handler
	pci_register_intr_handler(pcidev, virtio_gpu_intr);
	// initialize lock
	initlock(&dev->lock, "virtio-gpu");
	acquire(&dev->lock);
	// allocate memory for queues
	virtio_read_cap(&pcidev->addr, &dev->virtio_dev);
	virtio_allocate_queue(&dev->controlq);
	virtio_allocate_queue(&dev->cursorq);
	virtio_set_feature(&dev->virtio_dev, 0);
	virtio_setup_queue(&dev->virtio_dev, &dev->controlq, 0);
	virtio_setup_queue(&dev->virtio_dev, &dev->cursorq, 1);
	release(&dev->lock);
	if (virtio_gpu_get_display_info(dev)) {
		return;
	}
	hal_display_register_device("virtio-gpu", dev, &virtio_gpu_fb_driver);
}

const struct PCIDeviceID virtio_gpu_device_id[] = {{0x1af4, 0x1050}, {}};

struct PCIDriver virtio_gpu_pci_driver;

void module_init(void) {
	virtio_gpu_pci_driver.name = "virtio-gpu";
	virtio_gpu_pci_driver.match_table = virtio_gpu_device_id;
	virtio_gpu_pci_driver.init = virtio_gpu_init;
	virtio_gpu_fb_driver.enable = virtio_gpu_display_enable;
	virtio_gpu_fb_driver.disable = virtio_gpu_display_disable;
	virtio_gpu_fb_driver.update = virtio_gpu_update;
	pci_register_driver(&virtio_gpu_pci_driver);
}
