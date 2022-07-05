/*
 * SATA Structure and Constant definition
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

#ifndef _DRIVER_AHCI_SATA_STRUCT_H
#define _DRIVER_AHCI_SATA_STRUCT_H

#include <common/types.h>

enum SATAFISType {
	RegisterFISHostToDevice = 0x27,
	RegisterFISDeviceToHost = 0x34,
	DMAActivateFIS = 0x39,
	DMASetupFIS = 0x41,
	DataFIS = 0x46,
	BISTActiveFIS = 0x58,
	PIOSetupFIS = 0x5f,
	SetDeviceBitsFIS = 0xa1,
};

struct SATARegisterFISHostToDevice {
	uint8_t fis_type;
	uint8_t pm_c;
	uint8_t command;
	uint8_t features_7_0;
	uint8_t lba_7_0, lba_15_8, lba_23_16, device;
	uint8_t lba_31_24, lba_39_32, lba_47_40, features_15_8;
	uint8_t count_7_0, count_15_8, icc, control;
	uint32_t reserved;
} PACKED;

#define SATA_REGISTER_FIS_H2D_PM_C_COMMAND (1 << 7)
#define SATA_REGISTER_FIS_H2D_PM_C_PM_SHIFT 0
#define SATA_REGISTER_FIS_H2D_PM_C_PM_MASK 0xf

struct SATARegisterFISDeviceToHost {
	uint8_t fis_type;
	uint8_t pm_i;
	uint8_t status, error;
	uint8_t lba_7_0, lba_15_8, lba_23_16, device;
	uint8_t lba_31_24, lba_39_32, lba_47_40, reserved_0;
	uint8_t count_7_0, count_15_8, reserved_1[2];
	uint32_t reserved_2;
} PACKED;

#define SATA_REGISTER_FIS_D2H_PM_I_INTERRUPT (1 << 6)
#define SATA_REGISTER_FIS_D2H_PM_I_PM_SHIFT 0
#define SATA_REGISTER_FIS_D2H_PM_I_PM_MASK 0xf

// SATA device signatures
#define SATA_SIGNATURE_ATA 0x00000101
#define SATA_SIGNATURE_ATAPI 0xeb140101
#define SATA_SIGNATURE_EnclosureServicesDevice 0xc33c0101
#define SATA_SIGNATURE_PortMultiplier 0x96690101

#endif
