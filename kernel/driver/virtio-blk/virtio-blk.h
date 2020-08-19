#ifndef _DRIVER_VIRTIO_BLK_H
#define _DRIVER_VIRTIO_BLK_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

struct VirtioDevice {
	volatile struct VirtioPciCommonConfig* cmcfg;
	volatile unsigned int* isr;
	volatile unsigned int* notify;
	volatile struct VirtioBlockConfig* devcfg;
};

struct VirtioQueue {
	int size;
	volatile struct VirtqDesc* desc;
	volatile struct VirtqAvail* avail;
	volatile struct VirtqUsed* used;
};

struct VirtioBlockDevice {
	struct VirtioDevice virtio_dev;
	struct VirtioQueue virtio_queue;
	struct spinlock lock;
};

// virtio.c
void virtio_allocate_queue(struct VirtioQueue* queue);
void virtio_read_cap(const struct PciAddress* addr, struct VirtioDevice* dev);
void virtio_setup_queue(struct VirtioDevice* dev, struct VirtioQueue* queue,
						int feature);
int virtio_intr_ack(struct VirtioDevice* dev);
void virtio_queue_notify_wait(struct VirtioDevice* dev, struct VirtioQueue* queue);
void virtio_queue_avail_insert(struct VirtioQueue* queue, int desc);
void virtio_queue_notify(struct VirtioDevice* dev);
int* virtio_alloc_desc(struct VirtioQueue* queue, int* desc, int num);
void virtio_free_desc(struct VirtioQueue* queue, int desc);

// virtio-blk.c
void virtio_blk_init(void);
int virtio_blk_read(void* private, unsigned int begin, int count, void* buf);
int virtio_blk_write(void* private, unsigned int begin, int count, const void* buf);

#endif
