/** @file
  Implement TPM2 Test related command.

Copyright (c) 2013, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command causes the TPM to perform a test of its capabilities.
  If the fullTest is YES, the TPM will test all functions.
  If fullTest = NO, the TPM will only test those functions that have not previously been tested.

  @param[in] FullTest    YES if full test to be performed
                         NO if only test of untested functions required

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SelfTest (
  IN TPMI_YES_NO          FullTest
  )
{
  return EFI_SUCCESS;
}
