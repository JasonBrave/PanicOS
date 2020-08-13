#ifndef _DRIVER_ATA_ATA_H
#define _DRIVER_ATA_ATA_H

#include <driver/pci/pci.h>

struct ATAAdapter {
	ioport_t cmdblock_base[2];
	ioport_t control_base[2];
	ioport_t bus_master_base;
	struct {
		unsigned short bus_master : 1;
		unsigned short pci_native : 1;
	};
};

#define ATA_ADAPTER_MAX 8

extern struct ATAAdapter ata_adapter[ATA_ADAPTER_MAX];

struct ATADevice {
	const struct ATAAdapter* adapter;
	struct {
		unsigned char channel : 1; // primary/secondary
		unsigned char drive : 1; // master/slave
	};
	unsigned int sectors; // number of sectors
};

#define ATA_DEVICE_MAX 8

extern struct ATADevice ata_device[ATA_DEVICE_MAX];

// ata.c
void ata_init(void);
int ata_identify(const struct ATAAdapter* adapter, int channel, int drive,
				 struct ATADevice* dev, char* model);
void ata_bus_reset(const struct ATAAdapter* adapter, int channel);
int ata_exec_pio_in(const struct ATAAdapter* adapter, int channel, int drive,
					uint8_t cmd, unsigned int lba, unsigned int count, void* buf);

// adapter.c
void ata_adapter_dev_init(const struct PciAddress* addr);
void ata_adapter_init(void);
void ata_legacy_intr(int irq);
void ata_pci_intr(const struct PciAddress* addr);

#endif
