#include <defs.h>

#include "acpi_table.h"

void acpi_init(void) {
	struct acpi_rsdp_t *rsdp = 0;

	uint8_t *search_ptr = (uint8_t *)P2V(0xe0000);
	for (; search_ptr < P2V(0xfffff); search_ptr += 16) {
		if (memcmp(search_ptr, "RSD PTR ", 8) == 0) {
			rsdp = (struct acpi_rsdp_t *)search_ptr;
			cprintf(
				"[acpi] RSDP %x Revision %d RSDTAddr %x XSDTAddr %lx\r\n",
				V2P(rsdp),
				rsdp->Revision,
				rsdp->RsdtAddress,
				rsdp->XsdtAddress
			);
			break;
		}
	}

	if (!rsdp) {
		return;
	}

	struct acpi_rsdt_t *rsdt = P2V(rsdp->RsdtAddress);
	unsigned int rsdt_entries =
		(rsdt->header.length - sizeof(struct acpi_descriptor_table_header)) / 4;
	cprintf(
		"[acpi] Table RSDT %x Length %u Entries %u\r\n",
		V2P(rsdt),
		rsdt->header.length,
		rsdt_entries
	);

	for (unsigned int i = 0; i < rsdt_entries; i++) {
		struct acpi_descriptor_table_header *header = P2V(rsdt->entry[i]);
		cprintf(
			"[acpi] Table %c%c%c%c %x Length %u\r\n",
			header->signature[0],
			header->signature[1],
			header->signature[2],
			header->signature[3],
			V2P(header),
			header->length
		);
	}
}
