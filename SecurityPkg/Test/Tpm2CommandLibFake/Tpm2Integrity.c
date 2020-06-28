/** @file
  Implement TPM2 Integrity related command.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command is used to cause an update to the indicated PCR.
  The digests parameter contains one or more tagged digest value identified by an algorithm ID.
  For each digest, the PCR associated with pcrHandle is Extended into the bank identified by the tag (hashAlg).

  @param[in] PcrHandle   Handle of the PCR
  @param[in] Digests     List of tagged digest values to be extended

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2PcrExtend (
  IN      TPMI_DH_PCR               PcrHandle,
  IN      TPML_DIGEST_VALUES        *Digests
  )
{
  return EFI_SUCCESS;
}

/**
  This command is used to cause an update to the indicated PCR.
  The data in eventData is hashed using the hash algorithm associated with each bank in which the
  indicated PCR has been allocated. After the data is hashed, the digests list is returned. If the pcrHandle
  references an implemented PCR and not TPM_ALG_NULL, digests list is processed as in
  TPM2_PCR_Extend().
  A TPM shall support an Event.size of zero through 1,024 inclusive.

  @param[in]  PcrHandle   Handle of the PCR
  @param[in]  EventData   Event data in sized buffer
  @param[out] Digests     List of digest

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2PcrEvent (
  IN      TPMI_DH_PCR               PcrHandle,
  IN      TPM2B_EVENT               *EventData,
     OUT  TPML_DIGEST_VALUES        *Digests
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the values of all PCR specified in pcrSelect.

  @param[in]  PcrSelectionIn     The selection of PCR to read.
  @param[out] PcrUpdateCounter   The current value of the PCR update counter.
  @param[out] PcrSelectionOut    The PCR in the returned list.
  @param[out] PcrValues          The contents of the PCR indicated in pcrSelect.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PcrRead (
  IN      TPML_PCR_SELECTION        *PcrSelectionIn,
     OUT  UINT32                    *PcrUpdateCounter,
     OUT  TPML_PCR_SELECTION        *PcrSelectionOut,
     OUT  TPML_DIGEST               *PcrValues
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command is used to set the desired PCR allocation of PCR and algorithms.

  @param[in]  AuthHandle         TPM_RH_PLATFORM+{PP}
  @param[in]  AuthSession        Auth Session context
  @param[in]  PcrAllocation      The requested allocation
  @param[out] AllocationSuccess  YES if the allocation succeeded
  @param[out] MaxPCR             maximum number of PCR that may be in a bank
  @param[out] SizeNeeded         number of octets required to satisfy the request
  @param[out] SizeAvailable      Number of octets available. Computed before the allocation

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PcrAllocate (
  IN  TPMI_RH_PLATFORM          AuthHandle,
  IN  TPMS_AUTH_COMMAND         *AuthSession,
  IN  TPML_PCR_SELECTION        *PcrAllocation,
  OUT TPMI_YES_NO               *AllocationSuccess,
  OUT UINT32                    *MaxPCR,
  OUT UINT32                    *SizeNeeded,
  OUT UINT32                    *SizeAvailable
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  Alloc PCR data.

  @param[in]  PlatformAuth      platform auth value. NULL means no platform auth change.
  @param[in]  SupportedPCRBanks Supported PCR banks
  @param[in]  PCRBanks          PCR banks

  @retval EFI_SUCCESS Operation completed successfully.
**/
EFI_STATUS
EFIAPI
Tpm2PcrAllocateBanks (
  IN TPM2B_AUTH                *PlatformAuth,  OPTIONAL
  IN UINT32                    SupportedPCRBanks,
  IN UINT32                    PCRBanks
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
