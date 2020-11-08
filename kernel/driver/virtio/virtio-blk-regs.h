#ifndef _DRIVER_VIRTIO_BLK_REGS_H
#define _DRIVER_VIRTIO_BLK_REGS_H

#include <common/types.h>

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
};

// VirtIO Block device feature bits
enum VirtioBlockFeatureBits {
	VIRTIO_BLK_F_SIZE_MAX = (1 << 1),
	VIRTIO_BLK_F_SEG_MAX = (1 << 2),
	VIRTIO_BLK_F_GEOMETRY = (1 << 4),
	VIRTIO_BLK_F_RO = (1 << 5),
	VIRTIO_BLK_F_BLK_SIZE = (1 << 6),
	VIRTIO_BLK_F_FLUSH = (1 << 9),
	VIRTIO_BLK_F_TOPOLOGY = (1 << 10),
	VIRTIO_BLK_F_CONFIG_WCE = (1 << 11),
	VIRTIO_BLK_F_DISCARD = (1 << 13),
	VIRTIO_BLK_F_WRITE_ZEROES = (1 << 14),
};

enum VirtioBlockRequestType {
	VIRTIO_BLK_T_IN = 0,
	VIRTIO_BLK_T_OUT = 1,
	VIRTIO_BLK_T_FLUSH = 4,
	VIRTIO_BLK_T_DISCARD = 11,
	VIRTIO_BLK_T_WRITE_ZEROES = 13,
};

enum VirtioBlockRequestStatus {
	VIRTIO_BLK_S_OK = 0,
	VIRTIO_BLK_S_IOERR = 1,
	VIRTIO_BLK_S_UNSUPP = 2,
};

#endif
