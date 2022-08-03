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
#include <common/types.h>
#include <common/x86.h>
#include <defs.h>
#include <hal/hal.h>
#include <memlayout.h>

#include "ata.h"

enum ATACommands {
	ATA_COMMAND_READ_SECTOR = 0x20,
	ATA_COMMAND_WRITE_SECTOR = 0x30,
	ATA_COMMAND_READ_DMA = 0xc8,
	ATA_COMMAND_WRITE_DMA = 0xca,
	ATA_COMMAND_IDENTIFY = 0xec,
};

struct ATADevice* ata_device_alloc(void) {
	struct ATADevice* dev = kalloc();
	memset(dev, 0, sizeof(struct ATADevice));
	return dev;
}

void ata_init(void) {
	pata_adapter_init();
}

int ata_identify(struct ATADevice* dev, char* model) {
	uint16_t* identify = kalloc();

	if (dev->transport == ATA_TRANSPORT_PARALLEL_ATA) {
		if (pata_exec_pio_in(dev->pata.adapter, dev->pata.channel, dev->pata.drive,
							 ATA_COMMAND_IDENTIFY, 0, 0, identify, 1)) {
			kfree(identify);
			return -1;
		}
	} else if (dev->transport == ATA_TRANSPORT_SERIAL_ATA) {
		if (sata_exec_pio_in(&dev->sata, ATA_COMMAND_IDENTIFY, 0, 0, identify, 1)) {
			kfree(identify);
			return -1;
		}
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

	dev->sectors = identify[60] + (identify[61] << 16);
	for (int i = 1; i <= 14; i++) {
		if (identify[80] & (1 << i)) {
			dev->ata_rev = i;
		}
	}

	if (identify[83] & (1 << 10)) {
		dev->support_lba48 = 1;
	}

	if (dev->transport == ATA_TRANSPORT_PARALLEL_ATA) {
		if (identify[49] & (1 << 8)) {
			dev->pata.dma = 1;
		}
		if (identify[64] & 3) {
			dev->pata.pio = 4;
		} else {
			dev->pata.pio = 2;
		}
		for (int i = 0; i <= 2; i++) {
			if (identify[63] & (1 << i)) {
				dev->pata.mdma = i;
			}
		}
		for (int i = 0; i <= 6; i++) {
			if (identify[88] & (1 << i)) {
				dev->pata.udma = i;
			}
		}
	} else if (dev->transport == ATA_TRANSPORT_SERIAL_ATA) {
		dev->sata.ncq_queue_depth = (identify[75] & 0x1f) + 1;
		if (identify[76] & (1 << 8)) {
			dev->sata.support_ncq = 1;
		}
		if (identify[76] & (1 << 12)) {
			dev->sata.support_ncq_priority_info = 1;
		}
		if (identify[77] & (1 << 4)) {
			dev->sata.support_ncq_streaming = 1;
		}
		if (identify[77] & (1 << 5)) {
			dev->sata.support_ncq_queue_mgmt_cmd = 1;
		}
		if (identify[77] & (1 << 6)) {
			dev->sata.support_receive_send_fpdma_queued = 1;
		}
	}

	kfree(identify);
	return 0;
}

int ata_read(void* private, unsigned int begin, int count, void* buf) {
	struct ATADevice* dev = private;
	if (dev->transport == ATA_TRANSPORT_PARALLEL_ATA) {
		if (dev->pata.use_dma) {
			if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP
				|| (phyaddr_t)buf % PGSIZE)
				panic("ata dma");
			if (count == 0 || count > 8)
				panic("ata count");
			if (pata_exec_dma_in(dev->pata.adapter, dev->pata.channel, dev->pata.drive,
								 ATA_COMMAND_READ_DMA, begin, count, buf, count)) {
				return ERROR_READ_FAIL;
			}
		} else {
			if (pata_exec_pio_in(dev->pata.adapter, dev->pata.channel, dev->pata.drive,
								 ATA_COMMAND_READ_SECTOR, begin, count, buf, count)) {
				return ERROR_READ_FAIL;
			}
		}
	} else if (dev->transport == ATA_TRANSPORT_SERIAL_ATA) {
		if (sata_exec_pio_in(&dev->sata, ATA_COMMAND_READ_SECTOR, begin, count, buf, count)) {
			return ERROR_READ_FAIL;
		}
	}
	return 0;
}

int ata_write(void* private, unsigned int begin, int count, const void* buf) {
	struct ATADevice* dev = private;
	if (dev->transport == ATA_TRANSPORT_PARALLEL_ATA) {
		if (dev->pata.use_dma) {
			if ((phyaddr_t)buf < KERNBASE || (phyaddr_t)buf > KERNBASE + PHYSTOP
				|| (phyaddr_t)buf % PGSIZE)
				panic("ata dma");
			if (count == 0 || count > 8)
				panic("ata count");
			if (pata_exec_dma_out(dev->pata.adapter, dev->pata.channel, dev->pata.drive,
								  ATA_COMMAND_WRITE_DMA, begin, count, buf, count)) {
				return ERROR_WRITE_FAIL;
			}
		} else {
			if (pata_exec_pio_out(dev->pata.adapter, dev->pata.channel, dev->pata.drive,
								  ATA_COMMAND_WRITE_SECTOR, begin, count, buf, count)) {
				return ERROR_WRITE_FAIL;
			}
		}
	} else if (dev->transport == ATA_TRANSPORT_SERIAL_ATA) {
		if (sata_exec_pio_out(&dev->sata, ATA_COMMAND_WRITE_SECTOR, begin, count, buf, count)) {
			return ERROR_WRITE_FAIL;
		}
	}
	return 0;
}

const struct BlockDeviceDriver ata_block_driver = {
	.block_read = ata_read,
	.block_write = ata_write,
};

void ata_register_ata_device(struct ATADevice* ata_dev) {
	char model[50];
	if (ata_identify(ata_dev, model) == 0) {
		cprintf("[ata] Disk model %s %d sectors ata_rev %d LBA48%s\n", model, ata_dev->sectors,
				ata_dev->ata_rev, BOOL2SIGN(ata_dev->support_lba48));

		if (ata_dev->transport == ATA_TRANSPORT_PARALLEL_ATA) {
			cprintf("[ata] PATA dma %d pio %d mdma %d udma %d\n", ata_dev->pata.dma,
					ata_dev->pata.pio, ata_dev->pata.mdma, ata_dev->pata.udma);
			if (ata_dev->pata.dma && ata_dev->pata.adapter->bus_master) {
				ata_dev->pata.use_dma = 1;
				cprintf("[ata] Use DMA for channel %d drive %d\n", ata_dev->pata.channel,
						ata_dev->pata.drive);
				pata_adapter_bmdma_init(ata_dev->pata.adapter, ata_dev->pata.channel,
										ata_dev->pata.drive);
			} else {
				ata_dev->pata.use_dma = 0;
				cprintf("[ata] Use PIO for channel %d drive %d\n", ata_dev->pata.channel,
						ata_dev->pata.drive);
			}
		} else if (ata_dev->transport == ATA_TRANSPORT_SERIAL_ATA) {
			if (ata_dev->sata.support_ncq) {
				cprintf(
					"[ata] SATA NCQ%s NCQPrio%s NCQStream%s NCQMgmtCmd%s NCQRXTXFPDMA%s QLen %d\n",
					BOOL2SIGN(ata_dev->sata.support_ncq),
					BOOL2SIGN(ata_dev->sata.support_ncq_priority_info),
					BOOL2SIGN(ata_dev->sata.support_ncq_streaming),
					BOOL2SIGN(ata_dev->sata.support_ncq_queue_mgmt_cmd),
					BOOL2SIGN(ata_dev->sata.support_receive_send_fpdma_queued),
					ata_dev->sata.ncq_queue_depth);
			} else {
				cprintf("[ata] SATA NCQ not supported\n");
			}
		}

		hal_block_register_device("ata", ata_dev, &ata_block_driver);
	}
}

void ata_register_atapi_device(struct ATADevice* ata_dev) {
	panic("register ATAPI device not supported");
}
