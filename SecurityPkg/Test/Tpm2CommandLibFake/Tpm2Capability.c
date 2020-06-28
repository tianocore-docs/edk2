/** @file
  Implement TPM2 Capability related command.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command returns various information regarding the TPM and its current state.

  The capability parameter determines the category of data returned. The property parameter
  selects the first value of the selected category to be returned. If there is no property
  that corresponds to the value of property, the next higher value is returned, if it exists.
  The moreData parameter will have a value of YES if there are more values of the requested
  type that were not returned.
  If no next capability exists, the TPM will return a zero-length list and moreData will have
  a value of NO.

  NOTE:
  To simplify this function, leave returned CapabilityData for caller to unpack since there are
  many capability categories and only few categories will be used in firmware. It means the caller
  need swap the byte order for the fields in CapabilityData.

  @param[in]  Capability         Group selection; determines the format of the response.
  @param[in]  Property           Further definition of information.
  @param[in]  PropertyCount      Number of properties of the indicated type to return.
  @param[out] MoreData           Flag to indicate if there are more values of this type.
  @param[out] CapabilityData     The capability data.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapability (
  IN      TPM_CAP                   Capability,
  IN      UINT32                    Property,
  IN      UINT32                    PropertyCount,
  OUT     TPMI_YES_NO               *MoreData,
  OUT     TPMS_CAPABILITY_DATA      *CapabilityData
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM Family.

  This function parse the value got from TPM2_GetCapability and return the Family.

  @param[out] Family             The Family of TPM. (a 4-octet character string)

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityFamily (
  OUT     CHAR8                     *Family
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM manufacture ID.

  This function parse the value got from TPM2_GetCapability and return the TPM manufacture ID.

  @param[out] ManufactureId      The manufacture ID of TPM.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityManufactureID (
  OUT     UINT32                    *ManufactureId
  )
{
  *ManufactureId = 0x12345678;
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM FirmwareVersion.

  This function parse the value got from TPM2_GetCapability and return the TPM FirmwareVersion.

  @param[out] FirmwareVersion1   The FirmwareVersion1.
  @param[out] FirmwareVersion2   The FirmwareVersion2.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityFirmwareVersion (
  OUT     UINT32                    *FirmwareVersion1,
  OUT     UINT32                    *FirmwareVersion2
  )
{
  *FirmwareVersion1 = 0x20200628;
  *FirmwareVersion2 = 0x20200628;
  return EFI_SUCCESS;
}

/**
  This command returns the information of the maximum value for commandSize and responseSize in a command.

  This function parse the value got from TPM2_GetCapability and return the max command size and response size

  @param[out] MaxCommandSize     The maximum value for commandSize in a command.
  @param[out] MaxResponseSize    The maximum value for responseSize in a command.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityMaxCommandResponseSize (
  OUT UINT32                    *MaxCommandSize,
  OUT UINT32                    *MaxResponseSize
  )
{
  *MaxCommandSize = 0x400;
  *MaxResponseSize = 0x400;
  return EFI_SUCCESS;
}

/**
  This command returns Returns a list of TPMS_ALG_PROPERTIES. Each entry is an
  algorithm ID and a set of properties of the algorithm.

  This function parse the value got from TPM2_GetCapability and return the list.

  @param[out] AlgList      List of algorithm.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilitySupportedAlg (
  OUT TPML_ALG_PROPERTY      *AlgList
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM LockoutCounter.

  This function parse the value got from TPM2_GetCapability and return the LockoutCounter.

  @param[out] LockoutCounter     The LockoutCounter of TPM.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityLockoutCounter (
  OUT     UINT32                    *LockoutCounter
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM LockoutInterval.

  This function parse the value got from TPM2_GetCapability and return the LockoutInterval.

  @param[out] LockoutInterval    The LockoutInterval of TPM.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityLockoutInterval (
  OUT     UINT32                    *LockoutInterval
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM InputBufferSize.

  This function parse the value got from TPM2_GetCapability and return the InputBufferSize.

  @param[out] InputBufferSize    The InputBufferSize of TPM.
                                 the maximum size of a parameter (typically, a TPM2B_MAX_BUFFER)

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityInputBufferSize (
  OUT     UINT32                    *InputBufferSize
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM PCRs.

  This function parse the value got from TPM2_GetCapability and return the PcrSelection.

  @param[out] Pcrs    The Pcr Selection

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityPcrs (
  OUT TPML_PCR_SELECTION      *Pcrs
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This function will query the TPM to determine which hashing algorithms
  are supported and which PCR banks are currently active.

  @param[out]  TpmHashAlgorithmBitmap A bitmask containing the algorithms supported by the TPM.
  @param[out]  ActivePcrBanks         A bitmask containing the PCRs currently allocated.

  @retval     EFI_SUCCESS   TPM was successfully queried and return values can be trusted.
  @retval     Others        An error occurred, likely in communication with the TPM.

**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilitySupportedAndActivePcrs (
  OUT UINT32                            *TpmHashAlgorithmBitmap,
  OUT UINT32                            *ActivePcrBanks
  )
{
  *TpmHashAlgorithmBitmap = HASH_ALG_SHA256;
  *ActivePcrBanks = HASH_ALG_SHA256;
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM AlgorithmSet.

  This function parse the value got from TPM2_GetCapability and return the AlgorithmSet.

  @param[out] AlgorithmSet    The AlgorithmSet of TPM.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityAlgorithmSet (
  OUT     UINT32      *AlgorithmSet
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}

/**
  This command is used to check to see if specific combinations of algorithm parameters are supported.

  @param[in]  Parameters              Algorithm parameters to be validated

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2TestParms (
  IN  TPMT_PUBLIC_PARMS           *Parameters
  )
{
  return EFI_SUCCESS;
}
