/** @file
  CPU DXE Module to produce CPU ARCH Protocol.

  Copyright (c) 2008 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Protocol/MemoryAttribute.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

/**
  Initialize the state information for the CPU Architectural Protocol.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to the System Table.

  @retval EFI_SUCCESS           Thread can be successfully created
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Cannot create the thread

**/
EFI_STATUS
EFIAPI
InitializeCpuDxeTest (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_MEMORY_ATTRIBUTE_PROTOCOL  *MemoryAttribute;
  EFI_PHYSICAL_ADDRESS           BaseAddress;
  UINT64                         Length;
  UINT64                         Attributes;
  UINT64                         OldAttributes;

  Status = gBS->LocateProtocol (&gEfiMemoryAttributeProtocolGuid, NULL, (VOID **)&MemoryAttribute);
  ASSERT_EFI_ERROR (Status);

  BaseAddress = (UINTN)InitializeCpuDxeTest & ~(SIZE_4KB - 1);
  Length = SIZE_4KB;
  Attributes = 0;

  Status = MemoryAttribute->GetMemoryAttributes (MemoryAttribute, BaseAddress, Length, &OldAttributes);
  ASSERT_EFI_ERROR (Status);

  Status = MemoryAttribute->ClearMemoryAttributes (MemoryAttribute, BaseAddress, Length, OldAttributes);
  ASSERT_EFI_ERROR (Status);

  Status = MemoryAttribute->GetMemoryAttributes (MemoryAttribute, BaseAddress, Length, &Attributes);
  ASSERT_EFI_ERROR (Status);

  Status = MemoryAttribute->SetMemoryAttributes (MemoryAttribute, BaseAddress, Length, OldAttributes);
  ASSERT_EFI_ERROR (Status);

  Status = MemoryAttribute->GetMemoryAttributes (MemoryAttribute, BaseAddress, Length, &Attributes);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
