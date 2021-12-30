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

// AHCI Generic Host Control registers

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

// AHCI Port Control registers
#define AHCI_PORT_CONTROL_OFFSET(port) (0x100 + port * 0x80)

#define AHCI_PxCLB_LOWER 0x0
#define AHCI_PxCLB_UPPER 0x4
#define AHCI_PxFB_LOWER 0x8
#define AHCI_PxFB_UPPER 0xc

#define AHCI_PxIS 0x10
#define AHCI_PxIS_CPDS (1 << 31)
#define AHCI_PxIS_TFES (1 << 30)
#define AHCI_PxIS_HBFS (1 << 29)
#define AHCI_PxIS_HBDS (1 << 28)
#define AHCI_PxIS_IFS (1 << 27)
#define AHCI_PxIS_INFS (1 << 26)
#define AHCI_PxIS_OFS (1 << 24)
#define AHCI_PxIS_IPMS (1 << 23)
#define AHCI_PxIS_PRCS (1 << 22)
#define AHCI_PxIS_DMPS (1 << 7)
#define AHCI_PxIS_PCS (1 << 6)
#define AHCI_PxIS_DPS (1 << 5)
#define AHCI_PxIS_UFS (1 << 4)
#define AHCI_PxIS_SDBS (1 << 3)
#define AHCI_PxIS_DSS (1 << 2)
#define AHCI_PxIS_PSS (1 << 1)
#define AHCI_PxIS_DHRS (1 << 0)

#define AHCI_PxIE 0x14
#define AHCI_PxIE_CPDE (1 << 31)

#define AHCI_PxCMD 0x18
#define AHCI_PxCMD_FRE (1 << 4)

#define AHCI_PxTFD 0x20

#define AHCI_PxSIG 0x24

#define AHCI_PxSSTS 0x28
#define AHCI_PxSSTS_IPM_SHIFT 8
#define AHCI_PxSSTS_IPM_MASK 0xf
#define AHCI_PxSSTS_IPM_NOT_PRESENT 0
#define AHCI_PxSSTS_IPM_ACTIVE 1
#define AHCI_PxSSTS_IPM_PARTIAL 2
#define AHCI_PxSSTS_IPM_SLUMBER 6
#define AHCI_PxSSTS_IPM_DEVSLEEP 8
#define AHCI_PxSSTS_SPD_SHIFT 4
#define AHCI_PxSSTS_SPD_MASK 0xf
#define AHCI_PxSSTS_SPD_NOT_PRESENT 0
#define AHCI_PxSSTS_SPD_GEN1 1
#define AHCI_PxSSTS_SPD_GEN2 2
#define AHCI_PxSSTS_SPD_GEN3 3
#define AHCI_PxSSTS_DET_SHIFT 0
#define AHCI_PxSSTS_DET_MASK 0xf
#define AHCI_PxSSTS_DET_NO_DETECT 0
#define AHCI_PxSSTS_DET_PRESENSE_NO_COMM 1
#define AHCI_PxSSTS_DET_PRESENSE_COMM 3
#define AHCI_PxSSTS_DET_PHY_OFFLINE 4

#define AHCI_PxSCTL 0x2c

#define AHCI_PxSERR 0x30

#define AHCI_PxSACT 0x34

#define AHCI_PxCI 0x38

#define AHCI_PxSNTF 0x3c

#define AHCI_PxFBS 0x40

#define AHCI_PxDEVSLP 0x44

#endif
