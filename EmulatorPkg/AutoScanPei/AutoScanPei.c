/*++ @file

Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
Portions copyright (c) 2011, Apple Inc. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PiPei.h"
#include <Ppi/EmuThunk.h>
#include <Ppi/MemoryDiscovered.h>
#include <Guid/SmramMemoryReserve.h>

#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>

EFI_STATUS
EFIAPI
PeimInitializeAutoScanPei (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:
  Perform a call-back into the SEC simulator to get a memory value

Arguments:
  FfsHeader   - General purpose data available to every PEIM
  PeiServices - General purpose services available to every PEIM.

Returns:
  None

**/
{
  EFI_STATUS                  Status;
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  EMU_THUNK_PPI               *Thunk;
  UINT64                      MemorySize;
  EFI_PHYSICAL_ADDRESS        MemoryBase;
  UINTN                       Index;
  EFI_RESOURCE_ATTRIBUTE_TYPE Attributes;
  UINT64                                SmramMemorySize;
  EFI_PHYSICAL_ADDRESS                  SmramMemoryBase;
  EFI_SMRAM_HOB_DESCRIPTOR_BLOCK        *SmramHobDescriptorBlock;


  DEBUG ((EFI_D_ERROR, "Emu Autoscan PEIM Loaded\n"));

  //
  // Get the PEI UNIX Autoscan PPI
  //
  Status = PeiServicesLocatePpi (
             &gEmuThunkPpiGuid,      // GUID
             0,                      // INSTANCE
             &PpiDescriptor,         // EFI_PEI_PPI_DESCRIPTOR
             (VOID **)&Thunk         // PPI
             );
  ASSERT_EFI_ERROR (Status);

  Index = 0;
  SmramMemorySize = 0;
  SmramMemoryBase = 0;
  do {
    Status = Thunk->MemoryAutoScan (Index, &MemoryBase, &MemorySize);
    DEBUG ((EFI_D_ERROR, "AutoScan(%d) Status - %r\n", Index, Status));
    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AutoScan(%d) Base - 0x%lx\n", Index, MemoryBase));
      DEBUG ((EFI_D_ERROR, "AutoScan(%d) Size - 0x%lx\n", Index, MemorySize));

      Attributes =
        (
          EFI_RESOURCE_ATTRIBUTE_PRESENT |
          EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
          EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
          EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
          EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
          EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE
        );

      if (Index == 0) {

        if (FeaturePcdGet(PcdEmuSmmEnable)) {
          //
          // SMRAM
          //
          SmramMemorySize = PcdGet64(PcdEmuSmramSize);
          SmramMemoryBase = MemoryBase + MemorySize - SmramMemorySize;
          DEBUG ((EFI_D_ERROR, "SmramMemoryBase - 0x%lx\n", SmramMemoryBase));
          DEBUG ((EFI_D_ERROR, "SmramMemorySize - 0x%lx\n", SmramMemorySize));

          MemorySize      = MemorySize - SmramMemorySize;
        }

        //
        // Register the memory with the PEI Core
        //
        Status = PeiServicesInstallPeiMemory (MemoryBase, MemorySize);
        ASSERT_EFI_ERROR (Status);

        Attributes |= EFI_RESOURCE_ATTRIBUTE_TESTED;
      }

      BuildResourceDescriptorHob (
        EFI_RESOURCE_SYSTEM_MEMORY,
        Attributes,
        MemoryBase,
        MemorySize
        );
    }
    Index++;
  } while (!EFI_ERROR (Status));

  //
  // Build the CPU hob with 57-bit addressing and 16-bits of IO space.
  //
  BuildCpuHob (57, 16);


  if (FeaturePcdGet(PcdEmuSmmEnable)) {
    //
    // BuildSmramHob
    //
    if ((SmramMemoryBase != 0) && (SmramMemorySize != 0)) {
      SmramHobDescriptorBlock = BuildGuidHob (
                                  &gEfiSmmSmramMemoryGuid,
                                  sizeof (EFI_SMRAM_HOB_DESCRIPTOR_BLOCK) + sizeof (EFI_SMRAM_DESCRIPTOR)
                                  );
      ASSERT (SmramHobDescriptorBlock != NULL);
      SmramHobDescriptorBlock->NumberOfSmmReservedRegions = 1;

      SmramHobDescriptorBlock->Descriptor[0].PhysicalStart = SmramMemoryBase;
      SmramHobDescriptorBlock->Descriptor[0].CpuStart      = SmramMemoryBase;
      SmramHobDescriptorBlock->Descriptor[0].PhysicalSize  = SmramMemorySize;
      SmramHobDescriptorBlock->Descriptor[0].RegionState   = EFI_SMRAM_CLOSED;
    }
  }

  return Status;
}
