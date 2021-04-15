#ifndef _DRIVER_VIRTIO_REGS_H
#define _DRIVER_VIRTIO_REGS_H

#include <common/types.h>

#define VIRTIO_QUEUE_SIZE_MAX 256

struct VirtqDesc {
	/* Address (guest-physical). */
	uint64_t addr;
	/* Length. */
	uint32_t len;

/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT 1
/* This marks a buffer as device write-only (otherwise device read-only). */
#define VIRTQ_DESC_F_WRITE 2
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT 4
	/* The flags as indicated above. */
	uint16_t flags;
	/* Next field if flags & NEXT */
	uint16_t next;
};

struct VirtqAvail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[VIRTIO_QUEUE_SIZE_MAX];
	uint16_t used_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct VirtqUsed {
#define VIRTQ_USED_F_NO_NOTIFY 1
	uint16_t flags;
	uint16_t idx;
	struct virtq_used_elem {
		/* Index of start of used descriptor chain. */
		uint32_t id;
		/* Total length of the descriptor chain which was used (written to) */
		uint32_t len;
	} ring[VIRTIO_QUEUE_SIZE_MAX];
	uint16_t avail_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct VirtioPciCommonConfig {
	/* About the whole device. */
	uint32_t device_feature_select; /* read-write */
	uint32_t device_feature; /* read-only for driver */
	uint32_t driver_feature_select; /* read-write */
	uint32_t driver_feature; /* read-write */
	uint16_t msix_config; /* read-write */
	uint16_t num_queues; /* read-only for driver */
	uint8_t device_status; /* read-write */
	uint8_t config_generation; /* read-only for driver */

	/* About a specific virtqueue. */
	uint16_t queue_select; /* read-write */
	uint16_t queue_size; /* read-write */
	uint16_t queue_msix_vector; /* read-write */
	uint16_t queue_enable; /* read-write */
	uint16_t queue_notify_off; /* read-only for driver */
	uint64_t queue_desc; /* read-write */
	uint64_t queue_driver; /* read-write */
	uint64_t queue_device; /* read-write */
};

/* Common configuration */
#define VIRTIO_PCI_CAP_COMMON_CFG 1
/* Notifications */
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
/* ISR Status */
#define VIRTIO_PCI_CAP_ISR_CFG 3
/* Device specific configuration */
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
/* PCI configuration access */
#define VIRTIO_PCI_CAP_PCI_CFG 5

#define VIRTIO_PCI_CAP_OFF_CAP_VENDOR 0
#define VIRTIO_PCI_CAP_OFF_CAP_NEXT 1
#define VIRTIO_PCI_CAP_OFF_CAP_LEN 2
#define VIRTIO_PCI_CAP_OFF_TYPE 3
#define VIRTIO_PCI_CAP_OFF_BAR 4
#define VIRTIO_PCI_CAP_OFF_ID 5
#define VIRTIO_PCI_CAP_OFF_OFFSET 8
#define VIRTIO_PCI_CAP_OFF_LENGTH 12

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_FAILED 128
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 64

/* Vector value used to disable MSI for queue */
#define VIRTIO_MSI_NO_VECTOR 0xffff

#endif
