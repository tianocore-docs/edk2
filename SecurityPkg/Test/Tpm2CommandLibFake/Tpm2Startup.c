/** @file
  Implement TPM2 Startup related command.

Copyright (c) 2013 - 2016, Intel Corporation. All rights reserved. <BR>
(C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  Send Startup command to TPM2.

  @param[in] StartupType           TPM_SU_CLEAR or TPM_SU_STATE

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2Startup (
  IN      TPM_SU             StartupType
  )
{
  return EFI_SUCCESS;
}

/**
  Send Shutdown command to TPM2.

  @param[in] ShutdownType           TPM_SU_CLEAR or TPM_SU_STATE.

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2Shutdown (
  IN      TPM_SU             ShutdownType
  )
{
  return EFI_SUCCESS;
}
