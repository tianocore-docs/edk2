/** @file
  Implement TPM2 Miscellaneous related command.

Copyright (c) 2013 - 2016, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command allows the platform to change the set of algorithms that are used by the TPM.
  The algorithmSet setting is a vendor-dependent value.

  @param[in]  AuthHandle              TPM_RH_PLATFORM
  @param[in]  AuthSession             Auth Session context
  @param[in]  AlgorithmSet            A TPM vendor-dependent value indicating the
                                      algorithm set selection

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SetAlgorithmSet (
  IN  TPMI_RH_PLATFORM          AuthHandle,
  IN  TPMS_AUTH_COMMAND         *AuthSession,
  IN  UINT32                    AlgorithmSet
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
