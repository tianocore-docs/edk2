/** @file
  Implement TPM2 Context related command.

Copyright (c) 2014 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command causes all context associated with a loaded object or session to be removed from TPM memory.

  @param[in]  FlushHandle        The handle of the item to flush.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2FlushContext (
  IN      TPMI_DH_CONTEXT           FlushHandle
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

