/** @file
  Firmware File System driver that produce Firmware Volume protocol.
  Layers on top of Firmware Block protocol to produce a file abstraction
  of FV based files.

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

#include "DxeMain.h"
#include "FwVolDriver.h"

//
// Protocol notify related globals
//
VOID          *gEfiFwVolBlockNotifyReg;
EFI_EVENT     gEfiFwVolBlockEvent;

FV_DEVICE mFvDevice = {
  FV2_DEVICE_SIGNATURE,
  0, // BaseAddress
  0, // Length
  NULL,
  {
    FvGetVolumeAttributes,
    FvSetVolumeAttributes,
    FvReadFile,
    FvReadFileSection,
    FvWriteFile,
    FvGetNextFile,   
	sizeof (UINTN),
    NULL,
    FvGetVolumeInfo,
    FvSetVolumeInfo
  },
  NULL, // DevicePath
  NULL,
  NULL,
  NULL,
  NULL,
  { NULL, NULL },
  0,
  FALSE,
  0
};


//
// FFS helper functions
//

/**
  Given the supplied FW_VOL_BLOCK_PROTOCOL, allocate a buffer for output and
  copy the real length volume header into it.

  @param  FwVolHeader           Pointer to pointer to allocated buffer in which
                                the volume header is returned.

  @retval EFI_OUT_OF_RESOURCES  No enough buffer could be allocated.
  @retval EFI_SUCCESS           Successfully read volume header to the allocated
                                buffer.

**/
EFI_STATUS
GetFwVolHeader (
  IN     EFI_PHYSICAL_ADDRESS                   BaseAddress,
  OUT    EFI_FIRMWARE_VOLUME_HEADER             **FwVolHeader
  )
{
  EFI_FIRMWARE_VOLUME_HEADER  TempFvh;
  
  //
  // Read the standard FV header
  //
  CopyMem (&TempFvh, (VOID *)(UINTN)BaseAddress, sizeof (EFI_FIRMWARE_VOLUME_HEADER));

  //
  // Allocate a buffer for the caller
  //
  *FwVolHeader = AllocatePool (TempFvh.HeaderLength);
  if (*FwVolHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Copy the standard header into the buffer
  //
  CopyMem (*FwVolHeader, (VOID *)(UINTN)BaseAddress, TempFvh.HeaderLength);

  return EFI_SUCCESS;
}



/**
  Free FvDevice resource when error happens

  @param  FvDevice              pointer to the FvDevice to be freed.

**/
VOID
FreeFvDeviceResource (
  IN FV_DEVICE  *FvDevice
  )
{
  FFS_FILE_LIST_ENTRY         *FfsFileEntry;
  LIST_ENTRY                  *NextEntry;

  //
  // Free File List Entry
  //
  FfsFileEntry = (FFS_FILE_LIST_ENTRY *)FvDevice->FfsFileListHeader.ForwardLink;
  while (&FfsFileEntry->Link != &FvDevice->FfsFileListHeader) {
    NextEntry = (&FfsFileEntry->Link)->ForwardLink;

    if (FfsFileEntry->StreamHandle != 0) {
      //
      // Close stream and free resources from SEP
      //
      CloseSectionStream (FfsFileEntry->StreamHandle);
    }

    CoreFreePool (FfsFileEntry);

    FfsFileEntry = (FFS_FILE_LIST_ENTRY *) NextEntry;
  }


  //
  // Free the cache
  //
  CoreFreePool (FvDevice->CachedFv);

  //
  // Free Volume Header
  //
  CoreFreePool (FvDevice->FwVolHeader);

  return;
}



/**
  Check if an FV is consistent and allocate cache for it.

  @param  FvDevice              A pointer to the FvDevice to be checked.

  @retval EFI_OUT_OF_RESOURCES  No enough buffer could be allocated.
  @retval EFI_SUCCESS           FV is consistent and cache is allocated.
  @retval EFI_VOLUME_CORRUPTED  File system is corrupted.

**/
EFI_STATUS
FvCheck (
  IN OUT FV_DEVICE  *FvDevice
  )
{
  EFI_STATUS                            Status;
  EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader;
  EFI_FIRMWARE_VOLUME_EXT_HEADER        *FwVolExtHeader;
  EFI_FVB_ATTRIBUTES_2                  FvbAttributes;
  FFS_FILE_LIST_ENTRY                   *FfsFileEntry;
  EFI_FFS_FILE_HEADER                   *FfsHeader;
  UINTN                                 Size;
  EFI_FFS_FILE_STATE                    FileState;
  UINT8                                 *TopFvAddress;
  UINTN                                 TestLength;

  FwVolHeader = FvDevice->FwVolHeader;

  FvbAttributes = FwVolHeader->Attributes & ~EFI_FVB2_WRITE_STATUS;

  //
  // Size is the size of the FV minus the head. We have already allocated
  // the header to check to make sure the volume is valid
  //
  Size = (UINTN)(FwVolHeader->FvLength - FwVolHeader->HeaderLength);
  FvDevice->CachedFv = AllocatePool (Size);

  if (FvDevice->CachedFv == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Remember a pointer to the end fo the CachedFv
  //
  FvDevice->EndOfCachedFv = FvDevice->CachedFv + Size;

  //
  // Copy FV minus header into memory using the block map we have all ready
  // read into memory.
  //
  CopyMem (FvDevice->CachedFv, (UINT8 *)(UINTN)FvDevice->BaseAddress + FwVolHeader->HeaderLength, Size);

  //
  // Scan to check the free space & File list
  //
  if ((FvbAttributes & EFI_FVB2_ERASE_POLARITY) != 0) {
    FvDevice->ErasePolarity = 1;
  } else {
    FvDevice->ErasePolarity = 0;
  }


  //
  // go through the whole FV cache, check the consistence of the FV.
  // Make a linked list of all the Ffs file headers
  //
  Status = EFI_SUCCESS;
  InitializeListHead (&FvDevice->FfsFileListHeader);

  //
  // Build FFS list
  //
  if (FwVolHeader->ExtHeaderOffset != 0) {
    //
    // Searching for files starts on an 8 byte aligned boundary after the end of the Extended Header if it exists.
    //
    FwVolExtHeader = (EFI_FIRMWARE_VOLUME_EXT_HEADER *) (FvDevice->CachedFv + (FwVolHeader->ExtHeaderOffset - FwVolHeader->HeaderLength));
    FfsHeader = (EFI_FFS_FILE_HEADER *) ((UINT8 *) FwVolExtHeader + FwVolExtHeader->ExtHeaderSize);
    FfsHeader = (EFI_FFS_FILE_HEADER *) ALIGN_POINTER (FfsHeader, 8);
  } else {
    FfsHeader = (EFI_FFS_FILE_HEADER *) (FvDevice->CachedFv);
  }
  TopFvAddress = FvDevice->EndOfCachedFv;
  while ((UINT8 *) FfsHeader < TopFvAddress) {

    TestLength = TopFvAddress - ((UINT8 *) FfsHeader);
    if (TestLength > sizeof (EFI_FFS_FILE_HEADER)) {
      TestLength = sizeof (EFI_FFS_FILE_HEADER);
    }

    if (IsBufferErased (FvDevice->ErasePolarity, FfsHeader, TestLength)) {
      //
      // We have found the free space so we are done!
      //
      goto Done;
    }

    if (!IsValidFfsHeader (FvDevice->ErasePolarity, FfsHeader, &FileState)) {
      if ((FileState == EFI_FILE_HEADER_INVALID) ||
          (FileState == EFI_FILE_HEADER_CONSTRUCTION)) {
        if (IS_FFS_FILE2 (FfsHeader)) {
          if (!FvDevice->IsFfs3Fv) {
            DEBUG ((EFI_D_ERROR, "Found a FFS3 formatted file: %g in a non-FFS3 formatted FV.\n", &FfsHeader->Name));
          }
          FfsHeader = (EFI_FFS_FILE_HEADER *) ((UINT8 *) FfsHeader + sizeof (EFI_FFS_FILE_HEADER2));
        } else {
          FfsHeader = (EFI_FFS_FILE_HEADER *) ((UINT8 *) FfsHeader + sizeof (EFI_FFS_FILE_HEADER));
        }
        continue;
      } else {
        //
        // File system is corrputed
        //
        Status = EFI_VOLUME_CORRUPTED;
        goto Done;
      }
    }

    if (!IsValidFfsFile (FvDevice->ErasePolarity, FfsHeader)) {
      //
      // File system is corrupted
      //
      Status = EFI_VOLUME_CORRUPTED;
      goto Done;
    }

    if (IS_FFS_FILE2 (FfsHeader)) {
      ASSERT (FFS_FILE2_SIZE (FfsHeader) > 0x00FFFFFF);
      if (!FvDevice->IsFfs3Fv) {
        DEBUG ((EFI_D_ERROR, "Found a FFS3 formatted file: %g in a non-FFS3 formatted FV.\n", &FfsHeader->Name));
        FfsHeader = (EFI_FFS_FILE_HEADER *) ((UINT8 *) FfsHeader + FFS_FILE2_SIZE (FfsHeader));
        //
        // Adjust pointer to the next 8-byte aligned boundry.
        //
        FfsHeader = (EFI_FFS_FILE_HEADER *) (((UINTN) FfsHeader + 7) & ~0x07);
        continue;
      }
    }

    FileState = GetFileState (FvDevice->ErasePolarity, FfsHeader);

    //
    // check for non-deleted file
    //
    if (FileState != EFI_FILE_DELETED) {
      //
      // Create a FFS list entry for each non-deleted file
      //
      FfsFileEntry = AllocateZeroPool (sizeof (FFS_FILE_LIST_ENTRY));
      if (FfsFileEntry == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      FfsFileEntry->FfsHeader = FfsHeader;
      InsertTailList (&FvDevice->FfsFileListHeader, &FfsFileEntry->Link);
    }

    if (IS_FFS_FILE2 (FfsHeader)) {
      FfsHeader = (EFI_FFS_FILE_HEADER *) ((UINT8 *) FfsHeader + FFS_FILE2_SIZE (FfsHeader));
    } else {
      FfsHeader = (EFI_FFS_FILE_HEADER *) ((UINT8 *) FfsHeader + FFS_FILE_SIZE (FfsHeader));
    }

    //
    // Adjust pointer to the next 8-byte aligned boundry.
    //
    FfsHeader = (EFI_FFS_FILE_HEADER *)(((UINTN)FfsHeader + 7) & ~0x07);

  }

Done:
  if (EFI_ERROR (Status)) {
    FreeFvDeviceResource (FvDevice);
  }

  return Status;
}



/**
  This notification function is invoked when an instance of the
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL is produced.  It layers an instance of the
  EFI_FIRMWARE_VOLUME2_PROTOCOL on the same handle.  This is the function where
  the actual initialization of the EFI_FIRMWARE_VOLUME2_PROTOCOL is done.

  @param  Event                 The event that occured
  @param  Context               For EFI compatiblity.  Not used.

**/
VOID
EFIAPI
NotifyFwVolBlock (
  IN  EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN  UINT64                Length
  )
{
  EFI_STATUS                            Status;
  FV_DEVICE                             *FvDevice;
  EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader;

  {
    //
    // Make sure the Fv Header is O.K.
    //
    Status = GetFwVolHeader (BaseAddress, &FwVolHeader);
    if (EFI_ERROR (Status)) {
      return;
    }
    ASSERT (FwVolHeader != NULL);

    if (!VerifyFvHeaderChecksum (FwVolHeader)) {
      CoreFreePool (FwVolHeader);
      return ;
    }


    //
    // Check to see that the file system is indeed formatted in a way we can
    // understand it...
    //
    if ((!CompareGuid (&FwVolHeader->FileSystemGuid, &gEfiFirmwareFileSystem2Guid)) &&
        (!CompareGuid (&FwVolHeader->FileSystemGuid, &gEfiFirmwareFileSystem3Guid))) {
      return ;
    }

    {
      FvDevice = AllocateCopyPool (sizeof (FV_DEVICE), &mFvDevice);
      if (FvDevice == NULL) {
        return;
      }

      FvDevice->BaseAddress     = BaseAddress;
      FvDevice->Length          = Length;
      FvDevice->FwVolHeader     = FwVolHeader;
      FvDevice->IsFfs3Fv        = CompareGuid (&FwVolHeader->FileSystemGuid, &gEfiFirmwareFileSystem3Guid);
      FvDevice->Fv.ParentHandle = NULL;

      //
      // Inherit the authentication status from FVB.
      //
      FvDevice->AuthenticationStatus = 0;
      
      if (!EFI_ERROR (FvCheck (FvDevice))) {

        FvDevice->DevicePath = CreateFvDevicePath (FwVolHeader);

        //
        // Install an New FV protocol on the existing handle
        //
        Status = CoreInstallMultipleProtocolInterfaces (
                   &FvDevice->Handle,
                   &gEfiFirmwareVolume2ProtocolGuid,     &FvDevice->Fv,
                   &gEfiDevicePathProtocolGuid,          FvDevice->DevicePath,
                   NULL
                   );
        ASSERT_EFI_ERROR (Status);
      } else {
        //
        // Free FvDevice Buffer for the corrupt FV image.
        //
        CoreFreePool (FvDevice);
      }
    }
  }

  return;
}



/**
  This routine is the driver initialization entry point.  It registers
  a notification function.  This notification function are responsible
  for building the FV stack dynamically.

  @param  ImageHandle           The image handle.
  @param  SystemTable           The system table.

  @retval EFI_SUCCESS           Function successfully returned.

**/
EFI_STATUS
EFIAPI
FwVolDriverInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_PEI_HOB_POINTERS          FvHob;

  //
  // Core Needs Firmware Volumes to function
  //
  FvHob.Raw = GetHobList ();
  while ((FvHob.Raw = GetNextHob (EFI_HOB_TYPE_FV, FvHob.Raw)) != NULL) {
    //
    // Produce an FVB protocol for it
    //
    NotifyFwVolBlock (FvHob.FirmwareVolume->BaseAddress, FvHob.FirmwareVolume->Length);
    FvHob.Raw = GET_NEXT_HOB (FvHob);
  }

  return EFI_SUCCESS;
}


