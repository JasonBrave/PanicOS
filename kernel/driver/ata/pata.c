/*
 * Parallel ATA driver
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
#include <common/spinlock.h>
#include <common/types.h>
#include <common/x86.h>
#include <defs.h>

#include "ata.h"

enum PATAIORegisters {
	PATA_IO_DATA = 0,
	PATA_IO_ERROR = 1,
	PATA_IO_FEATURES = 1,
	PATA_IO_COUNT = 2,
	PATA_IO_LBALO = 3,
	PATA_IO_LBAMID = 4,
	PATA_IO_LBAHI = 5,
	PATA_IO_DRIVE = 6,
	PATA_IO_STATUS = 7,
	PATA_IO_COMMAND = 7,
};

#define PATA_DRIVE_DEFAULT 0xe0
#define PATA_DRIVE_DRV_BIT 4

enum PATAStatus {
	PATA_STATUS_ERR = 1,
	PATA_STATUS_IDX = 2,
	PATA_STATUS_CORR = 4,
	PATA_STATUS_DRQ = 8,
	PATA_STATUS_SRV = 16,
	PATA_STATUS_DF = 32,
	PATA_STATUS_RDY = 64,
	PATA_STATUS_BSY = 128,
};

enum PATADeviceSignature {
	PATA_SIGNATURE_DISK = 0x01010000,
	PATA_SIGNATURE_PACKET = 0x010114eb,
};

uint32_t pata_read_signature(const struct PATAAdapter* adapter, int channel, int drive) {
	outb(adapter->cmdblock_base[channel] + PATA_IO_DRIVE,
		 PATA_DRIVE_DEFAULT | (drive << PATA_DRIVE_DRV_BIT));

	udelay(5);

	return (inb(adapter->cmdblock_base[channel] + PATA_IO_COUNT) << 24)
		   | (inb(adapter->cmdblock_base[channel] + PATA_IO_LBALO) << 16)
		   | (inb(adapter->cmdblock_base[channel] + PATA_IO_LBAMID) << 8)
		   | inb(adapter->cmdblock_base[channel] + PATA_IO_LBAHI);
}

void pata_bus_reset(const struct PATAAdapter* adapter, int channel) {
	outb(adapter->control_base[channel], 1 << 2);
	udelay(5);
	outb(adapter->control_base[channel], 0);
	// wait for reset to finish
	mdelay(2);
}

int pata_exec_pio_in(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	outb(iobase + PATA_IO_DRIVE,
		 PATA_DRIVE_DEFAULT | (drive << PATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + PATA_IO_COUNT, count);
	outb(iobase + PATA_IO_LBALO, lba & 0xff);
	outb(iobase + PATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + PATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + PATA_IO_COMMAND, cmd);
	// check status
	for (int i = 0; i < blocks; i++) {
		while (inb(iobase + PATA_IO_STATUS) & PATA_STATUS_BSY) {
		}
		uint8_t status = inb(iobase + PATA_IO_STATUS);
		if (status == 0 || status & PATA_STATUS_ERR) {
			release(&adapter->lock[channel]);
			return -1;
		}
		insw(iobase + PATA_IO_DATA, buf + 512 * i, 512 / 2);
	}
	release(&adapter->lock[channel]);
	return 0;
}

int pata_exec_dma_in(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					 unsigned int lba, unsigned int count, void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	pata_adapter_bmdma_prepare(adapter, channel, V2P(buf), blocks * 512);
	outb(iobase + PATA_IO_DRIVE,
		 PATA_DRIVE_DEFAULT | (drive << PATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + PATA_IO_COUNT, count);
	outb(iobase + PATA_IO_LBALO, lba & 0xff);
	outb(iobase + PATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + PATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + PATA_IO_COMMAND, cmd);
	pata_adapter_bmdma_start_write(adapter, channel);
	// check status
	while ((inb(iobase + PATA_IO_STATUS) & PATA_STATUS_BSY)
		   || pata_adapter_bmdma_busy(adapter, channel)) {
	}
	pata_adapter_bmdma_stop(adapter, channel);
	uint8_t status = inb(iobase + PATA_IO_STATUS);
	if (status == 0 || status & PATA_STATUS_ERR) {
		release(&adapter->lock[channel]);
		return -1;
	}
	release(&adapter->lock[channel]);
	return 0;
}

int pata_exec_pio_out(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					  unsigned int lba, unsigned int count, const void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	outb(iobase + PATA_IO_DRIVE,
		 PATA_DRIVE_DEFAULT | (drive << PATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + PATA_IO_COUNT, count);
	outb(iobase + PATA_IO_LBALO, lba & 0xff);
	outb(iobase + PATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + PATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + PATA_IO_COMMAND, cmd);
	// check status
	for (int i = 0; i < blocks; i++) {
		while (inb(iobase + PATA_IO_STATUS) & PATA_STATUS_BSY) {
		}
		uint8_t status = inb(iobase + PATA_IO_STATUS);
		if (status == 0 || status & PATA_STATUS_ERR) {
			release(&adapter->lock[channel]);
			return -1;
		}
		outsw(iobase + PATA_IO_DATA, buf + 512 * i, 512 / 2);
	}
	while (inb(iobase + PATA_IO_STATUS) & PATA_STATUS_BSY) {
	}
	release(&adapter->lock[channel]);
	return 0;
}

int pata_exec_dma_out(struct PATAAdapter* adapter, int channel, int drive, uint8_t cmd,
					  unsigned int lba, unsigned int count, const void* buf, int blocks) {
	ioport_t iobase = adapter->cmdblock_base[channel];
	acquire(&adapter->lock[channel]);
	pata_adapter_bmdma_prepare(adapter, channel, V2P(buf), blocks * 512);
	outb(iobase + PATA_IO_DRIVE,
		 PATA_DRIVE_DEFAULT | (drive << PATA_DRIVE_DRV_BIT) | ((lba >> 24) & 0xf));
	udelay(1);
	outb(iobase + PATA_IO_COUNT, count);
	outb(iobase + PATA_IO_LBALO, lba & 0xff);
	outb(iobase + PATA_IO_LBAMID, (lba >> 8) & 0xff);
	outb(iobase + PATA_IO_LBAHI, (lba >> 16) & 0xff);
	outb(iobase + PATA_IO_COMMAND, cmd);
	pata_adapter_bmdma_start_read(adapter, channel);
	// check status
	while ((inb(iobase + PATA_IO_STATUS) & PATA_STATUS_BSY)
		   || pata_adapter_bmdma_busy(adapter, channel)) {
	}
	pata_adapter_bmdma_stop(adapter, channel);
	uint8_t status = inb(iobase + PATA_IO_STATUS);
	if (status == 0 || status & PATA_STATUS_ERR) {
		release(&adapter->lock[channel]);
		return -1;
	}
	release(&adapter->lock[channel]);
	return 0;
}

void pata_register_adapter(struct PATAAdapter* adapter) {
	for (int channel = 0; channel < 2; channel++) {
		pata_bus_reset(adapter, channel);
		uint32_t devtype[2];
		for (int drive = 0; drive < 2; drive++) {
			devtype[drive] = pata_read_signature(adapter, channel, drive);
		}
		for (int drive = 0; drive < 2; drive++) {
			if (devtype[drive] == PATA_SIGNATURE_DISK) {
				cprintf("[ata] ATA device on PATA controller channel %d drive %d\n", channel,
						drive);
				struct ATADevice* ata_dev = ata_device_alloc();
				ata_dev->transport = ATA_TRANSPORT_PARALLEL_ATA;
				ata_dev->pata.adapter = adapter;
				ata_dev->pata.channel = channel;
				ata_dev->pata.drive = drive;

				ata_register_ata_device(ata_dev);
			} else if (devtype[drive] == PATA_SIGNATURE_PACKET) {
				cprintf("[ata] ATAPI device on PATA controller channel %d drive %d\n", channel,
						drive);
			}
		}
	}
}
