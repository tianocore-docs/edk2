/*++

Copyright(c) 2013 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
* Neither the name of Intel Corporation nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Module Name:

    MrcWrapper.c

Abstract:

    Framework PEIM to initialize memory on a Quark Memory Controller.


--*/


#include "CommonHeader.h"
#include "MrcWrapper.h"
#include <Ioh.h>
#include "Platform.h"
#include <CpuRegs.h>

#include <Library/PlatformHelperLib.h>

//
// ------------------------ TSEG Base 
// ...............
// ------------------------ RESERVED_CPU_S3_SAVE_OFFSET
// CPU S3 data
// ------------------------ RESERVED_ACPI_S3_RANGE_OFFSET
// S3 Memory base structure
// ------------------------ TSEG + 1 page

// Strap configuration register specifying DDR setup
#define QUARK_SCSS_REG_STPDDRCFG   0x00

// Macro counting array elements
#define COUNT(a)                 (sizeof(a)/sizeof(*a))

/**
  Read south cluster GPIO input from Port A

**/
static UINT32
ScGpioRead(
  VOID
  )
{
  UINT32 GipMmioBase = 0xC1000000;
  UINT32 GipData;
  UINT32 GipAddr;
  UINT8  Cmd;

  GipAddr = PCI_LIB_ADDRESS(
      IOH_I2C_GPIO_BUS_NUMBER,
      IOH_I2C_GPIO_DEVICE_NUMBER,
      IOH_I2C_GPIO_FUNCTION_NUMBER, 0);

  Cmd = PciRead8 ( GipAddr + PCI_COMMAND_OFFSET);

  if( Cmd & EFI_PCI_COMMAND_MEMORY_SPACE) {
    // Someone already enabled mmio decoding
    GipMmioBase = PciRead32 ( GipAddr + PCI_BASE_ADDRESSREG_OFFSET); 

	// Interested in address bits only
	GipMmioBase &= 0xFFFFFF00;
    DEBUG ((EFI_D_INFO, "SC GPIO already enabled at %08X\n", GipMmioBase));

    // Assume default configuration all input
	GipData = MmioRead32( GipMmioBase + GPIO_EXT_PORTA);
  }
  else {
    DEBUG ((EFI_D_INFO, "SC GPIO temporary enable  at %08X\n", GipMmioBase));

    // Use predefined tempory memory resource
    PciWrite32 ( GipAddr + PCI_BASE_ADDRESSREG_OFFSET, GipMmioBase); 
    PciWrite8 ( GipAddr + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE); 

    // Assume default configuration all input
	GipData = MmioRead32( GipMmioBase + GPIO_EXT_PORTA);

    // Disable mmio decoding
	PciWrite8 ( GipAddr + PCI_COMMAND_OFFSET, 0); 
  }

  // Only 20 bits valid
  return GipData & 0x000FFFFF;
}

/**

  Configure ECC scrub

  @param MrcData - MRC configuration

**/
static VOID
EccScrubSetup(
  const MRC_PARAMS *MrcData
  )
{
	UINT32 BgnAdr = 0;
	UINT32 EndAdr = MrcData->mem_size;
	UINT32 BlkSize = PcdGet8(PcdEccScrubBlkSize) & SCRUB_CFG_BLOCKSIZE_MASK;
	UINT32 Interval = PcdGet8(PcdEccScrubInterval) & SCRUB_CFG_INTERVAL_MASK;

	if( MrcData->ecc_enables == 0 || MrcData->boot_mode == bmS3 || Interval == 0) {
		// No scrub configuration needed if ECC not enabled
		// On S3 resume reconfiguration is done as part of resume
		// script, see SNCS3Save.c ==> SaveRuntimeScriptTable()
		// Also if PCD disables scrub, then we do nothing.
		return;
	}

	QNCPortWrite (QUARK_NC_RMU_SB_PORT_ID, QUARK_NC_ECC_SCRUB_END_MEM_REG, EndAdr);
	QNCPortWrite (QUARK_NC_RMU_SB_PORT_ID, QUARK_NC_ECC_SCRUB_START_MEM_REG, BgnAdr);
	QNCPortWrite (QUARK_NC_RMU_SB_PORT_ID, QUARK_NC_ECC_SCRUB_NEXT_READ_REG, BgnAdr);
	QNCPortWrite (QUARK_NC_RMU_SB_PORT_ID, QUARK_NC_ECC_SCRUB_CONFIG_REG,
	  Interval << SCRUB_CFG_INTERVAL_SHIFT |
	  BlkSize << SCRUB_CFG_BLOCKSIZE_SHIFT);

    McD0PciCfg32 (QNC_ACCESS_PORT_MCR) = SCRUB_RESUME_MSG();
}

/** Post InstallS3Memory / InstallEfiMemory tasks given MrcData context.

  @param[in]       MrcData  MRC configuration.
  @param[in]       IsS3     TRUE if after InstallS3Memory.

**/
VOID
PostInstallMemory (
  IN MRC_PARAMS                           *MrcData
  )
{
  UINT32                            CmcDestBaseAddress;
  UINT32                            *CmcSrcBaseAddress;
  UINTN                             CmcSize;
  EFI_STATUS                        Status;

  //
  // Setup ECC policy (All boot modes).
  //
  QNCPolicyDblEccBitErr (V_WDT_CONTROL_DBL_ECC_BIT_ERR_WARM);

  //
  // Find the 64KB memory space for CMC at the top of available memory.
  //
  InfoPostInstallMemory (&CmcDestBaseAddress, NULL, NULL);
  DEBUG ((EFI_D_INFO, "CMC Base Address : 0x%x\n", CmcDestBaseAddress));

  //
  // Relocate CMC.
  //
    CmcSrcBaseAddress = (VOID *)(UINTN)FixedPcdGet32(PcdPlatformFlashRmuPayloadBase);
    CmcSize = FixedPcdGet32(PcdPlatformFlashRmuPayloadSize);
    Status = EFI_SUCCESS;
    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Found Microcode ADDR:SIZE 0x%08x:0x%04x\n", (UINTN) CmcSrcBaseAddress, CmcSize));
    }

    CmcRelocation (CmcDestBaseAddress, (UINT32) CmcSrcBaseAddress, CmcSize);
    QNCSendOpcodeDramReady (CmcDestBaseAddress);
    EccScrubSetup (MrcData);
}

/**

  This function sets up the platform specific IMR protection for the various
  memory regions.

  @param  PeiMemoryBaseAddress  Base address of memory allocated for PEI.
  @param  PeiMemoryLength       Length in bytes of the PEI memory (includes ACPI memory).
  @param  RequiredMemSize       Size in bytes of the ACPI/Runtime memory

  @return EFI_SUCCESS           The function completed successfully.
          EFI_ACCESS_DENIED     Access to IMRs failed.

**/
EFI_STATUS
SetPlatformImrPolicy (
  IN      EFI_PHYSICAL_ADDRESS    PeiMemoryBaseAddress,
  IN      UINT64                  PeiMemoryLength,
  IN      UINTN                   RequiredMemSize
  )
{
  UINT8         Index;
  UINT32        Register;
  UINT16        DeviceId;

  //
  // Check what Soc we are running on (read Host bridge DeviceId)
  //
  DeviceId = QNCMmPci16(0, MC_BUS, MC_DEV, MC_FUN, PCI_DEVICE_ID_OFFSET);

  //
  // If any IMR register is locked then we cannot proceed
  //
  for (Index = (QUARK_NC_MEMORY_MANAGER_IMR0+QUARK_NC_MEMORY_MANAGER_IMRXL); Index <=(QUARK_NC_MEMORY_MANAGER_IMR7+QUARK_NC_MEMORY_MANAGER_IMRXL); Index=Index+4)
  {
    Register = QNCPortRead (QUARK_NC_MEMORY_MANAGER_SB_PORT_ID, Index);
    if (Register & IMR_LOCK) {
      return EFI_ACCESS_DENIED;
    }
  }

  //
  // Add IMR0 protection for the 'PeiMemory'
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR0,
            (UINT32)(((RShiftU64(PeiMemoryBaseAddress, 8)) & IMRL_MASK) | IMR_EN),
            (UINT32)((RShiftU64((PeiMemoryBaseAddress+PeiMemoryLength-RequiredMemSize + EFI_PAGES_TO_SIZE(EDKII_DXE_MEM_SIZE_PAGES-1) - 1), 8)) & IMRL_MASK),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM)
        );

  //
  // Add IMR2 protection for shadowed RMU binary.
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR2,
            (UINT32)(((RShiftU64((PeiMemoryBaseAddress+PeiMemoryLength), 8)) & IMRH_MASK) | IMR_EN),
            (UINT32)((RShiftU64((PeiMemoryBaseAddress+PeiMemoryLength+PcdGet32(PcdFlashQNCMicrocodeSize)-1), 8)) & IMRH_MASK),
            (UINT32)(CPU_SNOOP + RMU + CPU0_NON_SMM),
            (UINT32)(CPU_SNOOP + RMU + CPU0_NON_SMM)
        );

  //
  // Add IMR3 protection for the default SMRAM.
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR3,
            (UINT32)(((RShiftU64((SMM_DEFAULT_SMBASE), 8)) & IMRL_MASK) | IMR_EN),
            (UINT32)((RShiftU64((SMM_DEFAULT_SMBASE+SMM_DEFAULT_SMBASE_SIZE_BYTES-1), 8)) & IMRH_MASK),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM)
        );

  //
  // Add IMR5 protection for the legacy S3 and AP Startup Vector region (below 1MB).
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR5,
            (UINT32)(((RShiftU64(AP_STARTUP_VECTOR, 8)) & IMRL_MASK) | IMR_EN),
            (UINT32)((RShiftU64((AP_STARTUP_VECTOR + EFI_PAGE_SIZE - 1), 8)) & IMRH_MASK),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM)
        );

  //
  // Add IMR6 protection for the ACPI Reclaim/ACPI/Runtime Services.
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR6,
            (UINT32)(((RShiftU64((PeiMemoryBaseAddress+PeiMemoryLength-RequiredMemSize+EFI_PAGES_TO_SIZE(EDKII_DXE_MEM_SIZE_PAGES-1)), 8)) & IMRL_MASK) | IMR_EN),
            (UINT32)((RShiftU64((PeiMemoryBaseAddress+PeiMemoryLength-EFI_PAGE_SIZE-1), 8)) & IMRH_MASK),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM)
        );

  //
  // Enable IMR4 protection of eSRAM.
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR4,
            (UINT32)(((RShiftU64((UINTN)FixedPcdGet32 (PcdEsramStage1Base), 8)) & IMRL_MASK) | IMR_EN),
            (UINT32)((RShiftU64(((UINTN)FixedPcdGet32 (PcdEsramStage1Base) + (UINTN)FixedPcdGet32 (PcdESramMemorySize) - 1), 8)) & IMRH_MASK),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM),
            (UINT32)(CPU_SNOOP + CPU0_NON_SMM)
        );

  //
  // Enable Interrupt on IMR/SMM Violation
  //
  QNCPortWrite (QUARK_NC_MEMORY_MANAGER_SB_PORT_ID, QUARK_NC_MEMORY_MANAGER_BIMRVCTL, (UINT32)(EnableIMRInt));
  if (DeviceId == QUARK2_MC_DEVICE_ID) {
    QNCPortWrite (QUARK_NC_MEMORY_MANAGER_SB_PORT_ID, QUARK_NC_MEMORY_MANAGER_BSMMVCTL, (UINT32)(EnableSMMInt));
  }

  //
  // Disable IMR7 memory protection (eSRAM + DDR3 memory) since our policies
  // are now setup.
  //
  QncImrWrite (
            QUARK_NC_MEMORY_MANAGER_IMR7,
            (UINT32)(IMRL_RESET & ~IMR_EN),
            (UINT32)IMRH_RESET,
            (UINT32)IMRX_ALL_ACCESS,
            (UINT32)IMRX_ALL_ACCESS
        );

  return EFI_SUCCESS;
}

/** Return info derived from Installing Memory by MemoryInit.

  @param[out]      CmcBaseAddressPtr   Return CmcBaseAddress to this location.
  @param[out]      SmramDescriptorPtr  Return start of Smram descriptor list to this location.
  @param[out]      NumSmramRegionsPtr  Return numbers of Smram regions to this location.

  @return Address of CMC region at the top of available memory.
  @return List of Smram descriptors for each Smram region.
  @return Numbers of Smram regions.
**/
VOID
EFIAPI
InfoPostInstallMemory (
  OUT     UINT32                    *CmcBaseAddressPtr OPTIONAL,
  OUT     EFI_SMRAM_DESCRIPTOR      **SmramDescriptorPtr OPTIONAL,
  OUT     UINTN                     *NumSmramRegionsPtr OPTIONAL
  )
{
  UINT64                                CalcLength;
  EFI_SMRAM_HOB_DESCRIPTOR_BLOCK        *SmramHobDescriptorBlock;

  if ((CmcBaseAddressPtr == NULL) && (SmramDescriptorPtr == NULL) && (NumSmramRegionsPtr == NULL)) {
    return;
  }

  SmramHobDescriptorBlock = NULL;
  if (SmramDescriptorPtr != NULL) {
    *SmramDescriptorPtr = NULL;
  }
  if (NumSmramRegionsPtr != NULL) {
    *NumSmramRegionsPtr = 0;
  }

  //
  // Calculate CMC region base address.
  // Set to 1 MB. Since 1MB cacheability will always be set
  // until override by CSM.
  //
  CalcLength = FixedPcdGet32(PcdMemorySize) - FixedPcdGet32(PcdTSegSize) - 0x10000;

  if (CmcBaseAddressPtr != NULL) {
    *CmcBaseAddressPtr = (UINT32) CalcLength;
  }
}
