/*
 * AHCI MMIO registers definition
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

#ifndef _DRIVER_AHCI_AHCI_REG_H
#define _DRIVER_AHCI_AHCI_REG_H

#define AHCI_CAP 0x0
#define AHCI_CAP_S64A (1 << 31)
#define AHCI_CAP_SNCQ (1 << 30)
#define AHCI_CAP_SSNTF (1 << 29)
#define AHCI_CAP_SMPS (1 << 28)
#define AHCI_CAP_SSS (1 << 27)
#define AHCI_CAP_SALP (1 << 26)
#define AHCI_CAP_SALED (1 << 25)
#define AHCI_CAP_SCLO (1 << 24)

#define AHCI_CAP_ISS_SHIFT 20
#define AHCI_CAP_ISS_MASK 0xf
#define AHCI_CAP_ISS_GEN1 1
#define AHCI_CAP_ISS_GEN2 2
#define AHCI_CAP_ISS_GEN3 3

#define AHCI_CAP_SAM (1 << 18)
#define AHCI_CAP_SPM (1 << 17)
#define AHCI_CAP_FBSS (1 << 16)
#define AHCI_CAP_PMD (1 << 15)
#define AHCI_CAP_SSC (1 << 14)
#define AHCI_CAP_PSC (1 << 13)
#define AHCI_CAP_NCMDS_SHIFT 8
#define AHCI_CAP_NCMDS_MASK 0x1f
#define AHCI_CAP_CCCS (1 << 7)
#define AHCI_CAP_EMS (1 << 6)
#define AHCI_CAP_SXS (1 << 5)
#define AHCI_CAP_NPORTS_SHIFT 0
#define AHCI_CAP_NPORTS_MASK 0x1f

#define AHCI_GHC 0x4
#define AHCI_GHC_AHCIEN (1 << 31)
#define AHCI_GHC_MRSM (1 << 2)
#define AHCI_GHC_INTREN (1 << 1)
#define AHCI_GHC_HBARESET (1 << 0)

#define AHCI_INTRSTAT 0x8

#define AHCI_PORTIMPL 0xc

#define AHCI_VERSION 0x10
#define AHCI_VERSION_MAJOR_SHIFT 16
#define AHCI_VERSION_MAJOR_MASK 0xffff
#define AHCI_VERSION_MINOR_SHIFT 0
#define AHCI_VERSION_MINOR_MASK 0xffff

#define AHCI_CCC_CTL 0x14
#define AHCI_CCC_CTL_TV_SHIFT 16
#define AHCI_CCC_CTL_TV_MASK 0xffff
#define AHCI_CCC_CTL_CC_SHIFT 8
#define AHCI_CCC_CTL_CC_MASK 0xff
#define AHCI_CCC_CTL_INT_SHIFT 3
#define AHCI_CCC_CTL_INT_MASK 0x1f
#define AHCI_CCC_CTL_EN (1 << 0)

#define AHCI_CCC_PORTS 9x18

#define AHCI_EM_LOC 0x1c
#define AHCI_EM_LOC_OFFSET_SHIFT 16
#define AHCI_EM_LOC_OFFSET_MASK 0xffff
#define AHCI_EM_LOC_BUFSZ_SHIFT 0
#define AHCI_EM_LOC_BUFSZ_MASK 0xffff

#define AHCI_EM_CTL 0x20
#define AHCI_EM_CTL_ATTR_PM (1 << 27)
#define AHCI_EM_CTL_ATTR_ALHD (1 << 26)
#define AHCI_EM_CTL_ATTR_XMT (1 << 25)
#define AHCI_EM_CTL_ATTR_SMB (1 << 24)
#define AHCI_EM_CTL_SUPP_SGPIO (1 << 19)
#define AHCI_EM_CTL_SUPP_SES2 (1 << 18)
#define AHCI_EM_CTL_SUPP_SAFTE (1 << 17)
#define AHCI_EM_CTL_SUPP_LED (1 << 16)
#define AHCI_EM_CTL_CTL_RST (1 << 9)
#define AHCI_EM_CTL_CTL_TM (1 << 8)
#define AHCI_EM_CTL_STS_MR (1 << 0)

#define AHCI_CAP2 0x24
#define AHCI_CAP2_DESO (1 << 5)
#define AHCI_CAP2_SADM (1 << 4)
#define AHCI_CAP2_SDS (1 << 3)
#define AHCI_CAP2_APST (1 << 2)
#define AHCI_CAP2_NVMP (1 << 1)
#define AHCI_CAP2_BOH (1 << 0)

#define AHCI_BOHC 0x28
#define AHCI_BOHC_BB (1 << 4)
#define AHCI_BOHC_OOC (1 << 3)
#define AHCI_BOHC_SOOE (1 << 2)
#define AHCI_BOHC_OOS (1 << 1)
#define AHCI_BOHC_BOS (1 << 0)

#endif
