#ifndef _VIRTIO_GPU_H
#define _VIRTIO_GPU_H

#include <kernel.h>
#include <virtio.h>

#include "virtio-gpu-regs.h"

struct VirtioGPUDevice {
	struct VirtioDevice* virtio_dev;
	struct VirtioQueue controlq;
	struct VirtioQueue cursorq;
	struct spinlock lock;
	unsigned int num_display;
	struct {
		unsigned int edid : 1;
		unsigned int virgl : 1;
	} features;
	unsigned int res_id;
};

struct VirtioGPUDisplay {
	struct VirtioGPUDevice* gpu;
	int xres, yres;
	phyaddr_t framebuffer;
	unsigned int enabled;
	unsigned int resource_id, scanout;
};

// cmds.c
int virtio_gpu_get_display_info(struct VirtioGPUDevice* dev,
								struct virtio_gpu_display_one* display_info);
unsigned int virtio_gpu_get_edid(struct VirtioGPUDevice* dev, unsigned int scanout, void* edid);
void virtio_gpu_res_create_2d(struct VirtioGPUDevice* dev, unsigned int resource_id,
							  enum virtio_gpu_formats format, unsigned int w, unsigned int h);
void virtio_gpu_set_scanout(struct VirtioGPUDevice* dev, unsigned int scanout,
							unsigned int resource_id, unsigned int w, unsigned int h);
void virtio_gpu_flush(struct VirtioGPUDevice* dev, unsigned int resource_id, unsigned int w,
					  unsigned int h);
void virtio_gpu_xfer_to_host_2d(struct VirtioGPUDevice* dev, unsigned int resource_id,
								unsigned int w, unsigned int h);
void virtio_gpu_attach_banking(struct VirtioGPUDevice* dev, unsigned int resource_id, phyaddr_t fb,
							   size_t length);

// display.c
void virtio_gpu_display_dev_init(struct VirtioGPUDevice* dev);
void virtio_gpu_display_global_init(void);

// virtio-gpu.c
unsigned int virtio_gpu_alloc_resource_id(struct VirtioGPUDevice* dev);

#endif
