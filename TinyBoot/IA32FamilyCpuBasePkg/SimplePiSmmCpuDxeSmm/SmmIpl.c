/** @file
SMM IPL inplementation

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

#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DxeSmmReadyToLock.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeCoffLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/SmmPlatformHookLib.h>

EFI_GUID  gSmmCoreFileName  = {0xfb41b879, 0x90ff, 0x4d20, {0x9d, 0xb6, 0xa7, 0xfe, 0x1a, 0xa8, 0x91, 0x6f}};

typedef struct {
  UINT32   SmrrBase;
  UINT32   SmrrSize;
  UINT32   UsableSmramBase;
  UINT32   UsableSmramSize;
} TINY_SMM_CORE_CONTEXT;

TINY_SMM_CORE_CONTEXT      mTinySmmCoreContext;

//
// SMM IPL global variables
//
EFI_SMRAM_DESCRIPTOR       mCurrentSmramRange;

/**
  Searches all Firmware Volumes for the first file matching FileType and SectionType and returns the section data.

  @param   FileType                FileType to search for within any of the firmware volumes in the platform.
  @param   SectionType             SectionType to search for within any of the matching FileTypes in the firmware volumes in the platform.
  @param   SourceSize              Return the size of the returned section data..

  @retval  != NULL                 Pointer to the allocated buffer containing the section data.
  @retval  NULL                    Section data was not found.

**/
VOID *
GetSectionInAnyFv (
  IN  CONST EFI_GUID    *NameGuid,
  IN  EFI_SECTION_TYPE  SectionType,
  OUT UINTN             *SourceSize
  )
{
  EFI_STATUS                    Status;
  UINTN                         HandleCount;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  VOID                          *SourceBuffer;
  UINT32                        AuthenticationStatus;

  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **)&Fv
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    
    //
    // Use Firmware Volume 2 Protocol to read a section of type SectionType
    //
    SourceBuffer = NULL;
    Status = Fv->ReadSection (Fv, NameGuid, SectionType, 0, &SourceBuffer, SourceSize, &AuthenticationStatus);
    if (!EFI_ERROR (Status)) {
      FreePool (HandleBuffer);
      return SourceBuffer;
    }
  }  

  FreePool(HandleBuffer);
  
  return NULL;
}
/**
  Load the SMM Core image into SMRAM and executes the SMM Core from SMRAM.

  @param[in] SmramRange  Descriptor for the range of SMRAM to reload the 
                         currently executing image.
  @param[in] Context     Context to pass into SMM Core

  @return  EFI_STATUS

**/
EFI_STATUS
ExecuteSmmCoreFromSmram (
  IN EFI_SMRAM_DESCRIPTOR  *SmramRange,
  IN VOID                  *Context
  )
{
  EFI_STATUS                    Status;
  VOID                          *SourceBuffer;
  UINTN                         SourceSize;
  PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;
  UINTN                         PageCount;
  EFI_PHYSICAL_ADDRESS          DestinationBuffer;
  EFI_IMAGE_ENTRY_POINT         EntryPoint;

  //
  // Search all Firmware Volumes for a PE/COFF image in a file of type SMM_CORE
  //  
  SourceBuffer = GetSectionInAnyFv (&gSmmCoreFileName, EFI_SECTION_PE32, &SourceSize);
  if (SourceBuffer == NULL) {
    return EFI_NOT_FOUND;
  }
  
  //
  // Initilize ImageContext
  //
  ImageContext.Handle    = SourceBuffer;
  ImageContext.ImageRead = PeCoffLoaderImageReadFromMemory;

  //
  // Get information about the image being loaded
  //
  Status = PeCoffLoaderGetImageInfo (&ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Allocate memory for the image being loaded from the EFI_SRAM_DESCRIPTOR 
  // specified by SmramRange
  //
  PageCount = (UINTN)EFI_SIZE_TO_PAGES(ImageContext.ImageSize + ImageContext.SectionAlignment);

  ASSERT ((SmramRange->PhysicalSize & EFI_PAGE_MASK) == 0);
  ASSERT (SmramRange->PhysicalSize > EFI_PAGES_TO_SIZE (PageCount));

  SmramRange->PhysicalSize -= EFI_PAGES_TO_SIZE (PageCount);
  DestinationBuffer = SmramRange->CpuStart + SmramRange->PhysicalSize;

  mTinySmmCoreContext.UsableSmramBase = (UINT32)SmramRange->CpuStart;
  mTinySmmCoreContext.UsableSmramSize = (UINT32)SmramRange->PhysicalSize;

  //
  // Align buffer on section boundry
  //
  ImageContext.ImageAddress = DestinationBuffer;
  
  ImageContext.ImageAddress += ImageContext.SectionAlignment - 1;
  ImageContext.ImageAddress &= ~(ImageContext.SectionAlignment - 1);

  //
  // Print debug message showing SMM Core load address.
  //
  DEBUG ((DEBUG_INFO, "SMM IPL loading SMM Core at SMRAM address %p\n", (VOID *)(UINTN)ImageContext.ImageAddress));

  //
  // Load the image to our new buffer
  //
  Status = PeCoffLoaderLoadImage (&ImageContext);
  if (!EFI_ERROR (Status)) {
    //
    // Relocate the image in our new buffer
    //
    Status = PeCoffLoaderRelocateImage (&ImageContext);
    if (!EFI_ERROR (Status)) {
      //
      // Flush the instruction cache so the image data are written before we execute it
      //
      InvalidateInstructionCacheRange ((VOID *)(UINTN)ImageContext.ImageAddress, (UINTN)ImageContext.ImageSize);

      //
      // Print debug message showing SMM Core entry point address.
      //
      DEBUG ((DEBUG_INFO, "SMM IPL calling SMM Core at SMRAM address %p\n", (VOID *)(UINTN)ImageContext.EntryPoint));

      //
      // Execute image
      //
      EntryPoint = (EFI_IMAGE_ENTRY_POINT)(UINTN)ImageContext.EntryPoint;
      Status = EntryPoint ((EFI_HANDLE)&mTinySmmCoreContext, gST);
    }
  }

  //
  // If the load operation, relocate operation, or the image execution return an
  // error, then free memory allocated from the EFI_SRAM_DESCRIPTOR specified by 
  // SmramRange
  //
  if (EFI_ERROR (Status)) {
    SmramRange->PhysicalSize += EFI_PAGES_TO_SIZE (PageCount);
  }

  //
  // Always free memory allocted by GetFileBufferByFilePath ()
  //
  FreePool (SourceBuffer);

  return Status;
}

BOOLEAN mSmmLocked;

/**
  Event notification that is fired every time a DxeSmmReadyToLock protocol is added
  or if gEfiEventReadyToBootGuid is signalled.

  @param  Event                 The Event that is being processed, not used.
  @param  Context               Event Context, not used.

**/
VOID
EFIAPI
SmmIplReadyToLockEventNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS  Status;
  VOID        *Interface;

  //
  // See if we are already locked
  //
  if (mSmmLocked) {
    return;
  }
  
  //
  // Make sure this notification is for this handler
  //
  Status = gBS->LocateProtocol (&gEfiDxeSmmReadyToLockProtocolGuid, NULL, &Interface);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Lock the SMRAM (Note: Locking SMRAM may not be supported on all platforms)
  //
  LockSmramRegion ();
  
  //
  // Close protocol and event notification events that do not apply after the 
  // DXE SMM Ready To Lock Protocol has been installed or the Ready To Boot 
  // event has been signalled.
  //
  gBS->CloseEvent (Event);

  //
  // Print debug message that the SMRAM window is now locked.
  //
  DEBUG ((DEBUG_INFO, "SMM IPL locked SMRAM window\n"));
  
  //
  // Set flag so this operation will not be performed again
  //
  mSmmLocked = TRUE;
}


/**
  The Entry Point for SMM IPL

  Load SMM Core into SMRAM, register SMM Core entry point for SMIs, install 
  SMM Base 2 Protocol and SMM Communication Protocol, and register for the 
  critical events required to coordinate between DXE and SMM environments.
  
  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval Other          Some error occurred when executing this entry point.

**/
EFI_STATUS
EFIAPI
SmmIplEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  // Open all SMRAM ranges
  //
  if (!OpenSmramRegion ()) {
    ASSERT (FALSE);
  }

  //
  // Print debug message that the SMRAM window is now open.
  //
  DEBUG ((DEBUG_INFO, "SMM IPL opened SMRAM window\n"));

  //
  // Find the largest SMRAM range between 1MB and 4GB that is at least 256KB - 4K in size
  //
  GetSmramRegion (&mCurrentSmramRange.PhysicalStart, &mCurrentSmramRange.PhysicalSize);
  mCurrentSmramRange.CpuStart      = mCurrentSmramRange.PhysicalStart;
  mCurrentSmramRange.RegionState   = EFI_SMRAM_OPEN;

  //
  // Print debug message showing SMRAM window that will be used by SMM IPL and SMM Core
  //
  DEBUG ((DEBUG_INFO, "SMM IPL found SMRAM window %p - %x\n", 
    (VOID *)(UINTN)mCurrentSmramRange.CpuStart, 
    (VOID *)(UINTN)mCurrentSmramRange.PhysicalSize
    ));

  mTinySmmCoreContext.SmrrBase = (UINT32)mCurrentSmramRange.CpuStart;
  mTinySmmCoreContext.SmrrSize = (UINT32)mCurrentSmramRange.PhysicalSize;

  //
  // Load SMM Core into SMRAM and execute it from SMRAM
  //
  ExecuteSmmCoreFromSmram (&mCurrentSmramRange, &mCurrentSmramRange);

  if (!CloseSmramRegion ()) {
    ASSERT (FALSE);
  }

  //
  // Print debug message that the SMRAM window is now closed.
  //
  DEBUG ((DEBUG_INFO, "SMM IPL closed SMRAM window\n"));

  {
    EFI_EVENT  Event;
    VOID       *Registration;

    DEBUG ((DEBUG_INFO, "EfiCreateProtocolNotifyEvent ...\n"));

    Registration = NULL;
    Event = EfiCreateProtocolNotifyEvent (
              &gEfiDxeSmmReadyToLockProtocolGuid,
              TPL_CALLBACK,
              SmmIplReadyToLockEventNotify,
              NULL,
              &Registration
              );
    ASSERT (Event != NULL);

    DEBUG ((DEBUG_INFO, "EfiCreateProtocolNotifyEvent Done\n"));

  }


  return EFI_SUCCESS;
}
