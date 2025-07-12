#ifndef DRIVER_ACPI_ACPI_TABLE_H
#define DRIVER_ACPI_ACPI_TABLE_H

#include "common/types.h"
#include <defs.h>

// Root System Descriptor Pointer (RSDP), ACPI Spec 5.2.5.3
struct acpi_rsdp_t {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress; // deprecated since version 2.0

	uint32_t Length;
	uint64_t XsdtAddress;
	uint8_t ExtendedChecksum;
	uint8_t reserved[3];
} PACKED;

// System Descriptor Table Header, ACPI Spec 5.2.6
struct acpi_descriptor_table_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	char creater_id[4];
	uint32_t creater_revision;
} PACKED;

// Root System Description Table (RSDT), ACPI Spec 5.2.7
struct acpi_rsdt_t {
	struct acpi_descriptor_table_header header;
	uint32_t entry[64];
} PACKED;

#endif
