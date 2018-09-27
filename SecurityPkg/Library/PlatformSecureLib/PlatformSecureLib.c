/** @file
  NULL PlatformSecureLib instance does NOT really detect whether a physical present
  user exists but return TRUE directly. This instance can be used to verify security
  related features during platform enabling and development. It should be replaced
  by a platform-specific method(e.g. Button pressed) in a real platform for product.

Copyright (c) 2011 - 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Protocol/UserPhysicalPresence.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

BOOLEAN       mUserPhysicalPresence      = FALSE;

/**

  This function provides a platform-specific method to detect whether the platform
  is operating by a physically present user.

  Programmatic changing of platform security policy (such as disable Secure Boot,
  or switch between Standard/Custom Secure Boot mode) MUST NOT be possible during
  Boot Services or after exiting EFI Boot Services. Only a physically present user
  is allowed to perform these operations.

  NOTE THAT: This function cannot depend on any EFI Variable Service since they are
  not available when this function is called in AuthenticateVariable driver.

  @retval  TRUE       The platform is operated by a physically present user.
  @retval  FALSE      The platform is NOT operated by a physically present user.

**/
BOOLEAN
EFIAPI
UserPhysicalPresent (
  VOID
  )
{
  return mUserPhysicalPresence;
}

/**
  Notify Function in which convert EFI1.0 PassThru Packet back to UEF2.0
  SCSI IO Packet.

  @param  Event    The instance of EFI_EVENT.
  @param  Context  The parameter passed in.

**/
VOID
EFIAPI
NotifyFunction (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  EFI_STATUS                      Status;
  USER_PHYSICAL_PRESENCE_PROTOCOL *UserPPProtocol;

  gBS->CloseEvent(Event);

  Status = gBS->LocateProtocol (&gEdkiiUserPhysicalPresenceProtocolGuid, NULL, (VOID **) &UserPPProtocol);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Update PhysicalPresence value when User Physical Presece Protocol installed
  //
  mUserPhysicalPresence = UserPPProtocol->UserPhysicalPresent();
}

/**
  Save user physical presence state from a PCD to mUserPhysicalPresence.

  @param  ImageHandle   ImageHandle of the loaded driver.
  @param  SystemTable   Pointer to the EFI System Table.

  @retval  EFI_SUCCESS          PcdUserPhysicalPresence is got successfully.

**/
EFI_STATUS
EFIAPI
PlatformSecureLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_EVENT   Event;
  EFI_STATUS  Status;
  VOID        *Registration;

  //
  // Use PCD as intial value
  //
  mUserPhysicalPresence = PcdGetBool(PcdUserPhysicalPresence);

  //
  // Create the event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  NotifyFunction,
                  NULL,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);
   
  //
  // Register for protocol notifactions on this event
  //
  Status = gBS->RegisterProtocolNotify (
                  &gEdkiiUserPhysicalPresenceProtocolGuid,
                  Event,
                  &Registration
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
