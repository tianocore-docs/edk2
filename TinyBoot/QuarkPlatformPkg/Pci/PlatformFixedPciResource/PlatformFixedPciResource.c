/*++

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


--*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <IndustryStandard/Pci.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciEnumerationComplete.h>

typedef struct {
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT32  Bar[6];
  UINT16  Command;
} FIXED_PCI_DEVICE;

typedef struct {
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT32  Bar[2];
  UINT8   PrimaryBusNum;
  UINT8   SecondaryBusNum;
  UINT8   SubordinateBusNum;
  UINT8   IoBase;
  UINT8   IoLimit;
  UINT16  MemoryBase;
  UINT16  MemoryLimit;
  UINT16  PrefetchableMemoryBase;
  UINT16  PrefetchableMemoryLimit;
  UINT32  PrefetchableMemoryBaseUpper;
  UINT32  PrefetchableMemoryLimitUpper;
  UINT16  IoBaseUpper;
  UINT16  IoLimitUpper;
  UINT16  BridgeControl;
  UINT16  Command;
} FIXED_PCI_BRIDGE;

typedef enum {
  FixedSystemResourceMem,
  FixedSystemResourceIo,
} FIXED_SYSTEM_RESOURCE_TYPE;

typedef struct {
  EFI_GCD_MEMORY_TYPE  Type;
  EFI_PHYSICAL_ADDRESS BaseAddress;
  UINT64               Length;
  UINT64               Capabilities;
} FIXED_SYSTEM_RESOURCE;

FIXED_PCI_DEVICE  mFixedPciDevice[] = {
// Bus   Dev   Func  Bar0                                                    Bar1                                                    Bar2        Bar3        Bar4        Bar5        Command
  {0x00, 0x00, 0x00, 0x00000000,                                             0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x00, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00010000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x01, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x0000F000, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x0000E000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x02, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00004000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x03, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x0000D000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x04, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x0000C000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x05, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x0000B000, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x0000A000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x06, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00002000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x14, 0x07, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00000000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x15, 0x00, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00009000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x15, 0x01, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00008000, 0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x15, 0x02, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00007000, FixedPcdGet32(PcdPciHostBridgeMemory32Base)+0x00006000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x1F, 0x00, 0x00000000,                                             0x00000000,                                             0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
};

FIXED_PCI_BRIDGE  mFixedPciBridge[] = {
// Bus   Dev   Func  Bar0        Bar1        PriB  SecB  SubB  IoB   IoL   MemB    MemL    PMemB   PMemL   PMemBU      PMemLU      IoBU    IoLU    BriCtl  Command
  {0x00, 0x17, 0x00, 0x00000000, 0x00000000, 0x00, 0x01, 0x01, 0xF0, 0x00, 0xFFF0, 0x0000, 0xFFF1, 0x0001, 0xFFFFFFFF, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x17, 0x01, 0x00000000, 0x00000000, 0x00, 0x02, 0x02, 0xF0, 0x00, 0xFFF0, 0x0000, 0xFFF1, 0x0001, 0xFFFFFFFF, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
};

FIXED_SYSTEM_RESOURCE  mFixedMmioResource[] = {
// Type                             BaseAddress                                  Length                                       Capabilities
  {EfiGcdMemoryTypeMemoryMappedIo,  FixedPcdGet32(PcdPciHostBridgeMemory32Base), FixedPcdGet32(PcdPciHostBridgeMemory32Size), 0x0},
  {EfiGcdMemoryTypeMemoryMappedIo,  FixedPcdGet32(PcdPciHostBridgeMemory64Base), FixedPcdGet32(PcdPciHostBridgeMemory64Size), 0x0},
};

FIXED_SYSTEM_RESOURCE  mFixedIoResource[] = {
// Type                             BaseAddress Length      Capabilities
  {EfiGcdIoTypeIo,                  0x2000,     0xE000,     0x0},
};

FIXED_SYSTEM_RESOURCE  mUsedMmioResource[] = {
// Type                             BaseAddress                                  Length      Capabilities
  {EfiGcdMemoryTypeMemoryMappedIo,  FixedPcdGet32(PcdPciHostBridgeMemory32Base), FixedPcdGet32(PcdPciHostBridgeMemory32Size), 0x0},
};

FIXED_SYSTEM_RESOURCE  mUsedIoResource[] = {
// Type                             BaseAddress Length      Capabilities
  {EfiGcdIoTypeIo,                  0x0000,     0x0000,     0x0},
};

EFI_STATUS
EFIAPI
FixedPciResourceAssignPciResource (
  VOID
  )
{
  UINTN                Index;
  UINTN                SubIndex;
  UINTN                PciAddress;
  
  //
  // Assign PCI bridge resource
  //
  for (Index = 0; Index < sizeof(mFixedPciBridge)/sizeof(mFixedPciBridge[0]); Index++) {
    PciAddress = PCI_LIB_ADDRESS (
                   mFixedPciBridge[Index].Bus,
                   mFixedPciBridge[Index].Device,
                   mFixedPciBridge[Index].Function,
                   0
                   );

    for (SubIndex = 0; SubIndex < 2; SubIndex++) {
      PciWrite32 (
        PciAddress + PCI_BASE_ADDRESSREG_OFFSET + SubIndex * sizeof(UINT32),
        mFixedPciBridge[Index].Bar[SubIndex]
        );
    }
    PciWrite8 (
      PciAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
      mFixedPciBridge[Index].PrimaryBusNum
      );
    PciWrite8 (
      PciAddress + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET,
      mFixedPciBridge[Index].SecondaryBusNum
      );
    PciWrite8 (
      PciAddress + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET,
      mFixedPciBridge[Index].SubordinateBusNum
      );
    PciWrite8 (
      PciAddress + 0x1C,
      mFixedPciBridge[Index].IoBase
      );
    PciWrite8 (
      PciAddress + 0x1D,
      mFixedPciBridge[Index].IoLimit
      );
    PciWrite16 (
      PciAddress + 0x20,
      mFixedPciBridge[Index].MemoryBase
      );
    PciWrite16 (
      PciAddress + 0x22,
      mFixedPciBridge[Index].MemoryLimit
      );
    PciWrite16 (
      PciAddress + 0x24,
      mFixedPciBridge[Index].PrefetchableMemoryBase
      );
    PciWrite16 (
      PciAddress + 0x26,
      mFixedPciBridge[Index].PrefetchableMemoryLimit
      );
    PciWrite32 (
      PciAddress + 0x28,
      mFixedPciBridge[Index].PrefetchableMemoryBaseUpper
      );
    PciWrite32 (
      PciAddress + 0x2C,
      mFixedPciBridge[Index].PrefetchableMemoryLimitUpper
      );
    PciWrite16 (
      PciAddress + 0x30,
      mFixedPciBridge[Index].IoBaseUpper
      );
    PciWrite16 (
      PciAddress + 0x32,
      mFixedPciBridge[Index].IoLimitUpper
      );
    PciWrite16 (
      PciAddress + 0x3E,
      mFixedPciBridge[Index].BridgeControl
      );
    PciOr16 (
      PciAddress + PCI_COMMAND_OFFSET,
      mFixedPciBridge[Index].Command
      );
  }

  //
  // Assign PCI device resource - need after bridge resource or bus number may be incorrect.
  //
  for (Index = 0; Index < sizeof(mFixedPciDevice)/sizeof(mFixedPciDevice[0]); Index++) {
    PciAddress = PCI_LIB_ADDRESS (
                   mFixedPciDevice[Index].Bus,
                   mFixedPciDevice[Index].Device,
                   mFixedPciDevice[Index].Function,
                   0
                   );
    for (SubIndex = 0; SubIndex < 6; SubIndex++) {
      PciWrite32 (
        PciAddress + PCI_BASE_ADDRESSREG_OFFSET + SubIndex * sizeof(UINT32),
        mFixedPciDevice[Index].Bar[SubIndex]
        );
    }
    PciOr16 (
      PciAddress + PCI_COMMAND_OFFSET,
      mFixedPciBridge[Index].Command
      );
  }

  return EFI_SUCCESS;
}

VOID
DumpGcd (
  VOID
  )
{
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR *MemorySpaceMap;
  EFI_GCD_IO_SPACE_DESCRIPTOR     *IoSpaceMap;
  UINTN                           NumberOfDescriptors;
  UINTN                           Index;
  EFI_STATUS                      Status;

  Status = gDS->GetMemorySpaceMap (
                  &NumberOfDescriptors,
                  &MemorySpaceMap
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "GetMemorySpaceMap - %x\n", NumberOfDescriptors));
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    DEBUG ((
      EFI_D_INFO,
      "0x%016lx - %016lx (%016lx - %016lx) (%08x - %08x)\n",
      MemorySpaceMap[Index].BaseAddress,
      MemorySpaceMap[Index].Length,
      MemorySpaceMap[Index].Capabilities,
      MemorySpaceMap[Index].Attributes,
      MemorySpaceMap[Index].ImageHandle,
      MemorySpaceMap[Index].DeviceHandle
      ));
  }

  Status = gDS->GetIoSpaceMap (
                  &NumberOfDescriptors,
                  &IoSpaceMap
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "GetIoSpaceMap - %x\n", NumberOfDescriptors));
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    DEBUG ((
      EFI_D_INFO,
      "0x%016lx - %016lx (%08x - %08x)\n",
      IoSpaceMap[Index].BaseAddress,
      IoSpaceMap[Index].Length,
      IoSpaceMap[Index].ImageHandle,
      IoSpaceMap[Index].DeviceHandle
      ));
  }
}

EFI_STATUS
EFIAPI
FixedPciResourceAssignGcdResource (
  VOID
  )
{
  UINTN                Index;
  EFI_PHYSICAL_ADDRESS BaseAddress;
  EFI_STATUS           Status;

//  DumpGcd ();

  //
  // Add resource to GCD
  //
  for (Index = 0; Index < sizeof(mFixedIoResource)/sizeof(mFixedIoResource[0]); Index++) {
    Status = gDS->AddIoSpace (
                    EfiGcdIoTypeIo,
                    mFixedIoResource[Index].BaseAddress,
                    mFixedIoResource[Index].Length
                    );
    ASSERT_EFI_ERROR (Status);
  }

  for (Index = 0; Index < sizeof(mFixedMmioResource)/sizeof(mFixedMmioResource[0]); Index++) {
    Status = gDS->AddMemorySpace (
                    EfiGcdMemoryTypeMemoryMappedIo,
                    mFixedMmioResource[Index].BaseAddress,
                    mFixedMmioResource[Index].Length,
                    mFixedMmioResource[Index].Capabilities
                    );
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Allocate used resource
  //
  for (Index = 0; Index < sizeof(mUsedIoResource)/sizeof(mUsedIoResource[0]); Index++) {
    BaseAddress = mUsedIoResource[Index].BaseAddress;
    if (BaseAddress != 0) {
      Status = gDS->AllocateIoSpace (
                      EfiGcdAllocateAddress,
                      EfiGcdIoTypeIo,
                      0, // Alignment
                      mUsedIoResource[Index].Length,
                      &BaseAddress,
                      gImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
    }
  }

  for (Index = 0; Index < sizeof(mUsedMmioResource)/sizeof(mUsedMmioResource[0]); Index++) {
    BaseAddress = mUsedMmioResource[Index].BaseAddress;
    if (BaseAddress != 0) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAddress,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      0, // Alignment
                      mUsedMmioResource[Index].Length,
                      &BaseAddress,
                      gImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
    }
  }

  //
  // Handle PCIE base address
  //
  Status = gDS->AddMemorySpace (
                  EfiGcdMemoryTypeMemoryMappedIo,
                  PcdGet64(PcdPciExpressBaseAddress),
                  PcdGet64(PcdPciExpressSize),
                  EFI_MEMORY_RUNTIME | EFI_MEMORY_UC
                  );
  ASSERT_EFI_ERROR(Status);

  BaseAddress = PcdGet64(PcdPciExpressBaseAddress);
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateAddress,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  0,
                  PcdGet64(PcdPciExpressSize),
                  &BaseAddress,
                  gImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);

  Status = gDS->SetMemorySpaceAttributes (
                  PcdGet64(PcdPciExpressBaseAddress),
                  PcdGet64(PcdPciExpressSize),
                  EFI_MEMORY_RUNTIME
                  );
  ASSERT_EFI_ERROR (Status);


//  DumpGcd ();

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FixedPciResourceInitialization (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  FixedPciResourceAssignPciResource ();
  FixedPciResourceAssignGcdResource ();

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiPciRootBridgeIoProtocolGuid, NULL, 
                  &gEfiPciEnumerationCompleteProtocolGuid, NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
