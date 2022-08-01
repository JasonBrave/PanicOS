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

#include <defs.h>
#include <driver/ata/ata.h>
#include <driver/pci/pci.h>
#include <memlayout.h>

#include "ahci.h"
#include "ahci_reg.h"
#include "sata_struct.h"

static struct AHCIController* ahci_controller_new(void) {
	struct AHCIController* dev = kalloc();
	memset(dev, 0, sizeof(struct AHCIController));
	return dev;
}

static inline uint32_t ahci_read_reg32(struct AHCIController* ahci, unsigned int offset) {
	return ahci->abar[offset >> 2];
}

static inline void ahci_write_reg32(struct AHCIController* ahci, unsigned int offset,
									uint32_t value) {
	ahci->abar[offset >> 2] = value;
}

void ahci_init_port(struct AHCIController* ahci, unsigned int port) {
	if (((ahci_read_reg32(ahci, AHCI_PORTIMPL) >> port) & 1) == 0) {
		return;
	}
	cprintf("[ahci] Port %d SStatus %x SControl %x SError %x SActive %x\n", port,
			ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSSTS),
			ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSCTL),
			ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSERR),
			ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSACT));
	if (((ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSSTS)
		  >> AHCI_PxSSTS_DET_SHIFT)
		 & AHCI_PxSSTS_DET_MASK)
		== AHCI_PxSSTS_DET_PRESENSE_COMM) {
		if (((ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSSTS)
			  >> AHCI_PxSSTS_IPM_SHIFT)
			 & AHCI_PxSSTS_IPM_MASK)
			== AHCI_PxSSTS_IPM_ACTIVE) {
			cprintf("[ahci] Port %d connected Gen%d\n", port,
					(ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSSTS)
					 >> AHCI_PxSSTS_SPD_SHIFT)
						& AHCI_PxSSTS_SPD_MASK);
		} else {
			return;
		}
	} else {
		return;
	}
	// allocate command list
	volatile struct AHCICommandList* command_list = kalloc();
	ahci->command_list[port] = command_list;
	memset_volatile(command_list, 0, 4096);
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCLB_LOWER, V2P(command_list));
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCLB_UPPER, 0);
	// allocate received FIS buffer
	volatile void* received_fis_buffer = kalloc();
	ahci->received_fis_buffer[port] = received_fis_buffer;
	memset_volatile(received_fis_buffer, 0, 4096);
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxFB_LOWER,
					 V2P(received_fis_buffer));
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxFB_UPPER, 0);
	// enable receive FIS
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCMD,
					 ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCMD)
						 | AHCI_PxCMD_FRE);
	// clear error register
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSERR, 0xffffffff);
	// read and print signature
	uint32_t atasig = ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSIG);
	switch (atasig) {
	case SATA_SIGNATURE_ATA:
		cprintf("[ahci] Port %d ATA signature %x Type ATA Disk\n", port, atasig);
		break;
	case SATA_SIGNATURE_ATAPI:
		cprintf("[ahci] Port %d ATA signature %x Type ATAPI Device\n", port, atasig);
		break;
	default:
		cprintf("[ahci] Port %d ATA signature %x Type other\n", port, atasig);
	}
}

void ahci_controller_init(struct PCIDevice* pci_dev) {
	const struct PciAddress* pci_addr = &pci_dev->addr;
	struct AHCIController* ahci = ahci_controller_new();
	pci_dev->private = ahci;
	ahci->abar = map_mmio_region(pci_read_bar(pci_addr, 5), pci_read_bar_size(pci_addr, 5));
	cprintf("[ahci] AHCI Controller at PCI %x:%x.%x ABAR %x\n", pci_addr->bus, pci_addr->device,
			pci_addr->function, pci_read_bar(&pci_dev->addr, 5));
	// set AHCI enable bit
	ahci_write_reg32(ahci, AHCI_GHC, ahci_read_reg32(ahci, AHCI_GHC) | AHCI_GHC_AHCIEN);
	// reset AHCI HBA
	ahci_write_reg32(ahci, AHCI_GHC, ahci_read_reg32(ahci, AHCI_GHC) | AHCI_GHC_HBARESET);
	while (ahci_read_reg32(ahci, AHCI_GHC) & AHCI_GHC_HBARESET) {
	}
	// set AHCI enable bit again after reset
	ahci_write_reg32(ahci, AHCI_GHC, ahci_read_reg32(ahci, AHCI_GHC) | AHCI_GHC_AHCIEN);
	// print AHCI version
	uint32_t ahciver = ahci_read_reg32(ahci, AHCI_VERSION);
	cprintf("[ahci] AHCI version %x.%x\n",
			(ahciver >> AHCI_VERSION_MAJOR_SHIFT) & AHCI_VERSION_MAJOR_MASK,
			(ahciver >> AHCI_VERSION_MINOR_SHIFT) & AHCI_VERSION_MINOR_MASK);
	// print AHCI capability
	uint32_t ahcicap = ahci_read_reg32(ahci, AHCI_CAP);
	cprintf("[ahci] AHCI 64Addr%s NCQ%s Speed %x AHCIOnly%s NCmdSlot %d NPorts %d\n",
			BOOL2SIGN(ahcicap & AHCI_CAP_S64A), BOOL2SIGN(ahcicap & AHCI_CAP_SNCQ),
			(ahcicap >> AHCI_CAP_ISS_SHIFT) & AHCI_CAP_ISS_MASK, BOOL2SIGN(ahcicap & AHCI_CAP_SAM),
			((ahcicap >> AHCI_CAP_NCMDS_SHIFT) & AHCI_CAP_NCMDS_MASK) + 1,
			((ahcicap >> AHCI_CAP_NPORTS_SHIFT) & AHCI_CAP_NPORTS_MASK) + 1);
	// AHCI BIOS OS handoff
	if (ahci_read_reg32(ahci, AHCI_CAP2) & AHCI_CAP2_BOH) {
		cprintf("[ahci] Start BIOS OS handoff (not implemented)");
	}
	// per-port initialization
	for (unsigned int i = 0;
		 i
		 < ((ahci_read_reg32(ahci, AHCI_CAP) >> AHCI_CAP_NPORTS_SHIFT) & AHCI_CAP_NPORTS_MASK) + 1;
		 i++) {
		ahci_init_port(ahci, i);
	}
	// register SATA controller
	struct SATAController* sata_controller = kalloc();
	memset(sata_controller, 0, sizeof(struct SATAController));
	sata_controller->type = SATA_CONTROLLER_TYPE_AHCI;
	sata_controller->num_ports
		= ((ahci_read_reg32(ahci, AHCI_CAP) >> AHCI_CAP_NPORTS_SHIFT) & AHCI_CAP_NPORTS_MASK) + 1;
	sata_controller->private = ahci;
	sata_register_controller(sata_controller);
}

struct PCIDriver ahci_driver = {
	.name = "ahci",
	.class_type = 0x010601, // AHCI
	.init = ahci_controller_init,
};

void ahci_init(void) {
	pci_register_driver(&ahci_driver);
}

int ahci_port_get_link_status(struct AHCIController* ahci_controller, unsigned int port) {
	if (((ahci_read_reg32(ahci_controller, AHCI_PORTIMPL) >> port) & 1) == 0) {
		return 0;
	}
	return (ahci_read_reg32(ahci_controller, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSSTS)
			>> AHCI_PxSSTS_SPD_SHIFT)
		   & AHCI_PxSSTS_SPD_MASK;
}

uint32_t ahci_port_get_signature(struct AHCIController* ahci_controller, unsigned int port) {
	return ahci_read_reg32(ahci_controller, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxSIG);
}

void ahci_port_reset(struct AHCIController* ahci_controller, unsigned int port) {}

int ahci_exec_pio_in(struct AHCIController* ahci, unsigned int port, uint8_t cmd,
					 unsigned long long lba, unsigned int cont, void* buf, unsigned int blocks) {
	// allocate command table
	volatile struct AHCICommandTable* cmd_table = kalloc();
	memset_volatile(cmd_table, 0, 4096);
	// Set command list
	ahci->command_list[port][0].ctba = V2P(cmd_table);
	ahci->command_list[port][0].ctba_upper = 0;
	ahci->command_list[port][0].dw0
		= (1 << AHCI_CMDLIST_DW0_PRDTL_SHIFT) | (5 << AHCI_CMDLIST_DW0_CFL_SHIFT);
	ahci->command_list[port][0].prdbc = 0;
	// Set FIS
	volatile struct SATARegisterFISHostToDevice* fis
		= (volatile struct SATARegisterFISHostToDevice*)cmd_table;
	fis->fis_type = RegisterFISHostToDevice;
	fis->pm_c = SATA_REGISTER_FIS_H2D_PM_C_COMMAND;
	fis->command = cmd;
	fis->lba_7_0 = lba & 0xff;
	fis->lba_15_8 = (lba >> 8) & 0xff;
	fis->lba_23_16 = (lba >> 16) & 0xff;
	fis->lba_31_24 = (lba >> 24) & 0xff;
	fis->lba_39_32 = (lba >> 32) & 0xff;
	fis->lba_47_40 = (lba >> 40) & 0xff;
	fis->count_7_0 = cont & 0xff;
	fis->count_15_8 = (cont >> 8) & 0xff;
	fis->device = (1 << 6); // Bit 6 shall be set to one
	// Set receive buffer
	cmd_table->prdt[0].dba = V2P(buf);
	cmd_table->prdt[0].dba_upper = 0;
	cmd_table->prdt[0].dbc_i = AHCI_PRDT_DBC_I_I | (512 * blocks - 1);
	// enable command list
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCMD,
					 ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCMD)
						 | AHCI_PxCMD_ST);
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxIS, 0xffffffff);
	// issue command
	while (
		(ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxTFD) & AHCI_PxTFD_STATUS_BSY)
		|| (ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxTFD)
			& AHCI_PxTFD_STATUS_DRQ)) {
	}
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCI, 1);
	while (ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCI) & 1) {
	}
	kfree(cmd_table);
	return 0;
}

int ahci_exec_pio_out(struct AHCIController* ahci, unsigned int port, uint8_t cmd,
					  unsigned long long lba, unsigned int cont, const void* buf,
					  unsigned int blocks) {
	// allocate command table
	volatile struct AHCICommandTable* cmd_table = kalloc();
	memset_volatile(cmd_table, 0, 4096);
	// Set command list
	ahci->command_list[port][0].ctba = V2P(cmd_table);
	ahci->command_list[port][0].ctba_upper = 0;
	ahci->command_list[port][0].dw0 = (1 << AHCI_CMDLIST_DW0_PRDTL_SHIFT)
									  | (5 << AHCI_CMDLIST_DW0_CFL_SHIFT) | AHCI_CMDLIST_DW0_W;
	ahci->command_list[port][0].prdbc = 0;
	// Set FIS
	volatile struct SATARegisterFISHostToDevice* fis
		= (volatile struct SATARegisterFISHostToDevice*)cmd_table;
	fis->fis_type = RegisterFISHostToDevice;
	fis->pm_c = SATA_REGISTER_FIS_H2D_PM_C_COMMAND;
	fis->command = cmd;
	fis->lba_7_0 = lba & 0xff;
	fis->lba_15_8 = (lba >> 8) & 0xff;
	fis->lba_23_16 = (lba >> 16) & 0xff;
	fis->lba_31_24 = (lba >> 24) & 0xff;
	fis->lba_39_32 = (lba >> 32) & 0xff;
	fis->lba_47_40 = (lba >> 40) & 0xff;
	fis->count_7_0 = cont & 0xff;
	fis->count_15_8 = (cont >> 8) & 0xff;
	fis->device = (1 << 6); // Bit 6 shall be set to one
	// Set receive buffer
	cmd_table->prdt[0].dba = V2P(buf);
	cmd_table->prdt[0].dba_upper = 0;
	cmd_table->prdt[0].dbc_i = AHCI_PRDT_DBC_I_I | (512 * blocks - 1);
	// enable command list
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCMD,
					 ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCMD)
						 | AHCI_PxCMD_ST);
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxIS, 0xffffffff);
	// issue command
	while (
		(ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxTFD) & AHCI_PxTFD_STATUS_BSY)
		|| (ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxTFD)
			& AHCI_PxTFD_STATUS_DRQ)) {
	}
	ahci_write_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCI, 1);
	while (ahci_read_reg32(ahci, AHCI_PORT_CONTROL_OFFSET(port) + AHCI_PxCI) & 1) {
	}
	kfree(cmd_table);
	return 0;
}
