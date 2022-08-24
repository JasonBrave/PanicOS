#ifndef _FAT32_STRUCT_H
#define _FAT32_STRUCT_H

#include <common/types.h>

struct FAT32BootSector {
	uint8_t jmp[3];
	uint8_t oemname[8];
	uint16_t bytes_per_sector;
	uint8_t sector_per_cluster;
	uint16_t reserved_sector;
	uint8_t fat_number;
	uint16_t root_entry;
	uint16_t total_sector_16;
	uint8_t media;
	uint16_t dat_size_16;
	uint16_t sector_per_track;
	uint16_t head_number;
	uint32_t hidden_sector;
	uint32_t total_sector;
	// FAT32 specific
	uint32_t fat_size;
	uint16_t ext_falgs;
	uint16_t fs_ver;
	uint32_t root_cluster;
	uint16_t fsinfo;
	uint16_t backup_bootsect;
	uint8_t reserved[12];
	uint8_t drvnum;
	uint8_t reserved1;
	uint8_t bootsig;
	uint32_t volume_id;
	char volume_label[11];
	char fstype[8];
} PACKED;

struct FAT32DirEntry {
	uint8_t name[11];
	uint8_t attr;
	uint8_t nt_reserved;
	uint8_t create_time_millis;
	struct {
		uint16_t second : 5;
		uint16_t minute : 6;
		uint16_t hour : 5;
	} create_time;
	struct {
		uint16_t day : 5;
		uint16_t month : 4;
		uint16_t year : 7;
	} create_date;
	struct {
		uint16_t day : 5;
		uint16_t month : 4;
		uint16_t year : 7;
	} access_date;
	uint16_t cluster_hi;
	struct {
		uint16_t second : 5;
		uint16_t minute : 6;
		uint16_t hour : 5;
	} write_time;
	struct {
		uint16_t day : 5;
		uint16_t month : 4;
		uint16_t year : 7;
	} write_date;
	uint16_t cluster_lo;
	uint32_t size;
} PACKED;

enum FAT32DirAttr {
	ATTR_READ_ONLY = 0x01,
	ATTR_HIDDEN = 0x02,
	ATTR_SYSTEM = 0x04,
	ATTR_VOLUME_ID = 0x08,
	ATTR_DIRECTORY = 0x10,
	ATTR_ARCHIVE = 0x20,
	ATTR_LONG_NAME = 0x0f,
};

#endif
