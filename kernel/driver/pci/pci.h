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
	void* pcie_ecam_base;
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
void pci_enable_bus_mastering(const struct PciAddress* addr);
int pci_find_capability(const struct PciAddress* addr, uint8_t cap_id);
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
void pci_interrupt(int irq);
void pci_register_intr_handler(struct PCIDevice* dev,
							   void (*handler)(struct PCIDevice*));
void pci_enable_intx_intr(const struct PciAddress* addr);
void pci_disable_intx_intr(const struct PciAddress* addr);

// intel-pcie-mmcfg.c
void intel_pcie_mmcfg_init(const struct PciAddress* host_bridge_addr);

// msi.c
#define PCI_MSI_VECTOR_MAX 190
#define PCI_MSI_VECTOR_BASE 65

extern struct PCIMSIVector {
	struct {
		unsigned char used : 1;
	};
	void (*handler)(void*);
	void* private;
} pci_msi_vector[PCI_MSI_VECTOR_MAX];

void pci_msi_intr(int vector);
int pci_msi_alloc_vector(void (*handler)(void*), void* private);
void pci_msi_free_vector(int vector);
int pci_msi_enable(const struct PciAddress* addr, int vector, int lapicid);
void pci_msi_disable(const struct PciAddress* addr);

// driver.c
void pci_add_device(const struct PciAddress* addr, uint16_t vendor_id,
					uint16_t device_id, uint8_t class, uint8_t subclass, uint8_t progif,
					uint8_t irq);
void pci_register_driver(const struct PCIDriver* driver);
void pci_print_devices(void);

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
