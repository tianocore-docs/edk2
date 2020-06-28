/** @file
  Implement TPM2 EnhancedAuthorization related command.

Copyright (c) 2014 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command includes a secret-based authorization to a policy.
  The caller proves knowledge of the secret value using an authorization
  session using the authValue associated with authHandle.

  @param[in]  AuthHandle         Handle for an entity providing the authorization
  @param[in]  PolicySession      Handle for the policy session being extended.
  @param[in]  AuthSession        Auth Session context
  @param[in]  NonceTPM           The policy nonce for the session.
  @param[in]  CpHashA            Digest of the command parameters to which this authorization is limited.
  @param[in]  PolicyRef          A reference to a policy relating to the authorization.
  @param[in]  Expiration         Time when authorization will expire, measured in seconds from the time that nonceTPM was generated.
  @param[out] Timeout            Time value used to indicate to the TPM when the ticket expires.
  @param[out] PolicyTicket       A ticket that includes a value indicating when the authorization expires.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PolicySecret (
  IN      TPMI_DH_ENTITY            AuthHandle,
  IN      TPMI_SH_POLICY            PolicySession,
  IN      TPMS_AUTH_COMMAND         *AuthSession, OPTIONAL
  IN      TPM2B_NONCE               *NonceTPM,
  IN      TPM2B_DIGEST              *CpHashA,
  IN      TPM2B_NONCE               *PolicyRef,
  IN      INT32                     Expiration,
  OUT     TPM2B_TIMEOUT             *Timeout,
  OUT     TPMT_TK_AUTH              *PolicyTicket
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command allows options in authorizations without requiring that the TPM evaluate all of the options.
  If a policy may be satisfied by different sets of conditions, the TPM need only evaluate one set that
  satisfies the policy. This command will indicate that one of the required sets of conditions has been
  satisfied.

  @param[in] PolicySession      Handle for the policy session being extended.
  @param[in] HashList           the list of hashes to check for a match.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PolicyOR (
  IN TPMI_SH_POLICY           PolicySession,
  IN TPML_DIGEST              *HashList
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command indicates that the authorization will be limited to a specific command code.

  @param[in]  PolicySession      Handle for the policy session being extended.
  @param[in]  Code               The allowed commandCode.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PolicyCommandCode (
  IN      TPMI_SH_POLICY            PolicySession,
  IN      TPM_CC                    Code
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the current policyDigest of the session. This command allows the TPM
  to be used to perform the actions required to precompute the authPolicy for an object.

  @param[in]  PolicySession      Handle for the policy session.
  @param[out] PolicyHash         the current value of the policyHash of policySession.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PolicyGetDigest (
  IN      TPMI_SH_POLICY            PolicySession,
     OUT  TPM2B_DIGEST              *PolicyHash
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
