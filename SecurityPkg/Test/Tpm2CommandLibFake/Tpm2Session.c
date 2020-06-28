/** @file
  Implement TPM2 Session related command.

Copyright (c) 2014 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command is used to start an authorization session using alternative methods of
  establishing the session key (sessionKey) that is used for authorization and encrypting value.

  @param[in]  TpmKey             Handle of a loaded decrypt key used to encrypt salt.
  @param[in]  Bind               Entity providing the authValue.
  @param[in]  NonceCaller        Initial nonceCaller, sets nonce size for the session.
  @param[in]  Salt               Value encrypted according to the type of tpmKey.
  @param[in]  SessionType        Indicates the type of the session.
  @param[in]  Symmetric          The algorithm and key size for parameter encryption.
  @param[in]  AuthHash           Hash algorithm to use for the session.
  @param[out] SessionHandle      Handle for the newly created session.
  @param[out] NonceTPM           The initial nonce from the TPM, used in the computation of the sessionKey.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2StartAuthSession (
  IN      TPMI_DH_OBJECT            TpmKey,
  IN      TPMI_DH_ENTITY            Bind,
  IN      TPM2B_NONCE               *NonceCaller,
  IN      TPM2B_ENCRYPTED_SECRET    *Salt,
  IN      TPM_SE                    SessionType,
  IN      TPMT_SYM_DEF              *Symmetric,
  IN      TPMI_ALG_HASH             AuthHash,
     OUT  TPMI_SH_AUTH_SESSION      *SessionHandle,
     OUT  TPM2B_NONCE               *NonceTPM
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
