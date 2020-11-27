/** @file
  Serial I/O Port library constructor/destructor

  Copyright(c) 2013 Intel Corporation. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.
  * Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Module Name:  IohSerialPortLib.c

**/

#include <PiPei.h>
#include <IohAccess.h>

#include <Library/SerialPortLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Protocol/SerialIo.h>
#include <IndustryStandard/Pci22.h>
#include "IohSerialPortLib.h"

#include <Guid/EventGroup.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

///
/// Set Virtual Address Map Event
///
EFI_EVENT                               mIohSerialPortLibVirtualNotifyEvent = NULL;

///
/// Module global that contains the base physical address of the PCI Express MMIO range.
///
UINTN                                   mIohSerialPortLibUartMmioBase = 0;

UINT32
EnsureSerialPortInitialize (
  VOID
  );

/**
  Convert the physical PCI Express MMIO address to a virtual address.

  @param[in]    Event   The event that is being processed.
  @param[in]    Context The Event Context.
**/
VOID
EFIAPI
IohSerialPortLibVirtualNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                       Status;

  //
  // Cache the physical address of the PCI Express MMIO range into a module global variable
  //
  mIohSerialPortLibUartMmioBase = EnsureSerialPortInitialize ();

  //
  // Convert the physical PCI Express MMIO address to a virtual address.
  //
  Status = EfiConvertPointer (0, (VOID **) &mIohSerialPortLibUartMmioBase);

  ASSERT_EFI_ERROR (Status);
}

/**
  The constructor function to setup globals and goto virtual mode notify.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor completed successfully.
  @retval Other value   The constructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
IohSerialPortLibRuntimeConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Register SetVirtualAddressMap () notify function
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  IohSerialPortLibVirtualNotify,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mIohSerialPortLibVirtualNotifyEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  The destructor function frees any allocated buffers and closes the Set Virtual 
  Address Map event.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The destructor completed successfully.
  @retval Other value   The destructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
IohSerialPortLibRuntimeDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Close the Set Virtual Address Map event
  //
  Status = gBS->CloseEvent (mIohSerialPortLibVirtualNotifyEvent);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
