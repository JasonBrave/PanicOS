/*
 * Serial ATA driver
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

#include <common/types.h>
#include <defs.h>
#include <driver/ahci/ahci.h>
#include <driver/ahci/sata_struct.h>

#include "ata.h"

int sata_exec_pio_in(struct SATADevice* sata_dev, uint8_t cmd, unsigned long long lba,
					 unsigned int cont, void* buf, unsigned int blocks) {
	if (sata_dev->controller->type == SATA_CONTROLLER_TYPE_AHCI) {
		return ahci_exec_pio_in(sata_dev->controller->private, sata_dev->port, cmd, lba, cont, buf,
								blocks);
	} else {
		panic("unknown SATA controller type");
	}
}

int sata_exec_pio_out(struct SATADevice* sata_dev, uint8_t cmd, unsigned long long lba,
					  unsigned int cont, const void* buf, unsigned int blocks) {
	if (sata_dev->controller->type == SATA_CONTROLLER_TYPE_AHCI) {
		return ahci_exec_pio_out(sata_dev->controller->private, sata_dev->port, cmd, lba, cont, buf,
								 blocks);
	} else {
		panic("unknown SATA controller type");
	}
}

int sata_port_get_link_status(struct SATAController* sata_controller, unsigned int port) {
	if (sata_controller->type == SATA_CONTROLLER_TYPE_AHCI) {
		return ahci_port_get_link_status(sata_controller->private, port);
	} else {
		panic("unknown SATA controller type");
	}
}

uint32_t sata_port_get_signature(struct SATAController* sata_controller, unsigned int port) {
	if (sata_controller->type == SATA_CONTROLLER_TYPE_AHCI) {
		return ahci_port_get_signature(sata_controller->private, port);
	} else {
		panic("unknown SATA controller type");
	}
}

void sata_port_reset(struct SATAController* sata_controller, unsigned int port) {
	if (sata_controller->type == SATA_CONTROLLER_TYPE_AHCI) {
		ahci_port_reset(sata_controller->private, port);
	} else {
		panic("unknown SATA controller type");
	}
}

void sata_register_controller(struct SATAController* sata_controller) {
	for (unsigned int port = 0; port < sata_controller->num_ports; port++) {
		sata_port_reset(sata_controller, port);
		if (sata_port_get_link_status(sata_controller, port)) {
			switch (sata_port_get_signature(sata_controller, port)) {
			case SATA_SIGNATURE_ATA:
				cprintf("[ata] ATA device found on SATA controller port %d\n", port);
				struct ATADevice* ata_dev = ata_device_alloc();
				ata_dev->transport = ATA_TRANSPORT_SERIAL_ATA;
				ata_dev->sata.controller = sata_controller;
				ata_dev->sata.port = port;
				ata_register_ata_device(ata_dev);
				break;
			case SATA_SIGNATURE_ATAPI:
				cprintf("[ata] ATAPI device found on SATA controller port %d\n", port);
				break;
			}
		}
	}
}
