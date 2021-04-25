#ifndef _DRIVER_VIRTIO_BLK_H
#define _DRIVER_VIRTIO_BLK_H

#include <common/spinlock.h>

#include "virtio.h"

struct VirtioBlockDevice {
	struct VirtioDevice* virtio_dev;
	struct VirtioQueue requestq;
	struct spinlock lock;
};

// virtio-blk.c
void virtio_blk_init(void);
int virtio_blk_read(void* private, unsigned int begin, int count, void* buf);
int virtio_blk_write(void* private, unsigned int begin, int count, const void* buf);

#endif
