#ifndef _DRIVER_VIRTIO_BLK_H
#define _DRIVER_VIRTIO_BLK_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

#define VIRTIO_BLK_NUM_MAX 8

struct VirtioBlockDevice {
	// MMIO registers
	volatile struct VirtioPciCommonConfig* cmcfg;
	volatile unsigned int* isr;
	volatile unsigned int* notify;
	volatile struct VirtioBlockConfig* devcfg;
	// queues
	volatile struct VirtqDesc* virtq_desc;
	volatile struct VirtqAvail* virtq_avail;
	volatile struct VirtqUsed* virtq_used;
	// lock
	struct spinlock lock;
	// PCI address of device,used by ISR
	struct PciAddress addr;
};

extern struct VirtioBlockDevice virtio_blk_dev[VIRTIO_BLK_NUM_MAX];

void virtio_blk_init(void);
int virtio_blk_read(int id, unsigned int begin, int count, void* buf);

#endif
