#ifndef _VIRTIO_GPU_REGS_H
#define _VIRTIO_GPU_REGS_H

#include <kernel-types.h>

#define VIRTIO_GPU_F_VIRGL (1 << 0)
#define VIRTIO_GPU_F_EDID (1 << 1)
#define VIRTIO_GPU_F_RESOURCE_UUID (1 << 2)

#define VIRTIO_GPU_EVENT_DISPLAY (1 << 0)

struct virtio_gpu_config {
	uint32_t events_read;
	uint32_t events_clear;
	uint32_t num_scanouts;
	uint32_t reserved;
};

enum virtio_gpu_ctrl_type {
	/* 2d commands */
	VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
	VIRTIO_GPU_CMD_RESOURCE_UNREF,
	VIRTIO_GPU_CMD_SET_SCANOUT,
	VIRTIO_GPU_CMD_RESOURCE_FLUSH,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
	VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
	VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
	VIRTIO_GPU_CMD_GET_CAPSET_INFO,
	VIRTIO_GPU_CMD_GET_CAPSET,
	VIRTIO_GPU_CMD_GET_EDID,
	VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID,
	/* 3d commands (OpenGL) */
	VIRTIO_GPU_CMD_CTX_CREATE = 0x0200,
	VIRTIO_GPU_CMD_CTX_DESTROY,
	VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE,
	VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_3D,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D,
	VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D,
	VIRTIO_GPU_CMD_SUBMIT_3D,
	/* cursor commands */
	VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
	VIRTIO_GPU_CMD_MOVE_CURSOR,
	/* success responses */
	VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
	VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
	VIRTIO_GPU_RESP_OK_CAPSET_INFO,
	VIRTIO_GPU_RESP_OK_CAPSET,
	VIRTIO_GPU_RESP_OK_EDID,
	VIRTIO_GPU_RESP_OK_RESOURCE_UUID,
	/* error responses */
	VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
	VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
	VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

#define VIRTIO_GPU_FLAG_FENCE (1 << 0)

struct virtio_gpu_ctrl_hdr {
	uint32_t type;
	uint32_t flags;
	uint64_t fence_id;
	uint32_t ctx_id;
	uint32_t padding;
};

#define VIRTIO_GPU_MAX_SCANOUTS 16

struct virtio_gpu_rect {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct virtio_gpu_resp_display_info {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_display_one {
		struct virtio_gpu_rect r;
		uint32_t enabled;
		uint32_t flags;
	} pmodes[VIRTIO_GPU_MAX_SCANOUTS];
};

struct virtio_gpu_get_edid {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t scanout;
	uint32_t padding;
};

struct virtio_gpu_resp_edid {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t size;
	uint32_t padding;
	uint8_t edid[1024];
};

enum virtio_gpu_formats {
	VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM = 1,
	VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM = 2,
	VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM = 3,
	VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM = 4,

	VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM = 67,
	VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM = 68,

	VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM = 121,
	VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM = 134,
};

struct virtio_gpu_resource_create_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t format;
	uint32_t width;
	uint32_t height;
};

struct virtio_gpu_resource_unref {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t padding;
};

struct virtio_gpu_set_scanout {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect r;
	uint32_t scanout_id;
	uint32_t resource_id;
};

struct virtio_gpu_resource_flush {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect r;
	uint32_t resource_id;
	uint32_t padding;
};

struct virtio_gpu_transfer_to_host_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect r;
	uint64_t offset;
	uint32_t resource_id;
	uint32_t padding;
};

struct virtio_gpu_resource_attach_backing {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t nr_entries;
};

struct virtio_gpu_mem_entry {
	uint64_t addr;
	uint32_t length;
	uint32_t padding;
};

struct virtio_gpu_resource_detach_backing {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t padding;
};

struct virtio_gpu_resource_assign_uuid {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t padding;
};
struct virtio_gpu_resp_resource_uuid {
	struct virtio_gpu_ctrl_hdr hdr;
	uint8_t uuid[16];
};

struct virtio_gpu_cursor_pos {
	uint32_t scanout_id;
	uint32_t x;
	uint32_t y;
	uint32_t padding;
};

struct virtio_gpu_update_cursor {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_cursor_pos pos;
	uint32_t resource_id;
	uint32_t hot_x;
	uint32_t hot_y;
	uint32_t padding;
};

#endif
