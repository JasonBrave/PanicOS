#include <defs.h>

static void *map_region(phyaddr_t phyaddr, size_t size) {
	cprintf("Attempt to map region %x\n", phyaddr);
	return (void *)phyaddr;
}

void *map_mmio_region(phyaddr_t phyaddr, size_t size) {
	return map_region(phyaddr, size);
}

void *map_ram_region(phyaddr_t phyaddr, size_t size) {
	return map_region(phyaddr, size);
}

void *map_rom_region(phyaddr_t phyaddr, size_t size) {
	return map_region(phyaddr, size);
}
