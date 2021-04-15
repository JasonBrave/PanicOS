#ifndef _DRIVER_PCI_PCI_H
#define _DRIVER_PCI_PCI_H

#include <common/types.h>

#define PCI_DEVICE_MAX 32
#define PCI_FUNCTION_MAX 8

struct PciAddress {
	int bus, device, function;
};

extern struct PciHost {
	uint8_t (*read8)(const struct PciAddress* addr, int reg);
	uint16_t (*read16)(const struct PciAddress* addr, int reg);
	uint32_t (*read32)(const struct PciAddress* addr, int reg);
	void (*write8)(const struct PciAddress* addr, int reg, uint8_t data);
	void (*write16)(const struct PciAddress* addr, int reg, uint16_t data);
	void (*write32)(const struct PciAddress* addr, int reg, uint32_t data);
	int bus_num; // number of addressable busses
	volatile void* pcie_ecam_base;
} pci_host;

#define PCI_DEVICE_TABLE_SIZE 64

struct PCIDriver;

extern struct PCIDevice {
	struct PciAddress addr;
	uint16_t vendor_id, device_id;
	uint8_t class, subclass, progif;
	uint8_t irq;
	void* private;
	void (*intx_intr_handler)(struct PCIDevice*);
	const struct PCIDriver* driver;
	// MSI-X specific
	volatile uint32_t* msix_table;
	volatile uint32_t* msix_pba;
} pci_device_table[PCI_DEVICE_TABLE_SIZE];

struct PCIDeviceID {
	uint16_t vendor_id, device_id;
};

struct PCIDriver {
	const char* name;
	const struct PCIDeviceID* match_table;
	unsigned int class_type;
	void (*init)(struct PCIDevice*);
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
size_t pci_read_bar_size(const struct PciAddress* addr, int bar);
phyaddr_t pci_read_rom_bar(const struct PciAddress* addr);
size_t pci_read_rom_bar_size(const struct PciAddress* addr);
void pci_enable_bus_mastering(const struct PciAddress* addr);
int pci_find_capability(const struct PciAddress* addr, uint8_t cap_id);
struct PciAddress* pci_find_device(struct PciAddress* pciaddr, uint16_t vendor, uint16_t device);
struct PciAddress* pci_find_class(struct PciAddress* pciaddr, uint8_t class, uint8_t subclass);
struct PciAddress* pci_find_progif(struct PciAddress* pciaddr, uint8_t class, uint8_t subclass,
								   uint8_t progif);
struct PciAddress* pci_next_device(struct PciAddress* pciaddr, uint16_t vendor, uint16_t device);
struct PciAddress* pci_next_class(struct PciAddress* pciaddr, uint8_t class, uint8_t subclass);
struct PciAddress* pci_next_progif(struct PciAddress* pciaddr, uint8_t class, uint8_t subclass,
								   uint8_t progif);
void pci_enable_device(const struct PciAddress* addr);

// intx.c
void pci_interrupt(int irq);
void pci_register_intr_handler(struct PCIDevice* dev, void (*handler)(struct PCIDevice*));
void pci_enable_intx_intr(const struct PciAddress* addr);
void pci_disable_intx_intr(const struct PciAddress* addr);

// msi.c
struct MSIMessage;

int pci_msi_enable(const struct PciAddress* addr, const struct MSIMessage* msg);
void pci_msi_disable(const struct PciAddress* addr);

// msix.c
int pci_msix_enable(struct PCIDevice* pci_dev);
unsigned int pci_msix_get_num_vectors(struct PCIDevice* pci_dev);
void pci_msix_set_message(struct PCIDevice* pci_dev, unsigned int vec,
						  const struct MSIMessage* msg);
void pci_msix_mask(struct PCIDevice* pci_dev, unsigned int vec);
void pci_msix_unmask(struct PCIDevice* pci_dev, unsigned int vec);

// driver.c
void pci_add_device(const struct PciAddress* addr, uint16_t vendor_id, uint16_t device_id,
					uint8_t class, uint8_t subclass, uint8_t progif, uint8_t irq);
void pci_register_driver(const struct PCIDriver* driver);
void pci_print_devices(void);

// pci-kcall.c
int pci_kcall_handler(unsigned int);

#endif
