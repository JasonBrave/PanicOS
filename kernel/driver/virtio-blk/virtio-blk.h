#ifndef _DRIVER_VIRTIO_BLK_H
#define _DRIVER_VIRTIO_BLK_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

#define VIRTIO_BLK_NUM_MAX 8

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
	// lock
	struct spinlock lock;
	// PCI address of device,used by ISR
	struct PciAddress addr;
};

// virtio.c
void virtio_enum_device(int device_id,
						void (*devinitfunc)(const struct PciAddress* addr));
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
extern struct VirtioBlockDevice virtio_blk_dev[VIRTIO_BLK_NUM_MAX];
void virtio_blk_init(void);
int virtio_blk_read(int id, unsigned int begin, int count, void* buf);
int virtio_blk_write(int id, unsigned int begin, int count, const void* buf);

#endif
