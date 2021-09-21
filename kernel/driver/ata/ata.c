/*
 * ATA Hard Drive driver
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/delay.h>
#include <common/errorcode.h>
#include <common/x86.h>
#include <defs.h>
#include <hal/hal.h>
#include <memlayout.h>

#include "ata.h"

enum AtaIoRegisters {
	ATA_IO_DATA = 0,
	ATA_IO_ERROR = 1,
	ATA_IO_FEATURES = 1,
	ATA_IO_COUNT = 2,
	ATA_IO_LBALO = 3,
	ATA_IO_LBAMID = 4,
	ATA_IO_LBAHI = 5,
	ATA_IO_DRIVE = 6,
	ATA_IO_STATUS = 7,
	ATA_IO_COMMAND = 7,
};

#define ATA_DRIVE_DEFAULT 0xe0
#define ATA_DRIVE_DRV_BIT 4

enum AtaStatus {
	ATA_STATUS_ERR = 1,
	ATA_STATUS_IDX = 2,
	ATA_STATUS_CORR = 4,
	ATA_STATUS_DRQ = 8,
	ATA_STATUS_SRV = 16,
	ATA_STATUS_DF = 32,
	ATA_STATUS_RDY = 64,
	ATA_STATUS_BSY = 128,
};

enum AtaCommands {
	ATA_COMMAND_READ_SECTOR = 0x20,
	ATA_COMMAND_WRITE_SECTOR = 0x30,
	ATA_COMMAND_READ_DMA = 0xc8,
	ATA_COMMAND_WRITE_DMA = 0xca,
	ATA_COMMAND_IDENTIFY = 0xec,
};

enum ATASignature {
	ATA_SIGNATURE_DISK = 0x01010000,
	ATA_SIGNATURE_PACKET = 0x010114eb,
};

static struct ATADevice* ata_device_alloc(void) {
	struct ATADevice* dev = kalloc();
	memset(dev, 0, sizeof(struct ATADevice));
	return dev;
}

void ata_init(void) {
	ata_adapter_init();
}

uint32_t ata_read_signature(const struct ATAAdapter* adapter, int channel, int drive) {
	outb(adapter->cmdblock_base[channel] + ATA_IO_DRIVE,
		 ATA_DRIVE_DEFAULT | (drive << ATA_DRIVE_DRV_BIT));

	udelay(5);

	return (inb(adapter->cmdblock_base[channel] + ATA_IO_COUNT) << 24) |
		   (inb(adapter->cmdblock_base[channel] + ATA_IO_LBALO) << 16) |
		   (inb(adapter->cmdblock_base[channel] + ATA_IO_LBAMID) << 8) |
		   inb(adapter->cmdblock_base[channel] + ATA_IO_LBAHI);
}

int ata_identify(struct ATAAdapter* adapter, int channel, int drive, struct ATADevice* dev,
				 char* model) {
	uint16_t* identify = kalloc();
	if (ata_exec_pio_in(adapter, channel, drive, ATA_COMMAND_IDENTIFY, 0, 0, identify, 1)) {
		kfree(identify);
		return -1;
	}
	for (int i = 0; i < 20; i++) {
		model[i * 2] = identify[27 + i] >> 8;
		model[i * 2 + 1] = identify[27 + i];
	}
	for (int i = 0; i < 40; i++) {
		if (model[i] == ' ' && model[i + 1] == ' ') {
			model[i] = '\0';
			break;
		}
	}
	model[40] = '\0';

	dev->channel = channel;
	dev->drive = drive;
	dev->sectors = identify[60] + (identify[61] << 16);

	if (identify[49] & (1 << 8)) {
		dev->dma = 1;
	}
	if (identify[64] & 3) {
		dev->pio = 4;
	} else {
		dev->pio = 2;
	}
	for (int i = 0; i <= 2; i++) {
		if (identify[63] & (1 << i)) {
			dev->mdma = i;
		}
	}
	for (int i = 1; i <= 14; i++) {
		if (identify[80] & (1 << i)) {
			dev->ata_rev = i;
		}
	}
	for (int i = 0; i <= 6; i++) {
		if (identify[88] & (1 << i)) {
			dev->udma = i;
		}
	}

	kfree(identify);
	return 0;
}

void ata_bus_reset(const struct ATAAdapter* adapter, int channel) {
	outb(adapter->control_base[channel], 1 << 2);
	udelay(5);
	outb(adapter->control_base[channel], 0);
	// wait for reset to finish
	mdelay(2);
}

int ata_exec_pio_in(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					unsigned int lba, unsigned int count, void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	outb(iobase + ATA_IO_DRIVE,
		 ATA_DRIVE_DEFAULT | (drive << ATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + ATA_IO_COUNT, count);
	outb(iobase + ATA_IO_LBALO, lba & 0xff);
	outb(iobase + ATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + ATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + ATA_IO_COMMAND, cmd);
	// check status
	for (int i = 0; i < blocks; i++) {
		while (inb(iobase + ATA_IO_STATUS) & ATA_STATUS_BSY) {
		}
		uint8_t status = inb(iobase + ATA_IO_STATUS);
		if (status == 0 || status & ATA_STATUS_ERR) {
			release(&adapter->lock[channel]);
			return -1;
		}
		insw(iobase + ATA_IO_DATA, buf + 512 * i, 512 / 2);
	}
	release(&adapter->lock[channel]);
	return 0;
}

int ata_exec_dma_in(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					unsigned int lba, unsigned int count, void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	ata_adapter_bmdma_prepare(adapter, channel, V2P(buf), blocks * 512);
	outb(iobase + ATA_IO_DRIVE,
		 ATA_DRIVE_DEFAULT | (drive << ATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + ATA_IO_COUNT, count);
	outb(iobase + ATA_IO_LBALO, lba & 0xff);
	outb(iobase + ATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + ATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + ATA_IO_COMMAND, cmd);
	ata_adapter_bmdma_start_write(adapter, channel);
	// check status
	while ((inb(iobase + ATA_IO_STATUS) & ATA_STATUS_BSY) ||
		   ata_adapter_bmdma_busy(adapter, channel)) {
	}
	ata_adapter_bmdma_stop(adapter, channel);
	uint8_t status = inb(iobase + ATA_IO_STATUS);
	if (status == 0 || status & ATA_STATUS_ERR) {
		release(&adapter->lock[channel]);
		return -1;
	}
	release(&adapter->lock[channel]);
	return 0;
}

int ata_read(void* private, unsigned int begin, int count, void* buf) {
	struct ATADevice* dev = private;
	if (dev->use_dma) {
		if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP ||
			(phyaddr_t)buf % PGSIZE)
			panic("ata dma");
		if (count == 0 || count > 8)
			panic("ata count");
		if (ata_exec_dma_in(dev->adapter, dev->channel, dev->drive, ATA_COMMAND_READ_DMA, begin,
							count, buf, count)) {
			return ERROR_READ_FAIL;
		}
	} else {
		if (ata_exec_pio_in(dev->adapter, dev->channel, dev->drive, ATA_COMMAND_READ_SECTOR, begin,
							count, buf, count)) {
			return ERROR_READ_FAIL;
		}
	}
	return 0;
}

int ata_exec_pio_out(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, const void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	outb(iobase + ATA_IO_DRIVE,
		 ATA_DRIVE_DEFAULT | (drive << ATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + ATA_IO_COUNT, count);
	outb(iobase + ATA_IO_LBALO, lba & 0xff);
	outb(iobase + ATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + ATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + ATA_IO_COMMAND, cmd);
	// check status
	for (int i = 0; i < blocks; i++) {
		while (inb(iobase + ATA_IO_STATUS) & ATA_STATUS_BSY) {
		}
		uint8_t status = inb(iobase + ATA_IO_STATUS);
		if (status == 0 || status & ATA_STATUS_ERR) {
			release(&adapter->lock[channel]);
			return -1;
		}
		outsw(iobase + ATA_IO_DATA, buf + 512 * i, 512 / 2);
	}
	while (inb(iobase + ATA_IO_STATUS) & ATA_STATUS_BSY) {
	}
	release(&adapter->lock[channel]);
	return 0;
}

int ata_exec_dma_out(struct ATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, const void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	ata_adapter_bmdma_prepare(adapter, channel, V2P(buf), blocks * 512);
	outb(iobase + ATA_IO_DRIVE,
		 ATA_DRIVE_DEFAULT | (drive << ATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + ATA_IO_COUNT, count);
	outb(iobase + ATA_IO_LBALO, lba & 0xff);
	outb(iobase + ATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + ATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + ATA_IO_COMMAND, cmd);
	ata_adapter_bmdma_start_read(adapter, channel);
	// check status
	while ((inb(iobase + ATA_IO_STATUS) & ATA_STATUS_BSY) ||
		   ata_adapter_bmdma_busy(adapter, channel)) {
	}
	ata_adapter_bmdma_stop(adapter, channel);
	uint8_t status = inb(iobase + ATA_IO_STATUS);
	if (status == 0 || status & ATA_STATUS_ERR) {
		release(&adapter->lock[channel]);
		return -1;
	}
	release(&adapter->lock[channel]);
	return 0;
}

int ata_write(void* private, unsigned int begin, int count, const void* buf) {
	struct ATADevice* dev = private;
	if (dev->use_dma) {
		if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP ||
			(phyaddr_t)buf % PGSIZE)
			panic("ata dma");
		if (count == 0 || count > 8)
			panic("ata count");
		if (ata_exec_dma_out(dev->adapter, dev->channel, dev->drive, ATA_COMMAND_WRITE_DMA, begin,
							 count, buf, count)) {
			return ERROR_READ_FAIL;
		}
	} else {
		if (ata_exec_pio_out(dev->adapter, dev->channel, dev->drive, ATA_COMMAND_WRITE_SECTOR,
							 begin, count, buf, count)) {
			return ERROR_READ_FAIL;
		}
	}
	return 0;
}

const struct BlockDeviceDriver ata_block_driver = {
	.block_read = ata_read,
	.block_write = ata_write,
};

void ata_register_adapter(struct ATAAdapter* adapter) {
	for (int channel = 0; channel < 2; channel++) {
		ata_bus_reset(adapter, channel);
		uint32_t devtype[2];
		for (int drive = 0; drive < 2; drive++) {
			devtype[drive] = ata_read_signature(adapter, channel, drive);
		}
		for (int drive = 0; drive < 2; drive++) {
			if (devtype[drive] == ATA_SIGNATURE_DISK) {
				struct ATADevice* ata_dev = ata_device_alloc();
				if (!ata_dev) {
					panic("too many ATA device");
				}
				ata_dev->adapter = adapter;
				char model[50];
				if (ata_identify(adapter, channel, drive, ata_dev, model) == 0) {
					cprintf("[ata] Disk model %s %d sectors\n", model, ata_dev->sectors);
					cprintf("[ata] dma %d pio %d mdma %d udma %d ata_rev %d\n", ata_dev->dma,
							ata_dev->pio, ata_dev->mdma, ata_dev->udma, ata_dev->ata_rev);
					if (ata_dev->dma && ata_dev->adapter->bus_master) {
						ata_dev->use_dma = 1;
						cprintf("[ata] Use DMA for channel %d drive %d\n", channel, drive);
						ata_adapter_bmdma_init(ata_dev->adapter, channel, drive);
					} else {
						ata_dev->use_dma = 0;
						cprintf("[ata] Use PIO for channel %d drive %d\n", channel, drive);
					}
					hal_block_register_device("ata", ata_dev, &ata_block_driver);
				}
			} else if (devtype[drive] == ATA_SIGNATURE_PACKET) {
				cprintf("[ata] ATAPI device found\n");
			}
		}
	}
}
