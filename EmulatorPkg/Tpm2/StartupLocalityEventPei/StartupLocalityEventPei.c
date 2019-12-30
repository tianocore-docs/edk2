/** @file
  Report Startup Locality event

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/UefiTcgPlatform.h>

/**
  Main entry for this driver.

  @param FfsHeader     A pointer to the EFI_FFS_FILE_HEADER structure.
  @param PeiServices   General purpose services available to every PEIM.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
StartupLocalityEventPeiEntryPoint (
  IN        EFI_PEI_FILE_HANDLE   FileHandle,
  IN  CONST EFI_PEI_SERVICES      **PeiServices
  )
{
  UINT8  *StartupLocality;

  StartupLocality = BuildGuidHob (&gTpm2StartupLocalityHobGuid, sizeof(UINT8));
  ASSERT(StartupLocality != NULL);
  *StartupLocality = LOCALITY_0_INDICATOR;

  return EFI_SUCCESS;
}
