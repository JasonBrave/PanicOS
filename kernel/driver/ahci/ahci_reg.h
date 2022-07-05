/*
 * AHCI MMIO registers and memory structures definition
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

#include <common/types.h>

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
#define AHCI_PxIE_TFEE (1 << 30)
#define AHCI_PxIE_HBFE (1 << 29)
#define AHCI_PxIE_HBDE (1 << 28)
#define AHCI_PxIE_IFE (1 << 27)
#define AHCI_PxIE_INFE (1 << 26)
#define AHCI_PxIE_OFE (1 << 24)
#define AHCI_PxIE_IPME (1 << 23)
#define AHCI_PxIE_PRCE (1 << 22)
#define AHCI_PxIE_DMPE (1 << 7)
#define AHCI_PxIE_PCE (1 << 6)
#define AHCI_PxIE_DPE (1 << 5)
#define AHCI_PxIE_UFE (1 << 4)
#define AHCI_PxIE_SDBE (1 << 3)
#define AHCI_PxIE_DSE (1 << 2)
#define AHCI_PxIE_PSE (1 << 1)
#define AHCI_PxIE_DHRE (1 << 0)

#define AHCI_PxCMD 0x18
#define AHCI_PxCMD_ICC_SHIFT 28
#define AHCI_PxCMD_ICC_MASK 0xf
#define AHCI_PxCMD_ICC_DEVSLEEP 0x8
#define AHCI_PxCMD_ICC_SLUMBER 0x6
#define AHCI_PxCMD_ICC_PARTIAL 0x2
#define AHCI_PxCMD_ICC_ACTIVE 0x1
#define AHCI_PxCMD_ICC_IDLE 0x0

#define AHCI_PxCMD_ASP (1 << 27)
#define AHCI_PxCMD_ALPE (1 << 26)
#define AHCI_PxCMD_DLAE (1 << 25)
#define AHCI_PxCMD_ATAPI (1 << 24)
#define AHCI_PxCMD_APSTE (1 << 23)
#define AHCI_PxCMD_FBSCP (1 << 22)
#define AHCI_PxCMD_ESP (1 << 21)
#define AHCI_PxCMD_CPD (1 << 20)
#define AHCI_PxCMD_MPSP (1 << 19)
#define AHCI_PxCMD_HPCP (1 << 18)
#define AHCI_PxCMD_PMA (1 << 17)
#define AHCI_PxCMD_CPS (1 << 16)
#define AHCI_PxCMD_CR (1 << 15)
#define AHCI_PxCMD_FR (1 << 14)
#define AHCI_PxCMD_MPSS (1 << 13)
#define AHCI_PxCMD_CCS_SHIFT 8
#define AHCI_PxCMD_CCS_MASK 0x1f
#define AHCI_PxCMD_FRE (1 << 4)
#define AHCI_PxCMD_CLO (1 << 3)
#define AHCI_PxCMD_POD (1 << 2)
#define AHCI_PxCMD_SUD (1 << 1)
#define AHCI_PxCMD_ST (1 << 0)

#define AHCI_PxTFD 0x20
#define AHCI_PxTFD_ERROR_SHIFT 8
#define AHCI_PxTFD_ERROR_MASK 0xf
#define AHCI_PxTFD_STATUS_SHIFT 0
#define AHCI_PxTFD_STATUS_MASK 0xf
#define AHCI_PxTFD_STATUS_BSY (1 << 7)
#define AHCI_PxTFD_STATUS_DRQ (1 << 3)
#define AHCI_PxTFD_STATUS_ERR (1 << 0)

#define AHCI_PxSIG 0x24
#define AHCI_PxSIG_DRIVE 0x00000101
#define AHCI_PxSIG_ATAPI 0xeb140101

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
#define AHCI_PxSCTL_IPM_SHIFT 8
#define AHCI_PxSCTL_IPM_MASK 0xf
#define AHCI_PxSCTL_IPM_NO_RESTRICTIONS 0
#define AHCI_PxSCTL_IPM_PARTIAL_DISABLED 1
#define AHCI_PxSCTL_IPM_SLUMBER_DISABLED 2
#define AHCI_PxSCTL_IPM_PARTIAL_SLUMBER_DISABLED 3
#define AHCI_PxSCTL_IPM_DEVSLEEP_DISABLED 4
#define AHCI_PxSCTL_IPM_PARTIAL_DEVSLEEP_DISABLED 5
#define AHCI_PxSCTL_IPM_SLUMBER_DEVSLEEP_DISABLED 6
#define AHCI_PxSCTL_IPM_PARTIAL_SLUMBER_DEVSLEE_DISABLED 7
#define AHCI_PxSCTL_SPD_SHIFT 4
#define AHCI_PxSCTL_SPD_MASK 0xf
#define AHCI_PxSCTL_SPD_NO_RESTRICTIONS 0
#define AHCI_PxSCTL_SPD_MAX_GEN1 1
#define AHCI_PxSCTL_SPD_MAX_GEN2 2
#define AHCI_PxSCTL_SPD_MAX_GEN3 3
#define AHCI_PxSCTL_DET_SHIFT 0
#define AHCI_PxSCTL_DET_MASK 0xf
#define AHCI_PxSCTL_DET_NO_REQUEST 0
#define AHCI_PxSCTL_DET_INTERFACE_INIT 1
#define AHCI_PxSCTL_DET_DISABLE_SATA_INTERFACE 4

#define AHCI_PxSERR 0x30
#define AHCI_PxSERR_DIAG_SHIFT 16
#define AHCI_PxSERR_DIAG_MASK 0xffff
#define AHCI_PxSERR_DIAG_X (1 << 26)
#define AHCI_PxSERR_DIAG_F (1 << 25)
#define AHCI_PxSERR_DIAG_T (1 << 24)
#define AHCI_PxSERR_DIAG_S (1 << 23)
#define AHCI_PxSERR_DIAG_H (1 << 22)
#define AHCI_PxSERR_DIAG_C (1 << 21)
#define AHCI_PxSERR_DIAG_D (1 << 20)
#define AHCI_PxSERR_DIAG_B (1 << 19)
#define AHCI_PxSERR_DIAG_W (1 << 18)
#define AHCI_PxSERR_DIAG_I (1 << 17)
#define AHCI_PxSERR_DIAG_N (1 << 16)
#define AHCI_PxSERR_ERR_SHIFT 0
#define AHCI_PxSERR_ERR_MASK 0xffff
#define AHCI_PxSERR_ERR_E (1 << 11)
#define AHCI_PxSERR_ERR_P (1 << 10)
#define AHCI_PxSERR_ERR_C (1 << 9)
#define AHCI_PxSERR_ERR_T (1 << 8)
#define AHCI_PxSERR_ERR_M (1 << 1)
#define AHCI_PxSERR_ERR_I (1 << 0)

#define AHCI_PxSACT 0x34

#define AHCI_PxCI 0x38

#define AHCI_PxSNTF 0x3c
#define AHCI_PxSNTF_SHIFT 0
#define AHCI_PxSNTF_MASK 0xffff

#define AHCI_PxFBS 0x40
#define AHCI_PxFBS_DWE_SHIFT 16
#define AHCI_PxFBS_DWE_MASK 0xf
#define AHCI_PxFBS_ADO_SHIFT 12
#define AHCI_PxFBS_ADO_MASK 0xf
#define AHCI_PxFBS_DEV_SHIFT 8
#define AHCI_PxFBS_DEV_MASK 0xf
#define AHCI_PxFBS_SDE (1 << 2)
#define AHCI_PxFBS_DEC (1 << 1)
#define AHCI_PxFBS_EN (1 << 0)

#define AHCI_PxDEVSLP 0x44
#define AHCI_PxDEVSLP_DM_SHIFT 25
#define AHCI_PxDEVSLP_DM_MASK 0xf
#define AHCI_PxDEVSLP_DITO_SHIFT 15
#define AHCI_PxDEVSLP_DIOT_MASK 0x3f
#define AHCI_PxDEVSLP_MDAT_SHIFT 10
#define AHCI_PxDEVSLP_MDAT_MASK 0x1f
#define AHCI_PxDEVSLP_DETO_SHIFT 2
#define AHCI_PxDEVSLP_DETO_MASK 0xff
#define AHCI_PxDEVSLP_DSP (1 << 1)
#define AHCI_PxDEVSLP_ADSE (1 << 0)

#define AHCI_RECEIVED_FIS_DSFIS_OFFSET 0x00
#define AHCI_RECEIVED_FIS_PSFIS_OFFSET 0x20
#define AHCI_RECEIVED_FIS_RFIS_OFFSET 0x40
#define AHCI_RECEIVED_FIS_SDBFIS_OFFSET 0x58
#define AHCI_RECEIVED_FIS_UFIS_OFFSET 0x60

struct AHCICommandList {
	uint32_t dw0;
	uint32_t prdbc;
	uint32_t ctba;
	uint32_t ctba_upper;
	uint32_t reserved[4];
} PACKED;

#define AHCI_CMDLIST_DW0_PRDTL_SHIFT 16
#define AHCI_CMDLIST_DW0_PRDTL_MASK 0xffff
#define AHCI_CMDLIST_DW0_PMP_SHIFT 12
#define AHCI_CMDLIST_DW0_PMP_MASK 0xff
#define AHCI_CMDLIST_DW0_C (1 << 10)
#define AHCI_CMDLIST_DW0_B (1 << 9)
#define AHCI_CMDLIST_DW0_R (1 << 8)
#define AHCI_CMDLIST_DW0_P (1 << 7)
#define AHCI_CMDLIST_DW0_W (1 << 6)
#define AHCI_CMDLIST_DW0_A (1 << 5)
#define AHCI_CMDLIST_DW0_CFL_SHIFT 0
#define AHCI_CMDLIST_DW0_CFL_MASK 0x1f

struct AHCIPRDT {
	uint32_t dba, dba_upper;
	uint32_t reserved;
	uint32_t dbc_i;
} PACKED;

#define AHCI_PRDT_DBC_I_I (1 << 31)
#define AHCI_PRDT_DBC_I_DBC_SHIFT 0
#define AHCI_PRDT_DBC_I_DBC_MASK 0x3fffff

struct AHCICommandTable {
	uint8_t cfis[0x40];
	uint8_t acmd[0x10];
	uint8_t reserved[0x30];
	struct AHCIPRDT prdt[];
} PACKED;

#endif
