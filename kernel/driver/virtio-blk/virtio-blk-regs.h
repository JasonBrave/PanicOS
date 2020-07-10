#ifndef _DRIVER_VIRTIO_BLK_REGS_H
#define _DRIVER_VIRTIO_BLK_REGS_H

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
} PACKED;

struct VirtqAvail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[VIRTIO_QUEUE_SIZE_MAX];
	uint16_t used_event; /* Only if VIRTIO_F_EVENT_IDX */
} PACKED;

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
} PACKED;

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
} PACKED;

struct VirtioBlockConfig {
	uint64_t capacity;
	uint32_t size_max;
	uint32_t seg_max;
	struct virtio_blk_geometry {
		uint16_t cylinders;
		uint8_t heads;
		uint8_t sectors;
	} geometry;
	uint32_t blk_size;
	struct virtio_blk_topology {
		// # of logical blocks per physical block (log2)
		uint8_t physical_block_exp;
		// offset of first aligned logical block
		uint8_t alignment_offset;
		// suggested minimum I/O size in blocks
		uint16_t min_io_size;
		// optimal (suggested maximum) I/O size in blocks
		uint32_t opt_io_size;
	} topology;
	uint8_t writeback;
	uint8_t unused0[3];
	uint32_t max_discard_sectors;
	uint32_t max_discard_seg;
	uint32_t discard_sector_alignment;
	uint32_t max_write_zeroes_sectors;
	uint32_t max_write_zeroes_seg;
	uint8_t write_zeroes_may_unmap;
	uint8_t unused1[3];
} PACKED;

struct VirtioPciCap {
	uint8_t cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */
	uint8_t cap_next; /* Generic PCI field: next ptr. */
	uint8_t cap_len; /* Generic PCI field: capability length */
	uint8_t cfg_type; /* Identifies the structure. */
	uint8_t bar; /* Where to find it. */
	uint8_t padding[3]; /* Pad to full dword. */
	uint32_t offset; /* Offset within bar. */
	uint32_t length; /* Length of the structure, in bytes. */
} PACKED;

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

#endif
