#ifndef _DRIVER_PCI_PCI_H
#define _DRIVER_PCI_PCI_H

#include <common/types.h>

#define PCI_BUS_MAX 256
#define PCI_DEVICE_MAX 32
#define PCI_FUNCTION_MAX 8

struct PciAddress {
	int bus, device, function;
};

// pci.c
void pci_init(void);
uint8_t pci_read_config_reg8(const struct PciAddress* addr, int reg);
uint16_t pci_read_config_reg16(const struct PciAddress* addr, int reg);
uint32_t pci_read_config_reg32(const struct PciAddress* addr, int reg);
void pci_write_config_reg8(const struct PciAddress* addr, int reg, uint8_t data);
void pci_write_config_reg16(const struct PciAddress* addr, int reg, uint16_t data);
void pci_write_config_reg32(const struct PciAddress* addr, int reg, uint32_t data);
phyaddr_t pci_read_bar(const struct PciAddress* addr, int bar);
struct PciAddress* pci_find_device(struct PciAddress* pciaddr, uint16_t vendor,
								   uint16_t device);
struct PciAddress* pci_find_class(struct PciAddress* pciaddr, uint8_t class,
								  uint8_t subclass);
struct PciAddress* pci_find_progif(struct PciAddress* pciaddr, uint8_t class,
								   uint8_t subclass, uint8_t progif);
struct PciAddress* pci_next_device(struct PciAddress* pciaddr, uint16_t vendor,
								   uint16_t device);
struct PciAddress* pci_next_class(struct PciAddress* pciaddr, uint8_t class,
								  uint8_t subclass);
struct PciAddress* pci_next_progif(struct PciAddress* pciaddr, uint8_t class,
								   uint8_t subclass, uint8_t progif);

#endif
