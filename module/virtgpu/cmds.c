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

#include <klibc.h>
#include <memory.h>
#include <virtio.h>

#include "virtio-gpu-regs.h"
#include "virtio-gpu.h"

int virtio_gpu_get_display_info(struct VirtioGPUDevice* dev,
								struct virtio_gpu_display_one* display_info) {
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
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	if (resp->hdr.type != VIRTIO_GPU_RESP_OK_DISPLAY_INFO) {
		cprintf("[virtio-gpu] get display info failed with 0x%x\n", resp->hdr.type);
		kfree(req);
		kfree((void*)resp);
		virtio_free_desc(&dev->controlq, desc[0]);
		release(&dev->lock);
		return -1;
	}

	memcpy(display_info, (void*)resp->pmodes,
		   sizeof(struct virtio_gpu_display_one) * VIRTIO_GPU_MAX_SCANOUTS);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
	return 0;
}

unsigned int virtio_gpu_get_edid(struct VirtioGPUDevice* dev, unsigned int scanout, void* edid) {
	acquire(&dev->lock);

	struct virtio_gpu_get_edid* req = kalloc();
	volatile struct virtio_gpu_resp_edid* resp = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_GET_EDID;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->scanout = scanout;

	int desc[2];
	if (!virtio_alloc_desc(&dev->controlq, desc, 2)) {
		panic("virtio-gpu out of desc");
	}

	dev->controlq.desc[desc[0]].addr = V2P(req);
	dev->controlq.desc[desc[0]].len = sizeof(struct virtio_gpu_get_edid);
	dev->controlq.desc[desc[0]].flags = VIRTQ_DESC_F_NEXT;
	dev->controlq.desc[desc[0]].next = desc[1];

	dev->controlq.desc[desc[1]].addr = V2P(resp);
	dev->controlq.desc[desc[1]].len = sizeof(struct virtio_gpu_resp_edid);
	dev->controlq.desc[desc[1]].flags = VIRTQ_DESC_F_WRITE;
	dev->controlq.desc[desc[1]].next = 0;

	virtio_queue_avail_insert(&dev->controlq, desc[0]);
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	if (resp->hdr.type != VIRTIO_GPU_RESP_OK_EDID) {
		cprintf("[virtio-gpu] get edid info failed with 0x%x\n", resp->hdr.type);
		kfree(req);
		kfree((void*)resp);
		virtio_free_desc(&dev->controlq, desc[0]);
		release(&dev->lock);
		return 0;
	}

	int sz = resp->size;
	memcpy(edid, (void*)resp->edid, resp->size);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
	return sz;
}

void virtio_gpu_res_create_2d(struct VirtioGPUDevice* dev, unsigned int resource_id,
							  enum virtio_gpu_formats format, unsigned int w, unsigned int h) {
	acquire(&dev->lock);

	struct virtio_gpu_resource_create_2d* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->resource_id = resource_id;
	req->format = format;
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
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

void virtio_gpu_set_scanout(struct VirtioGPUDevice* dev, unsigned int scanout,
							unsigned int resource_id, unsigned int w, unsigned int h) {
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
	req->scanout_id = scanout;
	req->resource_id = resource_id;

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
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

void virtio_gpu_flush(struct VirtioGPUDevice* dev, unsigned int resource_id, unsigned int w,
					  unsigned int h) {
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
	req->resource_id = resource_id;

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
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

void virtio_gpu_xfer_to_host_2d(struct VirtioGPUDevice* dev, unsigned int resource_id,
								unsigned int w, unsigned int h) {
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
	req->resource_id = resource_id;

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
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}

void virtio_gpu_attach_banking(struct VirtioGPUDevice* dev, unsigned int resource_id, phyaddr_t fb,
							   size_t length) {
	acquire(&dev->lock);

	struct virtio_gpu_resource_attach_backing* req = kalloc();
	volatile struct virtio_gpu_ctrl_hdr* resp = kalloc();
	struct virtio_gpu_mem_entry* mement = kalloc();

	req->hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	req->hdr.flags = 0;
	req->hdr.fence_id = 0;
	req->hdr.ctx_id = 0;
	req->resource_id = resource_id;
	req->nr_entries = 1;

	mement->addr = fb;
	mement->length = length;

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
	virtio_queue_notify_wait(dev->virtio_dev, &dev->controlq);

	kfree(req);
	kfree(mement);
	kfree((void*)resp);
	virtio_free_desc(&dev->controlq, desc[0]);
	release(&dev->lock);
}
