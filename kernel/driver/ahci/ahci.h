/*
 * Advanced Host Controller Interface (AHCI) driver
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

#ifndef _DRIVER_AHCI_AHCI_H
#define _DRIVER_AHCI_AHCI_H

#include <common/types.h>

struct AHCIController {
	volatile uint32_t* abar;
	volatile struct AHCICommandList* command_list[32];
	volatile void* received_fis_buffer[32];
};

void ahci_init(void);
int ahci_port_get_link_status(struct AHCIController* ahci_controller, unsigned int port);
uint32_t ahci_port_get_signature(struct AHCIController* ahci_controller, unsigned int port);
void ahci_port_reset(struct AHCIController* ahci_controller, unsigned int port);
int ahci_exec_pio_in(struct AHCIController* ahci_controller, unsigned int port, uint8_t cmd,
					 unsigned long long lba, unsigned int cont, void* buf, unsigned int blocks);
int ahci_exec_pio_out(struct AHCIController* ahci, unsigned int port, uint8_t cmd,
					  unsigned long long lba, unsigned int cont, const void* buf,
					  unsigned int blocks);
#endif
