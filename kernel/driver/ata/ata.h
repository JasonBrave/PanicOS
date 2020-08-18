#ifndef _DRIVER_ATA_ATA_H
#define _DRIVER_ATA_ATA_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

struct ATAAdapter {
	ioport_t cmdblock_base[2];
	ioport_t control_base[2];
	ioport_t bus_master_base;
	struct {
		unsigned short bus_master : 1;
		unsigned short pci_native : 1;
	};
	struct spinlock lock[2];
};

#define ATA_ADAPTER_MAX 8

extern struct ATAAdapter ata_adapter[ATA_ADAPTER_MAX];

struct ATADevice {
	struct ATAAdapter* adapter;
	struct {
		unsigned char channel : 1; // primary/secondary
		unsigned char drive : 1; // master/slave
	};
	unsigned int sectors; // number of sectors
	char dma, pio, mdma, udma, ata_rev;
};

#define ATA_DEVICE_MAX 8

extern struct ATADevice ata_device[ATA_DEVICE_MAX];

// ata.c
void ata_init(void);
uint32_t ata_read_signature(const struct ATAAdapter* adapter, int channel, int drive);
int ata_identify(struct ATAAdapter* adapter, int channel, int drive,
				 struct ATADevice* dev, char* model);
void ata_bus_reset(const struct ATAAdapter* adapter, int channel);
int ata_exec_pio_in(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					unsigned int lba, unsigned int count, void* buf, int blocks);
int ata_read(int id, unsigned int begin, int count, void* buf);
int ata_exec_pio_out(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, const void* buf, int blocks);
int ata_write(int id, unsigned int begin, int count, const void* buf);

// adapter.c
void ata_adapter_dev_init(struct PCIDevice* dev);
void ata_adapter_init(void);
void ata_legacy_intr(int irq);
void ata_pci_intr(struct PCIDevice* dev);

#endif
