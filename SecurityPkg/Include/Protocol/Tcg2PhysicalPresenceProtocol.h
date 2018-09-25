/** @file
  This library is intended to be used by BDS modules.
  This library will execute TPM2 request.

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _TCG2_PHYSICAL_PRESENCE_PROTOCOL_H_
#define _TCG2_PHYSICAL_PRESENCE_PROTOCOL_H_

#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/TcgPhysicalPresence.h>
#include <Protocol/Tcg2Protocol.h>

#define EFI_TCG2_PHYSICAL_PRESENCE_PROTOCOL_GUID \
  { 0xe2e791e0, 0xe408, 0x4219, { 0xb2, 0x71, 0x68, 0x1d, 0xfd, 0xaa, 0x97, 0x2f } }

typedef struct _EFI_TCG2_PHYSICALPRESENCE_PROTOCOL TCG2_PHYSICAL_PRESENCE_PROTOCOL;

/**
  Check and execute the pending TPM request.

  The TPM request may come from OS or BIOS. This API will display request information and wait
  for user confirmation if TPM request exists. The TPM request will be sent to TPM device after
  the TPM request is confirmed, and one or more reset may be required to make TPM request to
  take effect.

  This API should be invoked after console in and console out are all ready as they are required
  to display request information and get user input to confirm the request.

  @param  PlatformAuth                   platform auth value. NULL means no platform auth change.
**/
typedef
VOID
(EFIAPI *TCG2_PHYSICAL_PRESENCE_PROCESS_REQUEST) (
  IN      TPM2B_AUTH                     *PlatformAuth  OPTIONAL
  );

/**
  Check if the pending TPM request needs user input to confirm.

  The TPM request may come from OS. This API will check if TPM request exists and need user
  input to confirmation.

  @retval    TRUE        TPM needs input to confirm user physical presence.
  @retval    FALSE       TPM doesn't need input to confirm user physical presence.

**/
typedef
BOOLEAN
(EFIAPI *TCG2_PHYSICAL_PRESENCE_NEED_USER_CONFIRM) (
  VOID
  );

/**
  Return TPM2 ManagementFlags set by PP interface.

  @retval    ManagementFlags    TPM2 Management Flags.
**/
typedef
UINT32
(EFIAPI *TCG2_PHYSICAL_PRESENCE_GET_MANAGEMENT_FLAGS) (
  VOID
  );

/**
  The handler for TPM physical presence function:
  Return TPM Operation Response to OS Environment.

  This API should be invoked in OS runtime phase to interface with ACPI method.

  @param[out]     MostRecentRequest Most recent operation request.
  @param[out]     Response          Response to the most recent operation request.

  @return Return Code for Return TPM Operation Response to OS Environment.
**/
typedef
UINT32
(EFIAPI *TCG2_PHYSICAL_PRESENCE_RETURN_OPERATION_RESPONSE_TO_OS) (
  OUT UINT32                *MostRecentRequest,
  OUT UINT32                *Response
  );

/**
  The handler for TPM physical presence function:
  Submit TPM Operation Request to Pre-OS Environment and
  Submit TPM Operation Request to Pre-OS Environment 2.

  This API should be invoked in OS runtime phase to interface with ACPI method.

  Caution: This function may receive untrusted input.

  @param[in, out]  Pointer to OperationRequest TPM physical presence operation request.
  @param[in, out]  Pointer to RequestParameter TPM physical presence operation request parameter.

  @return Return Code for Submit TPM Operation Request to Pre-OS Environment and
        Submit TPM Operation Request to Pre-OS Environment 2.
  **/
typedef
UINT32
(EFIAPI *TCG2_PHYSICAL_PRESENCE_SUBMIT_REQUEST_TO_PREOS_EX) (
  IN OUT UINT32               *OperationRequest,
  IN OUT UINT32               *RequestParameter
  );

/**
  The handler for TPM physical presence function:
  Submit TPM Operation Request to Pre-OS Environment and
  Submit TPM Operation Request to Pre-OS Environment 2.

  This API should be invoked in OS runtime phase to interface with ACPI method.

  Caution: This function may receive untrusted input.

  @param[in]      OperationRequest TPM physical presence operation request.
  @param[in]      RequestParameter TPM physical presence operation request parameter.

  @return Return Code for Submit TPM Operation Request to Pre-OS Environment and
          Submit TPM Operation Request to Pre-OS Environment 2.
**/
typedef
UINT32
(EFIAPI *TCG2_PHYSICAL_PRESENCE_SUBMIT_REQUEST_TO_PREOS) (
  IN UINT32                 OperationRequest,
  IN UINT32                 RequestParameter
  );

/**
  Return TPM2 ManagementFlags set by PP interface.

  @retval    ManagementFlags    TPM2 Management Flags.
**/
typedef
UINT32
(EFIAPI *TCG2_PHYSICAL_PRESENCE_GET_MANAGEMENT_FLAGS) (
  VOID
  );

struct _EFI_TCG2_PHYSICALPRESENCE_PROTOCOL {
  TCG2_PHYSICAL_PRESENCE_PROCESS_REQUEST                     ProcessRequest;
  TCG2_PHYSICAL_PRESENCE_NEED_USER_CONFIRM                   NeedUserConfirm;
  TCG2_PHYSICAL_PRESENCE_GET_MANAGEMENT_FLAGS                GetManagementFlags;
  TCG2_PHYSICAL_PRESENCE_RETURN_OPERATION_RESPONSE_TO_OS     ReturnOperationResponseToOs;
  TCG2_PHYSICAL_PRESENCE_SUBMIT_REQUEST_TO_PREOS             SubmitRequestToPreOs;
};

extern EFI_GUID gEdkiiPhysicalPresenceProtocolGuid;

#endif
