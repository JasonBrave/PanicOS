/*
 * GUID Partition Table support
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

#include <common/types.h>
#include <defs.h>

#include "hal.h"

typedef struct GUID {
	uint64_t upper, lower;
} PACKED GUID;

struct GPTHeader {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_crc32;
	uint32_t reserved;
	uint64_t current_lba;
	uint64_t backup_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	GUID disk_guid;
	uint64_t partition_entry_starting_lba;
	uint32_t num_partition_entry;
	uint32_t size_of_partition_entry;
	uint32_t partition_entry_array_crc32;
} PACKED;

#define GPT_HEADER_SIGNATURE 0x5452415020494645

struct GPTPartitionEntry {
	GUID partition_type_guid;
	GUID unique_partition_guid;
	uint64_t first_lba;
	uint64_t last_lba;
	uint64_t attribute_flags;
	uint16_t partition_name[36];
} PACKED;

#define GPT_PARTITION_ENTRY_ATTRIBUTES_REQUIRED_PARTITION (1 << 0)
#define GPT_PARTITION_ENTRY_ATTRIBUTES_NO_BLOCK_IO_PROTOCOL (1 << 1)
#define GPT_PARTITION_ENTRY_ATTRIBUTES_LEGACY_BIOS_BOOTABLE (1 << 2)

void gpt_probe_partition(int block_id) {
	struct GPTHeader* gpt_header = kalloc();
	if (hal_disk_read(block_id, 1, 1, gpt_header) < 0) {
		panic("disk read error");
	}
	if (gpt_header->current_lba != 1) {
		panic("invalid GPT");
	}
	cprintf("[hal] GPT block %d revision %x num_parts %d\n", block_id, gpt_header->revision,
			gpt_header->num_partition_entry);
	kfree(gpt_header);
}
