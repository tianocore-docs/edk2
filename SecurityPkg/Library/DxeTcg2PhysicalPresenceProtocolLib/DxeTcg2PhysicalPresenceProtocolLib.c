/** @file
  Execute pending TPM2 requests from OS or BIOS.

  Caution: This module requires additional review when modified.
  This driver will have external input - variable.
  This external input must be validated carefully to avoid security issue.

  Tpm2ExecutePendingTpmRequest() will receive untrusted input and do validation.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Protocol/Tcg2PhysicalPresenceProtocol.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>

TCG2_PHYSICAL_PRESENCE_PROTOCOL  *mTcg2PhysicalPresenceProtocol;

TCG2_PHYSICAL_PRESENCE_PROTOCOL *
EFIAPI
Tcg2CheckPhysicalPresenceProtocol (
  VOID
  )
{
  EFI_STATUS                Status;

  if (mTcg2PhysicalPresenceProtocol == NULL) {
    Status = gBS->LocateProtocol (&gEdkiiTcg2PhysicalPresenceProtocolGuid, NULL, (VOID **) &mTcg2PhysicalPresenceProtocol);
    if (EFI_ERROR (Status)) {
      //
      // Tcg2 protocol is not installed. So, TPM2 is not present.
      //
      DEBUG ((EFI_D_ERROR, "Tcg2CheckPhysicalPresenceProtocol - Locate Tcg2 PhysicalPrensence Protocol - %r\n", Status));
      return NULL;
    }
  }

  return mTcg2PhysicalPresenceProtocol;
}

/**
  Check and execute the pending TPM request.

  The TPM request may come from OS or BIOS. This API will display request information and wait
  for user confirmation if TPM request exists. The TPM request will be sent to TPM device after
  the TPM request is confirmed, and one or more reset may be required to make TPM request to
  take effect.

  This API should be invoked after console in and console out are all ready as they are required
  to display request information and get user input to confirm the request.

  @param[in]  PlatformAuth                   platform auth value. NULL means no platform auth change.
**/
VOID
EFIAPI
Tcg2PhysicalPresenceLibProcessRequest (
  IN      TPM2B_AUTH                     *PlatformAuth  OPTIONAL
  )
{
  TCG2_PHYSICAL_PRESENCE_PROTOCOL  *Tcg2PhysicalPresenceProtocol;

  Tcg2PhysicalPresenceProtocol = Tcg2CheckPhysicalPresenceProtocol();
  if (Tcg2PhysicalPresenceProtocol != NULL) {
    Tcg2PhysicalPresenceProtocol->ProcessRequest(PlatformAuth);
  }
}

/**
  Check if the pending TPM request needs user input to confirm.

  The TPM request may come from OS. This API will check if TPM request exists and need user
  input to confirmation.

  @retval    TRUE        TPM needs input to confirm user physical presence.
  @retval    FALSE       TPM doesn't need input to confirm user physical presence.

**/
BOOLEAN
EFIAPI
Tcg2PhysicalPresenceLibNeedUserConfirm(
  VOID
  )
{
  TCG2_PHYSICAL_PRESENCE_PROTOCOL  *Tcg2PhysicalPresenceProtocol;

  Tcg2PhysicalPresenceProtocol = Tcg2CheckPhysicalPresenceProtocol();
  if (Tcg2PhysicalPresenceProtocol != NULL) {
    return Tcg2PhysicalPresenceProtocol->NeedUserConfirm();
  }

  return FALSE;

}


/**
  The handler for TPM physical presence function:
  Return TPM Operation Response to OS Environment.

  @param[out]     MostRecentRequest Most recent operation request.
  @param[out]     Response          Response to the most recent operation request.

  @return Return Code for Return TPM Operation Response to OS Environment.
**/
UINT32
EFIAPI
Tcg2PhysicalPresenceLibReturnOperationResponseToOsFunction (
  OUT UINT32                *MostRecentRequest,
  OUT UINT32                *Response
  )
{ 
  TCG2_PHYSICAL_PRESENCE_PROTOCOL  *Tcg2PhysicalPresenceProtocol;

  Tcg2PhysicalPresenceProtocol = Tcg2CheckPhysicalPresenceProtocol();
  if (Tcg2PhysicalPresenceProtocol != NULL) {
    return Tcg2PhysicalPresenceProtocol->ReturnOperationResponseToOs(MostRecentRequest, Response);
  }

  return 0;
}

/**
  The handler for TPM physical presence function:
  Submit TPM Operation Request to Pre-OS Environment and
  Submit TPM Operation Request to Pre-OS Environment 2.

  Caution: This function may receive untrusted input.

  @param[in]      OperationRequest TPM physical presence operation request.
  @param[in]      RequestParameter TPM physical presence operation request parameter.

  @return Return Code for Submit TPM Operation Request to Pre-OS Environment and
          Submit TPM Operation Request to Pre-OS Environment 2.
**/
UINT32
EFIAPI
Tcg2PhysicalPresenceLibSubmitRequestToPreOSFunction (
  IN UINT32                 OperationRequest,
  IN UINT32                 RequestParameter
  )
{
  TCG2_PHYSICAL_PRESENCE_PROTOCOL  *Tcg2PhysicalPresenceProtocol;

  Tcg2PhysicalPresenceProtocol = Tcg2CheckPhysicalPresenceProtocol();
  if (Tcg2PhysicalPresenceProtocol != NULL) {
    return Tcg2PhysicalPresenceProtocol->SubmitRequestToPreOs(OperationRequest, RequestParameter);
  }

  return 0;
}


/**
  Return TPM2 ManagementFlags set by PP interface.

  @retval    ManagementFlags    TPM2 Management Flags.
**/
UINT32
EFIAPI
Tcg2PhysicalPresenceLibGetManagementFlags (
  VOID
  )
{
  TCG2_PHYSICAL_PRESENCE_PROTOCOL  *Tcg2PhysicalPresenceProtocol;

  Tcg2PhysicalPresenceProtocol = Tcg2CheckPhysicalPresenceProtocol();
  if (Tcg2PhysicalPresenceProtocol != NULL) {
    return Tcg2PhysicalPresenceProtocol->GetManagementFlags();
  }

  return 0;
}
