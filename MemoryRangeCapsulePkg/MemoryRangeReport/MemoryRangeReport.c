/** @file
  This driver will register two callbacks to call fsp's notifies.

  Copyright (c) 2014 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Guid/MemoryRangeCapsule.h>

UINT64
CalcMemoryMap (
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize
  )
{
  EFI_MEMORY_DESCRIPTOR *Entry;
  UINTN                 NumberOfEntries;
  UINTN                 Index;
  UINT64                Pages;

  Pages = 0;
  Entry = MemoryMap;
  NumberOfEntries = MemoryMapSize / DescriptorSize;
  for (Index = 0; Index < NumberOfEntries; Index++) {
    if (Entry->Type < EfiMaxMemoryType &&
        Entry->Type != EfiConventionalMemory &&
        Entry->Type != EfiMemoryMappedIO &&
        Entry->Type != EfiMemoryMappedIOPortSpace &&
        Entry->Type != EfiPalCode) {
      Pages += Entry->NumberOfPages;
    }
    Entry = NEXT_MEMORY_DESCRIPTOR (Entry, DescriptorSize);
  }
  DEBUG ((DEBUG_INFO, "Total : %14ld Pages (%ld Bytes)\n", Pages, MultU64x64(SIZE_4KB, Pages)));

  return Pages;
}

/**
  Notification function of EVT_GROUP_READY_TO_BOOT event group.

  This is a notification function registered on EVT_GROUP_READY_TO_BOOT event group.
  When the Boot Manager is about to load and execute a boot option, it reclaims variable
  storage if free size is below the threshold.

  @param[in] Event        Event whose notification function is being invoked.
  @param[in] Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
OnReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS            Status;
  UINTN                 MapKey;
  UINT32                DescriptorVersion;
  EFI_MEMORY_DESCRIPTOR *MemoryMap;
  UINTN                 UefiMemoryMapSize;
  UINTN                 UefiDescriptorSize;
  EFI_MEMORY_RANGE_REQUIREMENT_DATA  *RangeRequirementData;
  
  gBS->CloseEvent (Event);

  Status = gBS->AllocatePool (EfiBootServicesData, sizeof(EFI_MEMORY_RANGE_REQUIREMENT_DATA), &RangeRequirementData);
  ASSERT (RangeRequirementData != NULL);
  if (RangeRequirementData == NULL) {
    return ;
  }

  UefiMemoryMapSize = 0;
  MemoryMap = NULL;
  Status = gBS->GetMemoryMap (
                  &UefiMemoryMapSize,
                  MemoryMap,
                  &MapKey,
                  &UefiDescriptorSize,
                  &DescriptorVersion
                  );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  do {
    Status = gBS->AllocatePool (EfiBootServicesData, UefiMemoryMapSize, (VOID **)&MemoryMap);
    ASSERT (MemoryMap != NULL);
    if (MemoryMap == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      return ;
    }

    Status = gBS->GetMemoryMap (
                    &UefiMemoryMapSize,
                    MemoryMap,
                    &MapKey,
                    &UefiDescriptorSize,
                    &DescriptorVersion
                    );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (MemoryMap);
      MemoryMap = NULL;
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  if (MemoryMap == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    return ;
  }
  
  RangeRequirementData->FirmwareMemoryRequirement = CalcMemoryMap (MemoryMap, UefiMemoryMapSize, UefiDescriptorSize);

  gBS->FreePool (MemoryMap);

  RangeRequirementData->FirmwareMemoryRequirement += PcdGet64 (PcdMemoryRangeCapsuleAdditionalPages);

  //
  //
  //
  Status = gBS->InstallConfigurationTable (&gEfiMemoryRangeRequirementDataGuid, RangeRequirementData);
  ASSERT_EFI_ERROR (Status);
  
  return ;
}

/**
  Initialize  DXE Platform.

  @param[in] ImageHandle       Image handle of this driver.
  @param[in] SystemTable       Global system service table.

  @retval EFI_SUCCESS           Initialization complete.
  @exception EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  @retval EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  @retval EFI_DEVICE_ERROR      Device error, driver exits abnormally.
**/
EFI_STATUS
EFIAPI
MemoryRangeReportEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_EVENT  ReadyToBootEvent;

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             OnReadyToBoot,
             NULL,
             &ReadyToBootEvent
             );
  ASSERT_EFI_ERROR (Status);
  
  return EFI_SUCCESS;
}