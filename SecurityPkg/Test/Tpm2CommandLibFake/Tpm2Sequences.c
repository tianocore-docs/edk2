/** @file
  Implement TPM2 Sequences related command.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command starts a hash or an Event sequence.
  If hashAlg is an implemented hash, then a hash sequence is started.
  If hashAlg is TPM_ALG_NULL, then an Event sequence is started.

  @param[in]  HashAlg           The hash algorithm to use for the hash sequence
                                An Event sequence starts if this is TPM_ALG_NULL.
  @param[out] SequenceHandle    A handle to reference the sequence

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2HashSequenceStart (
  IN TPMI_ALG_HASH   HashAlg,
  OUT TPMI_DH_OBJECT *SequenceHandle
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command is used to add data to a hash or HMAC sequence.
  The amount of data in buffer may be any size up to the limits of the TPM.
  NOTE: In all TPM, a buffer size of 1,024 octets is allowed.

  @param[in] SequenceHandle    Handle for the sequence object
  @param[in] Buffer            Data to be added to hash

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SequenceUpdate (
  IN TPMI_DH_OBJECT   SequenceHandle,
  IN TPM2B_MAX_BUFFER *Buffer
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command adds the last part of data, if any, to an Event sequence and returns the result in a digest list.
  If pcrHandle references a PCR and not TPM_RH_NULL, then the returned digest list is processed in
  the same manner as the digest list input parameter to TPM2_PCR_Extend() with the pcrHandle in each
  bank extended with the associated digest value.

  @param[in]  PcrHandle         PCR to be extended with the Event data
  @param[in]  SequenceHandle    Authorization for the sequence
  @param[in]  Buffer            Data to be added to the Event
  @param[out] Results           List of digests computed for the PCR

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2EventSequenceComplete (
  IN TPMI_DH_PCR         PcrHandle,
  IN TPMI_DH_OBJECT      SequenceHandle,
  IN TPM2B_MAX_BUFFER    *Buffer,
  OUT TPML_DIGEST_VALUES *Results
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command adds the last part of data, if any, to a hash/HMAC sequence and returns the result.

  @param[in]  SequenceHandle    Authorization for the sequence
  @param[in]  Buffer            Data to be added to the hash/HMAC
  @param[out] Result            The returned HMAC or digest in a sized buffer

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SequenceComplete (
  IN TPMI_DH_OBJECT      SequenceHandle,
  IN TPM2B_MAX_BUFFER    *Buffer,
  OUT TPM2B_DIGEST       *Result
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
