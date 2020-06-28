/** @file
  Implement TPM2 Hierarchy related command.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command allows setting of the authorization policy for the platform hierarchy (platformPolicy), the
  storage hierarchy (ownerPolicy), and and the endorsement hierarchy (endorsementPolicy).

  @param[in]  AuthHandle            TPM_RH_ENDORSEMENT, TPM_RH_OWNER or TPM_RH_PLATFORM+{PP} parameters to be validated
  @param[in]  AuthSession           Auth Session context
  @param[in]  AuthPolicy            An authorization policy hash
  @param[in]  HashAlg               The hash algorithm to use for the policy

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SetPrimaryPolicy (
  IN  TPMI_RH_HIERARCHY_AUTH    AuthHandle,
  IN  TPMS_AUTH_COMMAND         *AuthSession,
  IN  TPM2B_DIGEST              *AuthPolicy,
  IN  TPMI_ALG_HASH             HashAlg
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command removes all TPM context associated with a specific Owner.

  @param[in] AuthHandle        TPM_RH_LOCKOUT or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2Clear (
  IN TPMI_RH_CLEAR             AuthHandle,
  IN TPMS_AUTH_COMMAND         *AuthSession OPTIONAL
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  Disables and enables the execution of TPM2_Clear().

  @param[in] AuthHandle        TPM_RH_LOCKOUT or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
  @param[in] Disable           YES if the disableOwnerClear flag is to be SET,
                               NO if the flag is to be CLEAR.

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ClearControl (
  IN TPMI_RH_CLEAR             AuthHandle,
  IN TPMS_AUTH_COMMAND         *AuthSession, OPTIONAL
  IN TPMI_YES_NO               Disable
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command allows the authorization secret for a hierarchy or lockout to be changed using the current
  authorization value as the command authorization.

  @param[in] AuthHandle        TPM_RH_LOCKOUT, TPM_RH_ENDORSEMENT, TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
  @param[in] NewAuth           New authorization secret

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2HierarchyChangeAuth (
  IN TPMI_RH_HIERARCHY_AUTH    AuthHandle,
  IN TPMS_AUTH_COMMAND         *AuthSession,
  IN TPM2B_AUTH                *NewAuth
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This replaces the current EPS with a value from the RNG and sets the Endorsement hierarchy controls to
  their default initialization values.

  @param[in] AuthHandle        TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ChangeEPS (
  IN TPMI_RH_PLATFORM          AuthHandle,
  IN TPMS_AUTH_COMMAND         *AuthSession
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This replaces the current PPS with a value from the RNG and sets platformPolicy to the default
  initialization value (the Empty Buffer).

  @param[in] AuthHandle        TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ChangePPS (
  IN TPMI_RH_PLATFORM          AuthHandle,
  IN TPMS_AUTH_COMMAND         *AuthSession
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command enables and disables use of a hierarchy.

  @param[in] AuthHandle        TPM_RH_ENDORSEMENT, TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
  @param[in] Hierarchy         Hierarchy of the enable being modified
  @param[in] State             YES if the enable should be SET,
                               NO if the enable should be CLEAR

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2HierarchyControl (
  IN TPMI_RH_HIERARCHY         AuthHandle,
  IN TPMS_AUTH_COMMAND         *AuthSession,
  IN TPMI_RH_HIERARCHY         Hierarchy,
  IN TPMI_YES_NO               State
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
