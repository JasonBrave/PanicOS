#ifndef _DRIVER_ATA_ATA_H
#define _DRIVER_ATA_ATA_H

#include <common/spinlock.h>
#include <driver/pci/pci.h>

struct PATABMDMAPRD;

struct PATAAdapter {
	ioport_t cmdblock_base[2];
	ioport_t control_base[2];
	ioport_t bus_master_base;
	struct {
		unsigned short bus_master : 1;
		unsigned short pci_native : 1;
	};
	struct PATABMDMAPRD* prd[2];
	struct spinlock lock[2];
};

struct ATADevice {
	struct PATAAdapter* adapter;
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
void ata_register_adapter(struct PATAAdapter* adapter);
uint32_t ata_read_signature(const struct PATAAdapter* adapter, int channel, int drive);
int ata_identify(struct PATAAdapter* adapter, int channel, int drive, struct ATADevice* dev,
				 char* model);
void ata_bus_reset(const struct PATAAdapter* adapter, int channel);
int ata_exec_pio_in(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					unsigned int lba, unsigned int count, void* buf, int blocks);
int ata_read(void* private, unsigned int begin, int count, void* buf);
int ata_exec_pio_out(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, const void* buf, int blocks);
int ata_write(void* private, unsigned int begin, int count, const void* buf);

// adapter.c
void pata_adapter_dev_init(struct PCIDevice* dev);
void pata_adapter_init(void);
void pata_adapter_legacy_intr(int irq);
void pata_adapter_pci_intr(struct PCIDevice* dev);
void pata_adapter_bmdma_init(struct PATAAdapter* dev, int channel, int drive);
void pata_adapter_bmdma_prepare(struct PATAAdapter* dev, int channel, phyaddr_t addr,
								unsigned int size);
void pata_adapter_bmdma_start_write(struct PATAAdapter* dev, int channel);
void pata_adapter_bmdma_start_read(struct PATAAdapter* dev, int channel);
int pata_adapter_bmdma_busy(struct PATAAdapter* dev, int channel);
void pata_adapter_bmdma_stop(struct PATAAdapter* dev, int channel);

#endif
