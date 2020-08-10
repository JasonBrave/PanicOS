#ifndef _DRIVER_PCI_PCI_H
#define _DRIVER_PCI_PCI_H

#include <common/types.h>

#define PCI_DEVICE_MAX 32
#define PCI_FUNCTION_MAX 8

struct PciAddress {
	int bus, device, function;
};

struct PciIrqInfo {
	struct PciAddress addr;
	void (*handler)(const struct PciAddress* addr);
};

#define PCI_IRQ_MAX 64

extern struct PciHost {
	uint8_t (*read8)(const struct PciAddress* addr, int reg);
	uint16_t (*read16)(const struct PciAddress* addr, int reg);
	uint32_t (*read32)(const struct PciAddress* addr, int reg);
	void (*write8)(const struct PciAddress* addr, int reg, uint8_t data);
	void (*write16)(const struct PciAddress* addr, int reg, uint16_t data);
	void (*write32)(const struct PciAddress* addr, int reg, uint32_t data);
	int bus_num; // number of addressable busses
	void* pcie_ecam_base;
} pci_host;

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

// intx.c
extern struct PciIrqInfo pci_irq_10[PCI_IRQ_MAX], pci_irq_11[PCI_IRQ_MAX];
void pci_add_irq(int irq, const struct PciAddress* addr);
void pci_interrupt(int irq);
void pci_register_intr_handler(const struct PciAddress* addr,
							   void (*handler)(const struct PciAddress*));

// intel-pcie-mmcfg.c
void intel_pcie_mmcfg_init(const struct PciAddress* host_bridge_addr);

// helper functions
static inline int pci_read_capid(const struct PciAddress* addr, int capptr) {
	return pci_read_config_reg8(addr, capptr);
}

static inline int pci_read_next_cap(const struct PciAddress* addr, int capptr) {
	return pci_read_config_reg8(addr, capptr + 1);
}

static inline void pci_read_cap(const struct PciAddress* addr, int capptr, void* buf,
								int size) {
	uint8_t* p = buf;
	for (int i = 0; i < size; i++) {
		p[i] = pci_read_config_reg8(addr, capptr + i);
	}
}

#endif
