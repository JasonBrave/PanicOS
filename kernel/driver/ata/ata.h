#ifndef _DRIVER_ATA_ATA_H
#define _DRIVER_ATA_ATA_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

struct ATABMDMAPRD;

struct ATAAdapter {
	ioport_t cmdblock_base[2];
	ioport_t control_base[2];
	ioport_t bus_master_base;
	struct {
		unsigned short bus_master : 1;
		unsigned short pci_native : 1;
	};
	struct ATABMDMAPRD* prd[2];
	struct spinlock lock[2];
};

struct ATADevice {
	struct ATAAdapter* adapter;
	struct {
		unsigned char channel : 1; // primary/secondary
		unsigned char drive : 1; // master/slave
		unsigned char use_dma : 1; // use DMA
	};
	unsigned int sectors; // number of sectors
	char dma, pio, mdma, udma, ata_rev;
};

// ata.c
void ata_init(void);
void ata_register_adapter(struct ATAAdapter* adapter);
uint32_t ata_read_signature(const struct ATAAdapter* adapter, int channel, int drive);
int ata_identify(struct ATAAdapter* adapter, int channel, int drive,
				 struct ATADevice* dev, char* model);
void ata_bus_reset(const struct ATAAdapter* adapter, int channel);
int ata_exec_pio_in(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					unsigned int lba, unsigned int count, void* buf, int blocks);
int ata_read(void* private, unsigned int begin, int count, void* buf);
int ata_exec_pio_out(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, const void* buf, int blocks);
int ata_write(void* private, unsigned int begin, int count, const void* buf);

// adapter.c
void ata_adapter_dev_init(struct PCIDevice* dev);
void ata_adapter_init(void);
void ata_legacy_intr(int irq);
void ata_pci_intr(struct PCIDevice* dev);
void ata_adapter_bmdma_init(struct ATAAdapter* dev, int channel, int drive);
void ata_adapter_bmdma_prepare(struct ATAAdapter* dev, int channel, phyaddr_t addr,
							   unsigned int size);
void ata_adapter_bmdma_start_write(struct ATAAdapter* dev, int channel);
void ata_adapter_bmdma_start_read(struct ATAAdapter* dev, int channel);
int ata_adapter_bmdma_busy(struct ATAAdapter* dev, int channel);
void ata_adapter_bmdma_stop(struct ATAAdapter* dev, int channel);

#endif
