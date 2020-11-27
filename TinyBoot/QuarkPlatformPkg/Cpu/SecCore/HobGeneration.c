/** @file

Copyright(c) 2014 Intel Corporation. All rights reserved.

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


**/

#include "SecMain.h"
#include <Guid/MemoryAllocationHob.h>

#define DXE_CORE_FILE_NAME_GUID \
  { 0xD6A2CB7F, 0x6A18, 0x4e2f, {0xB4, 0x3B, 0x99, 0x20, 0xA7, 0x33, 0x70, 0x0A }}

/*
--------------------------------------------------------
 Memory Map:
--------------------------------------------------------
0x0
0xA0000
0x100000 (1M)
        Temporary Stack (1M)
0x200000

MemoryAbove1MB.PhysicalStart <-----------------------------------------------------+
        ...                                                                        |
        ...                                                                        |
                        <- Phit.EfiMemoryBottom -------------------+               |
        HOB                                                        |               |
                        <- Phit.EfiFreeMemoryBottom                |               |
                                                                   |     MemoryFreeAbove1MB.ResourceLength
                        <- Phit.EfiFreeMemoryTop ------+           |               |
                                                       |    4M = CONSUMED_MEMORY   |
                                                       |           |               |
                                                MemoryAllocation   |               |
                                                       |           |               |
                        <------------------------------+           |               |
        Permament Stack (0x20 Pages = 128K)            |           |               |
                        <- Phit.EfiMemoryTop ----------+-----------+---------------+<---------+
        DxeCore                                                                    |          |
                                                                                DxeCore       |
                                                                                   |   Allocated in SecIpl
                        <----------------------------------------------------------+          |
        BFV                                                                      MMIO         |
                        <- Top of Free Memory reported         --------------------+<---------+
        Reserved
                        <- Memory Top on RealMemory
*/


#define EFI_MEMORY_STACK_PAGE_NUM      0x20
#define CONSUMED_MEMORY                0x100000 * 80

#pragma pack(1)

typedef struct {
  EFI_HOB_HANDOFF_INFO_TABLE        Phit;
  EFI_HOB_FIRMWARE_VOLUME           Bfv;
  EFI_HOB_RESOURCE_DESCRIPTOR       BfvResource;
  EFI_HOB_CPU                       Cpu;
  EFI_HOB_MEMORY_ALLOCATION_STACK   Stack;
  EFI_HOB_MEMORY_ALLOCATION         MemoryAllocation;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryBelow1MB;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryABSeg;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryExBiosReg;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryAbove1MB;
  EFI_HOB_MEMORY_ALLOCATION_MODULE  DxeCore;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryDxeCore;
  EFI_HOB_GENERIC_HEADER            EndOfHobList;
} HOB_TEMPLATE;

#pragma pack()


#define EFI_CPUID_EXTENDED_FUNCTION  0x80000000
#define CPUID_EXTENDED_ADD_SIZE      0x80000008

HOB_TEMPLATE  gHobTemplate = {
  { // Phit
    {  // Header
      EFI_HOB_TYPE_HANDOFF,                 // HobType
      sizeof (EFI_HOB_HANDOFF_INFO_TABLE),  // HobLength
      0                                     // Reserved
    },
    EFI_HOB_HANDOFF_TABLE_VERSION,          // Version
    BOOT_WITH_FULL_CONFIGURATION,           // BootMode
    0,                                      // EfiMemoryTop
    0,                                      // EfiMemoryBottom
    0,                                      // EfiFreeMemoryTop
    0,                                      // EfiFreeMemoryBottom
    0                                       // EfiEndOfHobList
  }, 
  { // Bfv
    {
      EFI_HOB_TYPE_FV,                      // HobType
      sizeof (EFI_HOB_FIRMWARE_VOLUME),     // HobLength
      0                                     // Reserved
    },
    0,                                      // BaseAddress
    0                                       // Length
  },
  { // BfvResource
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,     // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR), // HobLength
      0                                     // Reserved
    },
    {
      0                                     // Owner Guid
    },
    EFI_RESOURCE_FIRMWARE_DEVICE,           // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT    |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
     EFI_RESOURCE_ATTRIBUTE_TESTED |
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),  // ResourceAttribute
    0,                                              // PhysicalStart
    0                                               // ResourceLength
  },
  { // Cpu
    { // Header
      EFI_HOB_TYPE_CPU,                     // HobType
      sizeof (EFI_HOB_CPU),                 // HobLength
      0                                     // Reserved
    },
    52,                                     // SizeOfMemorySpace - Architecture Max
    16,                                     // SizeOfIoSpace,
    {
      0, 0, 0, 0, 0, 0                      // Reserved[6]
    }
  },
  {   // Stack HOB
    {   // header
      EFI_HOB_TYPE_MEMORY_ALLOCATION,               // Hob type
      sizeof (EFI_HOB_MEMORY_ALLOCATION_STACK),     // Hob size
      0                                             // reserved
    },
    {
      EFI_HOB_MEMORY_ALLOC_STACK_GUID,
      0x0,                                          // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                          // UINT64                MemoryLength;
      EfiBootServicesData,                          // EFI_MEMORY_TYPE       MemoryType;  
      {0, 0, 0, 0}                                  // Reserved              Reserved[4]; 
    }
  },
  { // MemoryAllocation for HOB's & Images
    {
      EFI_HOB_TYPE_MEMORY_ALLOCATION,               // HobType
      sizeof (EFI_HOB_MEMORY_ALLOCATION),           // HobLength
      0                                             // Reserved
    },
    {
      {
        0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID       // Name
      },
      0x0,                                          // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                          // UINT64                MemoryLength;
      EfiBootServicesData,                          // EFI_MEMORY_TYPE       MemoryType;  
      {
        0, 0, 0, 0                                  // Reserved              Reserved[4]; 
      }
    }
   },
  { // MemoryFreeBelow1MB for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,                     // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_TESTED                  |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0x0,                                            // PhysicalStart
    0xA0000                                         // ResourceLength
  },
  { // MemoryFreeABSeg for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_MEMORY_RESERVED,                   // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0xA0000,                                        // PhysicalStart
    0x20000                                         // ResourceLength
  },
  { // MemoryFreeExBiosReg for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_MEMORY_RESERVED,                   // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0xC0000,                                        // PhysicalStart
    0x40000                                         // ResourceLength
  },
  { // MemoryFreeAbove1MB for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,                     // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_TESTED                  |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0x0,                                            // PhysicalStart
    0                                               // ResourceLength
  },
  {   // Memory Allocation Module for DxeCore
    {   // header
      EFI_HOB_TYPE_MEMORY_ALLOCATION,               // Hob type
      sizeof (EFI_HOB_MEMORY_ALLOCATION_MODULE),    // Hob size
      0                                             // reserved
    },
    {
      EFI_HOB_MEMORY_ALLOC_MODULE_GUID,
      0x0,                                          // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                          // UINT64                MemoryLength;
      EfiBootServicesCode,                          // EFI_MEMORY_TYPE       MemoryType;  
      {
        0, 0, 0, 0                                  // UINT8                 Reserved[4]; 
      },
    },
    DXE_CORE_FILE_NAME_GUID,
    0x0                                             //  EFI_PHYSICAL_ADDRESS of EntryPoint;
  },
  { // MemoryDxeCore
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,                     // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
//     EFI_RESOURCE_ATTRIBUTE_TESTED                  | // Do not mark as TESTED, or DxeCore will find it and use it before check Allocation
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0x0,                                            // PhysicalStart
    0                                               // ResourceLength
  },
  { // EndOfHobList
    EFI_HOB_TYPE_END_OF_HOB_LIST,      // HobType
    sizeof (EFI_HOB_GENERIC_HEADER),   // HobLength
    0                                  // Reserved
  }
};

HOB_TEMPLATE  *gHob = &gHobTemplate;

VOID *
PrepareHobMemory (
  IN EFI_PHYSICAL_ADDRESS   Base,
  IN UINT64                 Size
  )
/*++
Description:
  Update the Hob filling MemoryFreeUnder1MB, MemoryAbove1MB, MemoryAbove4GB

Arguments:

Return:
  VOID * : The end address of MemoryAbove1MB (or the top free memory under 4GB)
--*/
{
  //
  // Prepare High Memory
  //
  gHob->MemoryAbove1MB.PhysicalStart   = Base;
  gHob->MemoryAbove1MB.ResourceLength  = Size;

  return (VOID *)(UINTN)(gHob->MemoryAbove1MB.PhysicalStart + gHob->MemoryAbove1MB.ResourceLength);
}

VOID *
PrepareHobStack (
  IN VOID *StackTop
  )
{
  gHob->Stack.AllocDescriptor.MemoryLength      = EFI_MEMORY_STACK_PAGE_NUM * EFI_PAGE_SIZE;
  gHob->Stack.AllocDescriptor.MemoryBaseAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)StackTop - gHob->Stack.AllocDescriptor.MemoryLength;

  return (VOID *)(UINTN)gHob->Stack.AllocDescriptor.MemoryBaseAddress;
}

VOID
PrepareHobBfv (
  VOID  *Bfv,
  UINTN BfvLength
  )
{
  //UINTN BfvLengthPageSize;

  //
  // Calculate BFV location at top of the memory region.
  // This is like a RAM Disk. Align to page boundry.
  //
  //BfvLengthPageSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (BfvLength));
 
  gHob->Bfv.BaseAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)Bfv;
  gHob->Bfv.Length = BfvLength;

  //
  // Resource descriptor for the FV
  //
  gHob->BfvResource.PhysicalStart = gHob->Bfv.BaseAddress;
  gHob->BfvResource.ResourceLength = gHob->Bfv.Length;
}

VOID
PrepareHobDxeCore (
  VOID                  *DxeCoreEntryPoint,
  EFI_PHYSICAL_ADDRESS  DxeCoreImageBase,
  UINT64                DxeCoreLength
  )
{
  gHob->DxeCore.MemoryAllocationHeader.MemoryBaseAddress = DxeCoreImageBase;
  gHob->DxeCore.MemoryAllocationHeader.MemoryLength = DxeCoreLength;
  gHob->DxeCore.EntryPoint = (EFI_PHYSICAL_ADDRESS)(UINTN)DxeCoreEntryPoint;


  gHob->MemoryDxeCore.PhysicalStart   = DxeCoreImageBase;
  gHob->MemoryDxeCore.ResourceLength  = DxeCoreLength;  
}

VOID
PrepareHobPhit (
  VOID *MemoryTop,
  VOID *FreeMemoryTop
  )
{
  gHob->Phit.EfiMemoryTop        = (EFI_PHYSICAL_ADDRESS)(UINTN)MemoryTop;
  gHob->Phit.EfiMemoryBottom     = gHob->Phit.EfiMemoryTop - CONSUMED_MEMORY;
  gHob->Phit.EfiFreeMemoryTop    = (EFI_PHYSICAL_ADDRESS)(UINTN)FreeMemoryTop;
  gHob->Phit.EfiFreeMemoryBottom = gHob->Phit.EfiMemoryBottom + sizeof(HOB_TEMPLATE);

  CopyMem ((VOID *)(UINTN)gHob->Phit.EfiMemoryBottom, gHob, sizeof(HOB_TEMPLATE));
  gHob = (HOB_TEMPLATE *)(UINTN)gHob->Phit.EfiMemoryBottom;

  gHob->Phit.EfiEndOfHobList = (EFI_PHYSICAL_ADDRESS)(UINTN)&gHob->EndOfHobList;
}

VOID
PrepareHobCpu (
  VOID
  )
{
  UINT32  CpuidEax;

  //
  // Create a CPU hand-off information
  //
  gHob->Cpu.SizeOfMemorySpace = 36;

  AsmCpuid (EFI_CPUID_EXTENDED_FUNCTION, &CpuidEax, NULL, NULL, NULL);
  if (CpuidEax >= CPUID_EXTENDED_ADD_SIZE) {
    AsmCpuid (CPUID_EXTENDED_ADD_SIZE, &CpuidEax, NULL, NULL, NULL);
    gHob->Cpu.SizeOfMemorySpace = (UINT8)(CpuidEax & 0xFF);
  }
}

VOID
CompleteHobGeneration (
  VOID
  )
{
  gHob->MemoryAllocation.AllocDescriptor.MemoryBaseAddress  = gHob->Phit.EfiFreeMemoryTop;
  //
  // Reserve all the memory under Stack above FreeMemoryTop as allocated
  //
  gHob->MemoryAllocation.AllocDescriptor.MemoryLength       = gHob->Stack.AllocDescriptor.MemoryBaseAddress - gHob->Phit.EfiFreeMemoryTop;

  //
  // adjust Above1MB ResourceLength
  //
  if (gHob->MemoryAbove1MB.PhysicalStart + gHob->MemoryAbove1MB.ResourceLength > gHob->Phit.EfiMemoryTop) {
    gHob->MemoryAbove1MB.ResourceLength = gHob->Phit.EfiMemoryTop - gHob->MemoryAbove1MB.PhysicalStart;
  }
}

VOID
PrintHobInfo (
  VOID
  )
{
  //
  // Print Hob Info
  //
  DEBUG_WRITE_STRING ("*** Hob ***\n");
  DEBUG_WRITE_STRING ("HobStart = ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)gHob);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("Memory Top = ");
  DEBUG_WRITE_UINT64 (gHob->Phit.EfiMemoryTop);
  DEBUG_WRITE_STRING (", Bottom = ");
  DEBUG_WRITE_UINT64 (gHob->Phit.EfiMemoryBottom);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("Free Memory Top = ");
  DEBUG_WRITE_UINT64 (gHob->Phit.EfiFreeMemoryTop);
  DEBUG_WRITE_STRING (", Bottom = ");
  DEBUG_WRITE_UINT64 (gHob->Phit.EfiFreeMemoryBottom);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("BfvResource = ");
  DEBUG_WRITE_UINT64 (gHob->BfvResource.PhysicalStart);
  DEBUG_WRITE_STRING (", Length = ");
  DEBUG_WRITE_UINT64 (gHob->BfvResource.ResourceLength);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("Stack = ");
  DEBUG_WRITE_UINT64 (gHob->Stack.AllocDescriptor.MemoryBaseAddress);
  DEBUG_WRITE_STRING (", StackLength = ");
  DEBUG_WRITE_UINT64 (gHob->Stack.AllocDescriptor.MemoryLength);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("MemoryAbove1MB = ");
  DEBUG_WRITE_UINT64 (gHob->MemoryAbove1MB.PhysicalStart);
  DEBUG_WRITE_STRING (", MemoryAbove1MBLength = ");
  DEBUG_WRITE_UINT64 (gHob->MemoryAbove1MB.ResourceLength);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("DxeCore = ");
  DEBUG_WRITE_UINT64 (gHob->DxeCore.MemoryAllocationHeader.MemoryBaseAddress);
  DEBUG_WRITE_STRING (", DxeCoreLength = ");
  DEBUG_WRITE_UINT64 (gHob->DxeCore.MemoryAllocationHeader.MemoryLength);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("MemoryAllocation = ");
  DEBUG_WRITE_UINT64 (gHob->MemoryAllocation.AllocDescriptor.MemoryBaseAddress);
  DEBUG_WRITE_STRING (", MemoryLength = ");
  DEBUG_WRITE_UINT64 (gHob->MemoryAllocation.AllocDescriptor.MemoryLength);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("*** Hob ***\n");
}

typedef
VOID
(EFIAPI *EFI_DXE_ENTRY_POINT) (
  IN  VOID *HobStart
  );

VOID *
SecIplEntry (
  IN HAND_OFF_INFO *Handoff
  )
{
  VOID                  *StackTop;
  VOID                  *StackBottom;
  VOID                  *MemoryTopOnDescriptor;
  EFI_DXE_ENTRY_POINT   DxeEntryPoint;
  EFI_FIRMWARE_VOLUME_HEADER       *Fv;

  //
  // Hob Generation Guild line:
  //   * Don't report FV as physical memory
  //   * MemoryAllocation Hob should only cover physical memory
  //   * Use ResourceDescriptor Hob to report physical memory or Firmware Device and they shouldn't be overlapped
  DEBUG_WRITE_STRING ("Prepare Cpu HOB information ...\n");
  PrepareHobCpu ();

  //
  // 1. BFV
  //
  DEBUG_WRITE_STRING ("Prepare BFV HOB information ...\n");
  Fv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)FixedPcdGet32(PcdPlatformBootFvMemBase);
  PrepareHobBfv ((VOID *)(UINTN)FixedPcdGet32(PcdPlatformBootFvMemBase), (UINTN)Fv->FvLength);

  //
  // 2. Updates Memory information, and get the top free address under 4GB
  //
  DEBUG_WRITE_STRING ("Prepare Memory HOB information ...\n");
  MemoryTopOnDescriptor = PrepareHobMemory (0x100000, (UINTN)Handoff->MemoryTop - 0x100000);
  
  //
  // 3. Put [Stack], [HOB] just below the [top free address under 4GB]
  //
  
  //   3.1 Stack
  StackTop = MemoryTopOnDescriptor;
  StackBottom = PrepareHobStack (StackTop);
  //   3.2 Copy the Hob itself to EfiMemoryBottom, and update the PHIT Hob
  PrepareHobPhit (StackTop, StackBottom);

  //
  // 4. Register the memory occupied by DxeCore and DxeIpl together as DxeCore
  //
  DEBUG_WRITE_STRING ("Prepare DxeCore memory Hob ...\n");
  PrepareHobDxeCore (
    Handoff->DxeCoreEntryPoint,
    (EFI_PHYSICAL_ADDRESS)(UINTN)Handoff->DxeCoreImageBase,
    (UINTN)Handoff->DxeCoreLength
    );
  
  CompleteHobGeneration ();

  PrintHobInfo ();

  DxeEntryPoint = (EFI_DXE_ENTRY_POINT)(UINTN)Handoff->DxeCoreEntryPoint;
  if (0) {
    DxeEntryPoint (gHob);
  } else {
    SwitchStack (
      (SWITCH_STACK_ENTRY_POINT)(UINTN)DxeEntryPoint,
      gHob,
      NULL,
      StackTop
      );
  }

  CpuDeadLoop ();

  return NULL;
}