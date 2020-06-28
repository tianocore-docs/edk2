/** @file
  Implement TPM2 DictionaryAttack related command.

Copyright (c) 2013 - 2016, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command cancels the effect of a TPM lockout due to a number of successive authorization failures.
  If this command is properly authorized, the lockout counter is set to zero.

  @param[in]  LockHandle            TPM_RH_LOCKOUT
  @param[in]  AuthSession           Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2DictionaryAttackLockReset (
  IN  TPMI_RH_LOCKOUT           LockHandle,
  IN  TPMS_AUTH_COMMAND         *AuthSession
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command cancels the effect of a TPM lockout due to a number of successive authorization failures.
  If this command is properly authorized, the lockout counter is set to zero.

  @param[in]  LockHandle            TPM_RH_LOCKOUT
  @param[in]  AuthSession           Auth Session context
  @param[in]  NewMaxTries           Count of authorization failures before the lockout is imposed
  @param[in]  NewRecoveryTime       Time in seconds before the authorization failure count is automatically decremented
  @param[in]  LockoutRecovery       Time in seconds after a lockoutAuth failure before use of lockoutAuth is allowed

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2DictionaryAttackParameters (
  IN  TPMI_RH_LOCKOUT           LockHandle,
  IN  TPMS_AUTH_COMMAND         *AuthSession,
  IN  UINT32                    NewMaxTries,
  IN  UINT32                    NewRecoveryTime,
  IN  UINT32                    LockoutRecovery
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
