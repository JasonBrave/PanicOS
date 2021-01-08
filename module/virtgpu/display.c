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

#include <hal.h>
#include <klibc.h>
#include <memory.h>
#include <virtio.h>

#include "virtio-gpu.h"

static void virtio_gpu_display_update(void* private) {
	struct VirtioGPUDisplay* disp = private;
	virtio_gpu_xfer_to_host_2d(disp->gpu, disp->resource_id, disp->xres, disp->yres);
	virtio_gpu_flush(disp->gpu, disp->resource_id, disp->xres, disp->yres);
}

static phyaddr_t virtio_gpu_display_enable(void* private, int xres, int yres) {
	struct VirtioGPUDisplay* disp = private;
	disp->resource_id = virtio_gpu_alloc_resource_id(disp->gpu);
	disp->framebuffer = V2P(pgalloc(4096));
	virtio_gpu_res_create_2d(disp->gpu, disp->resource_id, VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM, xres,
							 yres);
	virtio_gpu_attach_banking(disp->gpu, disp->resource_id, disp->framebuffer, 16 * 1024 * 1024);
	virtio_gpu_set_scanout(disp->gpu, disp->scanout, disp->resource_id, xres, yres);
	disp->xres = xres;
	disp->yres = yres;
	return disp->framebuffer;
}

static void virtio_gpu_display_disable(void* private) {
	struct VirtioGPUDisplay* disp = private;
	if (disp->framebuffer) {
		pgfree(P2V(disp->framebuffer), 4096);
	}
}

static unsigned int virtio_gpu_display_read_edid(void* private, void* buffer, unsigned int bytes) {
	struct VirtioGPUDisplay* disp = private;
	if (!disp->gpu->features.edid) {
		return 0;
	}
	uint8_t edid[1024];
	unsigned int sz = virtio_gpu_get_edid(disp->gpu, disp->scanout, edid);
	if (sz == 0) {
		return 0;
	}
	memcpy(buffer, edid, (bytes > sz) ? sz : bytes);
	return (bytes > sz) ? sz : bytes;
}

struct FramebufferDriver virtio_gpu_fb_driver;

void virtio_gpu_display_dev_init(struct VirtioGPUDevice* dev) {
	volatile struct virtio_gpu_config* gpucfg = dev->virtio_dev->devcfg;
	dev->num_display = gpucfg->num_scanouts;
	struct virtio_gpu_display_one display_info[VIRTIO_GPU_MAX_SCANOUTS];
	virtio_gpu_get_display_info(dev, display_info);
	cprintf("[virtio-gpu] heads %d\n", dev->num_display);
	for (unsigned int i = 0; i < dev->num_display; i++) {
		struct VirtioGPUDisplay* disp = kalloc();
		memset(disp, 0, sizeof(struct VirtioGPUDisplay));
		disp->gpu = dev;
		disp->enabled = display_info[i].enabled;
		if (disp->enabled) {
			disp->xres = display_info[i].r.width;
			disp->yres = display_info[i].r.height;
		}
		disp->framebuffer = 0;
		disp->scanout = i;
		hal_display_register_device("virtio-gpu", disp, &virtio_gpu_fb_driver);
	}
}

void virtio_gpu_display_global_init(void) {
	virtio_gpu_fb_driver.enable = virtio_gpu_display_enable;
	virtio_gpu_fb_driver.disable = virtio_gpu_display_disable;
	virtio_gpu_fb_driver.update = virtio_gpu_display_update;
	virtio_gpu_fb_driver.read_edid = virtio_gpu_display_read_edid;
}
