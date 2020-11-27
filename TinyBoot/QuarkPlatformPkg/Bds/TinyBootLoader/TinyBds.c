/** @file
  This module produce main entry for BDS phase - BdsEntry.
  When this module was dispatched by DxeCore, gEfiBdsArchProtocolGuid will be installed
  which contains interface of BdsEntry.
  After DxeCore finish DXE phase, gEfiBdsArchProtocolGuid->BdsEntry will be invoked
  to enter BDS phase.

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

#include <PiDxe.h>

#include <Protocol/Bds.h>

#include <Guid/GlobalVariable.h>

#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/MtrrLib.h>
#include <Library/FvLib.h>

#include <Library/DxeServicesTableLib.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci.h>

//
//
//
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  );

//
// Template for BDS Architectural Protocol. Global template is a size optimization
//
EFI_HANDLE            mHandle = NULL;
EFI_BDS_ARCH_PROTOCOL mBds    = {
  BdsEntry
};

UINT32   mMsrIndex[] = {
  MTRR_LIB_IA32_MTRR_CAP, // 0x0FE
//  MTRR_LIB_IA32_MTRR_CAP_VCNT_MASK, // 0x0FF
  MTRR_LIB_IA32_MTRR_FIX64K_00000, // 0x250
  MTRR_LIB_IA32_MTRR_FIX16K_80000, // 0x258
  MTRR_LIB_IA32_MTRR_FIX16K_A0000, // 0x259
  MTRR_LIB_IA32_MTRR_FIX4K_C0000, // 0x268
  MTRR_LIB_IA32_MTRR_FIX4K_C8000, // 0x269
  MTRR_LIB_IA32_MTRR_FIX4K_D0000, // 0x26A
  MTRR_LIB_IA32_MTRR_FIX4K_D8000, // 0x26B
  MTRR_LIB_IA32_MTRR_FIX4K_E0000, // 0x26C
  MTRR_LIB_IA32_MTRR_FIX4K_E8000, // 0x26D
  MTRR_LIB_IA32_MTRR_FIX4K_F0000, // 0x26E
  MTRR_LIB_IA32_MTRR_FIX4K_F8000, // 0x26F
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE, // 0x200
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 1,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 2,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 3,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 4,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 5,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 6,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 7,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 8,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 9,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xA,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xB,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xC,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xD,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xE,
  MTRR_LIB_IA32_VARIABLE_MTRR_END, // 0x20F
  MTRR_LIB_IA32_MTRR_DEF_TYPE, // 0x2FF
};

VOID
DumpMtrr (
  VOID
  )
{
  UINTN    Index;
  UINT64   Data;
  DEBUG ((EFI_D_INFO, "*************************\n"));

  for (Index = 0; Index < sizeof(mMsrIndex)/sizeof(mMsrIndex[0]); Index++) {
    Data = AsmReadMsr64 (mMsrIndex[Index]);
    DEBUG ((EFI_D_INFO, "MSR(0x%x - 0x%lx)\n", mMsrIndex[Index], Data));
  }

  DEBUG ((EFI_D_INFO, "*************************\n"));
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

  DEBUG ((EFI_D_INFO, "*************************\n"));

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
  DEBUG ((EFI_D_INFO, "*************************\n"));
}

VOID
DumpPci (
  VOID
  )
{
  UINTN  Status;
  EFI_HANDLE                            *Handles;
  UINTN                                 HandleCount;
  UINTN                                 Index;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  UINT8                                 Buffer[sizeof(PCI_TYPE_GENERIC)];
  UINT8                                 DumpIndex;
  UINTN                                 SegmentNumber;
  UINTN                                 BusNumber;
  UINTN                                 DeviceNumber;
  UINTN                                 FunctionNumber;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );

  DEBUG ((EFI_D_INFO, "*************************\n"));
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    Status = PciIo->GetLocation (PciIo, &SegmentNumber, &BusNumber, &DeviceNumber, &FunctionNumber);
    DEBUG ((EFI_D_INFO, "PCI: [%02x|%02x|%02x]\n", BusNumber, DeviceNumber, FunctionNumber));
    Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0, sizeof(Buffer), Buffer);
    for (DumpIndex = 0; DumpIndex < sizeof(Buffer); DumpIndex++) {
      DEBUG ((EFI_D_INFO, "%02x ", Buffer[DumpIndex]));
      if (((DumpIndex + 1) % 16) == 0) {
        DEBUG ((EFI_D_INFO, "\n"));
      }
    }
  }
  DEBUG ((EFI_D_INFO, "*************************\n"));
}

GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN   mConnectAll      = FALSE;

VOID 
EfiBootManagerConnectAll (
  IN  BOOLEAN   OnlyOncePerBoot
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINTN       Index;

  if (OnlyOncePerBoot && mConnectAll) {
    return;
  }
  mConnectAll = TRUE;

  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
  }

  FreePool (HandleBuffer);

  return;  
}

VOID 
EfiBootManagerConnectAllAndDispatch (
  IN  BOOLEAN   OnlyOncePerBoot
  )
{
  EFI_STATUS  Status;

  do {
    //
    // Connect All EFI 1.10 drivers following EFI 1.10 algorithm
    //
    DEBUG ((EFI_D_INFO, "EfiBootManagerConnectAll begin\n"));
    EfiBootManagerConnectAll (OnlyOncePerBoot);
    DEBUG ((EFI_D_INFO, "EfiBootManagerConnectAll end\n"));

    //
    // Check to see if it's possible to dispatch an more DXE drivers.
    // The BdsLibConnectAllEfi () may have made new DXE drivers show up.
    // If anything is Dispatched Status == EFI_SUCCESS and we will try
    // the connect again.
    //
    Status = gDS->Dispatch ();

  } while (!EFI_ERROR (Status));
}

VOID
AddConsole (
  VOID
  )
{
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *SimpleTextOut;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL    *SimpleTextIn;
  EFI_STATUS                         Status;

  Status = gBS->LocateProtocol (&gEfiSimpleTextOutProtocolGuid, NULL, (VOID **) &SimpleTextOut);
  if (Status == EFI_SUCCESS){
    DEBUG ((EFI_D_INFO, "Get SimpleTextOut\n"));
    gST->ConOut = SimpleTextOut;
    gST->StdErr = SimpleTextOut;
  } else {
    DEBUG ((EFI_D_INFO, "Get SimpleTextOut failed!\n"));
  }

  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid, NULL, (VOID **) &SimpleTextIn);
  if (Status == EFI_SUCCESS){
    DEBUG ((EFI_D_INFO, "Get SimpleTextIn\n"));
    gST->ConIn = SimpleTextIn;
  } else {
    DEBUG ((EFI_D_INFO, "Get SimpleTextIn failed!\n"));
  }

  //
  // Fixup Tasble CRC
  //
  gBS->CalculateCrc32 ((VOID *)gST, sizeof (EFI_SYSTEM_TABLE), &gST->Hdr.CRC32);

}

#pragma pack(1)
typedef struct {
  MEMMAP_DEVICE_PATH  Memmap;
  EFI_DEVICE_PATH     End;
} THIS_DEVICE_PATH;
#pragma pack()

THIS_DEVICE_PATH         ThisDevicePath = {
  {
    {HARDWARE_DEVICE_PATH, HW_MEMMAP_DP, sizeof(MEMMAP_DEVICE_PATH), 0},
    EfiReservedMemoryType,
    0,
    0,
  },
  {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof(EFI_DEVICE_PATH_PROTOCOL), 0},
};

VOID
BootApp (
  VOID
  )
{
  EFI_STATUS               Status;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;
  VOID                     *SourceBuffer;
  UINTN                    SourceSize;
  EFI_HANDLE               ImageHandle;
  UINTN                    ExitDataSize;
  CHAR16                   *ExitData;

  SourceBuffer = FindFirstImageInFv (
                   (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)(FixedPcdGet32 (PcdPlatformFlashFvPayloadBase)),
                   &SourceSize
                   );

  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)&ThisDevicePath;
  ThisDevicePath.Memmap.Header.Type = HARDWARE_DEVICE_PATH;
  ThisDevicePath.Memmap.Header.SubType = HW_MEMMAP_DP;
  ThisDevicePath.Memmap.Header.Length[0] = sizeof(MEMMAP_DEVICE_PATH);
  ThisDevicePath.Memmap.Header.Length[1] = 0;
  ThisDevicePath.Memmap.MemoryType = EfiReservedMemoryType;
  ThisDevicePath.Memmap.StartingAddress = (UINT64)(UINTN)SourceBuffer;
  ThisDevicePath.Memmap.EndingAddress = (UINT64)((UINTN)SourceBuffer + SourceSize);
  ThisDevicePath.End.Type = END_DEVICE_PATH_TYPE;
  ThisDevicePath.End.SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;
  ThisDevicePath.End.Length[0] = sizeof(EFI_DEVICE_PATH_PROTOCOL);
  ThisDevicePath.End.Length[1] = 0;
  DEBUG ((EFI_D_INFO, "LoadImage ...\n"));
  Status = gBS->LoadImage (
                  TRUE,
                  gImageHandle,
                  DevicePath,
                  SourceBuffer,
                  SourceSize,
                  &ImageHandle
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "StartImage ...\n"));
  Status = gBS->StartImage (
                  ImageHandle,
                  &ExitDataSize,
                  &ExitData
                  );
  ASSERT (FALSE);
}

/**
  This function uses policy data from the platform to determine what operating 
  system or system utility should be loaded and invoked.  This function call 
  also optionally make the use of user input to determine the operating system 
  or system utility to be loaded and invoked.  When the DXE Core has dispatched 
  all the drivers on the dispatch queue, this function is called.  This 
  function will attempt to connect the boot devices required to load and invoke 
  the selected operating system or system utility.  During this process, 
  additional firmware volumes may be discovered that may contain addition DXE 
  drivers that can be dispatched by the DXE Core.   If a boot device cannot be 
  fully connected, this function calls the DXE Service Dispatch() to allow the 
  DXE drivers from any newly discovered firmware volumes to be dispatched.  
  Then the boot device connection can be attempted again.  If the same boot 
  device connection operation fails twice in a row, then that boot device has 
  failed, and should be skipped.  This function should never return.

  @param  This             The EFI_BDS_ARCH_PROTOCOL instance.

  @return None.

**/
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  )
{
  DEBUG ((EFI_D_INFO, "Tiny BDS\n"));

  DEBUG_CODE_BEGIN ();
    DumpPci ();
    DumpGcd ();
    DumpMtrr ();
  DEBUG_CODE_END ();

  if (0) {
    DEBUG ((EFI_D_INFO, "EfiBootManagerConnectAllAndDispatch begin\n"));
    EfiBootManagerConnectAllAndDispatch (FALSE);
    DEBUG ((EFI_D_INFO, "EfiBootManagerConnectAllAndDispatch end\n"));
  }

  AddConsole ();

  BootApp ();

  CpuDeadLoop ();
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the image goes into a library that calls this 
  function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
BdsIntialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                  Status;
  CHAR16                      *Str;

  gST->FirmwareRevision = PcdGet32 (PcdFirmwareRevision);

  Str = (CHAR16 *)PcdGetPtr (PcdFirmwareVendor);
  gST->FirmwareVendor = AllocateRuntimeCopyPool (StrSize (Str), Str);

  //
  // Fixup Tasble CRC after we updated Firmware Vendor and Revision
  //
  gBS->CalculateCrc32 ((VOID *)gST, sizeof (EFI_SYSTEM_TABLE), &gST->Hdr.CRC32);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiBdsArchProtocolGuid,  &mBds,
                  NULL       
                  );

  return Status;
}
