#include <defs.h>

struct acpi_xsdp_t {
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

void acpi_init(void) {
	uint8_t *search_ptr = (uint8_t *)P2V(0xe0000);

	for (; search_ptr < P2V(0xfffff); search_ptr += 16) {
		struct acpi_xsdp_t *xsdp = (struct acpi_xsdp_t *)search_ptr;
		if (memcmp(xsdp->Signature, "RSD PTR ", 8) == 0) {
			cprintf(
				"[acpi] RSDP %p Revision %d RSDPAddr %x XSDTAddr %lx\n",
				V2P(xsdp),
				xsdp->Revision,
				xsdp->RsdtAddress,
				xsdp->XsdtAddress
			);
		}
	}
}
