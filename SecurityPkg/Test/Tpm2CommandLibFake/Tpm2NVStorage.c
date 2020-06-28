/** @file
  Implement TPM2 NVStorage related command.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command is used to read the public area and Name of an NV Index.

  @param[in]  NvIndex            The NV Index.
  @param[out] NvPublic           The public area of the index.
  @param[out] NvName             The Name of the nvIndex.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvReadPublic (
  IN      TPMI_RH_NV_INDEX          NvIndex,
  OUT     TPM2B_NV_PUBLIC           *NvPublic,
  OUT     TPM2B_NAME                *NvName
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command defines the attributes of an NV Index and causes the TPM to
  reserve space to hold the data associated with the index.
  If a definition already exists at the index, the TPM will return TPM_RC_NV_DEFINED.

  @param[in]  AuthHandle         TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}.
  @param[in]  AuthSession        Auth Session context
  @param[in]  Auth               The authorization data.
  @param[in]  NvPublic           The public area of the index.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_ALREADY_STARTED    The command was returned successfully, but NvIndex is already defined.
**/
EFI_STATUS
EFIAPI
Tpm2NvDefineSpace (
  IN      TPMI_RH_PROVISION         AuthHandle,
  IN      TPMS_AUTH_COMMAND         *AuthSession, OPTIONAL
  IN      TPM2B_AUTH                *Auth,
  IN      TPM2B_NV_PUBLIC           *NvPublic
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command removes an index from the TPM.

  @param[in]  AuthHandle         TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}.
  @param[in]  NvIndex            The NV Index.
  @param[in]  AuthSession        Auth Session context

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvUndefineSpace (
  IN      TPMI_RH_PROVISION         AuthHandle,
  IN      TPMI_RH_NV_INDEX          NvIndex,
  IN      TPMS_AUTH_COMMAND         *AuthSession OPTIONAL
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command reads a value from an area in NV memory previously defined by TPM2_NV_DefineSpace().

  @param[in]     AuthHandle         the handle indicating the source of the authorization value.
  @param[in]     NvIndex            The index to be read.
  @param[in]     AuthSession        Auth Session context
  @param[in]     Size               Number of bytes to read.
  @param[in]     Offset             Byte offset into the area.
  @param[in,out] OutData            The data read.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvRead (
  IN      TPMI_RH_NV_AUTH           AuthHandle,
  IN      TPMI_RH_NV_INDEX          NvIndex,
  IN      TPMS_AUTH_COMMAND         *AuthSession, OPTIONAL
  IN      UINT16                    Size,
  IN      UINT16                    Offset,
  IN OUT  TPM2B_MAX_BUFFER          *OutData
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command writes a value to an area in NV memory that was previously defined by TPM2_NV_DefineSpace().

  @param[in]  AuthHandle         the handle indicating the source of the authorization value.
  @param[in]  NvIndex            The NV Index of the area to write.
  @param[in]  AuthSession        Auth Session context
  @param[in]  InData             The data to write.
  @param[in]  Offset             The offset into the NV Area.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvWrite (
  IN      TPMI_RH_NV_AUTH           AuthHandle,
  IN      TPMI_RH_NV_INDEX          NvIndex,
  IN      TPMS_AUTH_COMMAND         *AuthSession, OPTIONAL
  IN      TPM2B_MAX_BUFFER          *InData,
  IN      UINT16                    Offset
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command may be used to prevent further reads of the Index until the next TPM2_Startup (TPM_SU_CLEAR).

  @param[in]  AuthHandle         the handle indicating the source of the authorization value.
  @param[in]  NvIndex            The NV Index of the area to lock.
  @param[in]  AuthSession        Auth Session context

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvReadLock (
  IN      TPMI_RH_NV_AUTH           AuthHandle,
  IN      TPMI_RH_NV_INDEX          NvIndex,
  IN      TPMS_AUTH_COMMAND         *AuthSession OPTIONAL
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command may be used to inhibit further writes of the Index.

  @param[in]  AuthHandle         the handle indicating the source of the authorization value.
  @param[in]  NvIndex            The NV Index of the area to lock.
  @param[in]  AuthSession        Auth Session context

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvWriteLock (
  IN      TPMI_RH_NV_AUTH           AuthHandle,
  IN      TPMI_RH_NV_INDEX          NvIndex,
  IN      TPMS_AUTH_COMMAND         *AuthSession OPTIONAL
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  The command will SET TPMA_NV_WRITELOCKED for all indexes that have their TPMA_NV_GLOBALLOCK attribute SET.

  @param[in]  AuthHandle         TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}.
  @param[in]  AuthSession        Auth Session context

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvGlobalWriteLock (
  IN      TPMI_RH_PROVISION         AuthHandle,
  IN      TPMS_AUTH_COMMAND         *AuthSession OPTIONAL
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
