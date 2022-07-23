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

enum ATATransport {
	ATA_TRANSPORT_PARALLEL_ATA,
	ATA_TRANSPORT_SERIAL_ATA
};

struct PATADevice {
	struct PATAAdapter* adapter;
	struct {
		unsigned char channel : 1; // primary/secondary
		unsigned char drive : 1; // master/slave
		unsigned char use_dma : 1; // use DMA
	};
	char dma, pio, mdma, udma;
};

struct ATADevice {
	enum ATATransport transport;
	unsigned int sectors; // number of sectors
	unsigned int ata_rev;
	union {
		struct PATADevice pata;
	};
};

// ata.c
void ata_init(void);
int ata_identify(struct ATADevice* dev, char* model);
int ata_read(void* private, unsigned int begin, int count, void* buf);
int ata_write(void* private, unsigned int begin, int count, const void* buf);
void ata_register_ata_device(struct ATADevice* ata_dev);
void ata_register_atapi_device(struct ATADevice* ata_dev);
struct ATADevice* ata_device_alloc(void);

// pata.c
uint32_t pata_read_signature(const struct PATAAdapter* adapter, int channel, int drive);
void pata_bus_reset(const struct PATAAdapter* adapter, int channel);
int pata_exec_pio_in(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, void* buf, int blocks);
int pata_exec_dma_in(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, void* buf, int blocks);
int pata_exec_pio_out(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					  unsigned int lba, unsigned int count, const void* buf, int blocks);
int pata_exec_dma_out(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					  unsigned int lba, unsigned int count, const void* buf, int blocks);
void pata_register_adapter(struct PATAAdapter* adapter);

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
