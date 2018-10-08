/** @file
  Locate the entry point for the PEI Core

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/PeCoffGetEntryPointLib.h>

#include "SecMain.h"

/**
  Check if a block of buffer is erased.

  @param  ErasePolarity  Erase polarity attribute of the firmware volume
  @param  InBuffer       The buffer to be checked
  @param  BufferSize     Size of the buffer in bytes

  @retval TRUE           The block of buffer is erased
  @retval FALSE          The block of buffer is not erased

**/
BOOLEAN
IsBufferErased (
  IN UINT8    ErasePolarity,
  IN VOID     *InBuffer,
  IN UINTN    BufferSize
  )
{
  UINTN   Count;
  UINT8   EraseByte;
  UINT8   *Buffer;

  if(ErasePolarity == 1) {
    EraseByte = 0xFF;
  } else {
    EraseByte = 0;
  }

  Buffer = InBuffer;
  for (Count = 0; Count < BufferSize; Count++) {
    if (Buffer[Count] != EraseByte) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
  Find core image base.

  @param   BootFirmwareVolumePtr    Pointer to the boot firmware volume.
  @param   SecCoreImageBase         Pointer to the base address of the SEC core
                                    image, or NULL if no need to find SEC core.
  @param   PeiCoreImageBase         Pointer to the base address of the PEI core
                                    image, or NULL if no need to find PEI core.

**/
VOID
EFIAPI
FindImageBase (
  IN  EFI_FIRMWARE_VOLUME_HEADER       *BootFirmwareVolumePtr,
  OUT EFI_PHYSICAL_ADDRESS             *SecCoreImageBase, OPTIONAL
  OUT EFI_PHYSICAL_ADDRESS             *PeiCoreImageBase  OPTIONAL
  )
{
  EFI_PHYSICAL_ADDRESS        CurrentAddress;
  EFI_PHYSICAL_ADDRESS        EndOfFirmwareVolume;
  EFI_FFS_FILE_HEADER         *File;
  UINT32                      Size;
  EFI_PHYSICAL_ADDRESS        EndOfFile;
  EFI_COMMON_SECTION_HEADER   *Section;
  EFI_PHYSICAL_ADDRESS        EndOfSection;
  BOOLEAN                     ErasePolarity;
  UINTN                       TestLength;

  if ((SecCoreImageBase == NULL) && (PeiCoreImageBase == NULL)) {
    return;
  }

  if (SecCoreImageBase != NULL) {
    *SecCoreImageBase = 0;
  }
  if (PeiCoreImageBase != NULL) {
    *PeiCoreImageBase = 0;
  }

  CurrentAddress = (EFI_PHYSICAL_ADDRESS)(UINTN) BootFirmwareVolumePtr;
  EndOfFirmwareVolume = CurrentAddress + BootFirmwareVolumePtr->FvLength;

  ErasePolarity = ((BootFirmwareVolumePtr->Attributes & EFI_FVB2_ERASE_POLARITY) != 0);

  //
  // Loop through the FFS files in the Boot Firmware Volume
  //
  for (EndOfFile = CurrentAddress + BootFirmwareVolumePtr->HeaderLength; ; ) {

    CurrentAddress = (EndOfFile + 7) & 0xfffffffffffffff8ULL;
    if (CurrentAddress >= EndOfFirmwareVolume) {
      return;
    }

    TestLength = (UINTN) (EndOfFirmwareVolume - CurrentAddress);
    if (TestLength > sizeof (EFI_FFS_FILE_HEADER)) {
      TestLength = sizeof (EFI_FFS_FILE_HEADER);
    }

    if (IsBufferErased (ErasePolarity, (VOID *) (UINTN) CurrentAddress, TestLength)) {
      //
      // We have found the free space so we are done!
      //
      return;
    }

    File = (EFI_FFS_FILE_HEADER*)(UINTN) CurrentAddress;
    if (IS_FFS_FILE2 (File)) {
      Size = FFS_FILE2_SIZE (File);
      if (Size <= 0x00FFFFFF) {
        return;
      }
    } else {
      Size = FFS_FILE_SIZE (File);
      if (Size < sizeof (EFI_FFS_FILE_HEADER)) {
        return;
      }
    }

    EndOfFile = CurrentAddress + Size;

    //
    // Look for SEC Core / PEI Core files
    //
    if (File->Type != EFI_FV_FILETYPE_SECURITY_CORE &&
        File->Type != EFI_FV_FILETYPE_PEI_CORE) {
      continue;
    }

    if (((File->Type == EFI_FV_FILETYPE_SECURITY_CORE) && (SecCoreImageBase == NULL)) ||
        ((File->Type == EFI_FV_FILETYPE_PEI_CORE) && (PeiCoreImageBase == NULL))) {
      continue;
    }

    //
    // Loop through the FFS file sections within the FFS file
    //
    if (IS_FFS_FILE2 (File)) {
      EndOfSection = (EFI_PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) File + sizeof (EFI_FFS_FILE_HEADER2));
    } else {
      EndOfSection = (EFI_PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) File + sizeof (EFI_FFS_FILE_HEADER));
    }
    for (;;) {
      CurrentAddress = (EndOfSection + 3) & 0xfffffffffffffffcULL;
      Section = (EFI_COMMON_SECTION_HEADER*)(UINTN) CurrentAddress;

      if (IS_SECTION2 (Section)) {
        Size = SECTION2_SIZE (Section);
        if (Size <= 0x00FFFFFF) {
          return;
        }
      } else {
        Size = SECTION_SIZE (Section);
        if (Size < sizeof (EFI_COMMON_SECTION_HEADER)) {
          return;
        }
      }

      EndOfSection = CurrentAddress + Size;

      //
      // Look for executable sections
      //
      if (Section->Type == EFI_SECTION_PE32 || Section->Type == EFI_SECTION_TE) {
        if (File->Type == EFI_FV_FILETYPE_SECURITY_CORE) {
          if (IS_SECTION2 (Section)) {
            *SecCoreImageBase = (PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER2));
          } else {
            *SecCoreImageBase = (PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER));
          }
        } else {
          if (IS_SECTION2 (Section)) {
            *PeiCoreImageBase = (PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER2));
          } else {
            *PeiCoreImageBase = (PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER));
          }
        }
        break;
      }

      if (EndOfSection >= EndOfFile) {
        break;
      }
    }

    //
    // Both SEC Core and PEI Core images found
    //
    if (((SecCoreImageBase == NULL) || (*SecCoreImageBase != 0)) &&
        ((PeiCoreImageBase == NULL) || (*PeiCoreImageBase != 0))) {
      return;
    }

    if (EndOfFile >= EndOfFirmwareVolume) {
      return;
    }
  }
}

/**
  Find and report SecCore and PeiCore.

  It finds SEC and PEI Core file debug information.
  It will report them if remote debug is enabled.
  It will return Pei Core entry point.

  @param BootFirmwareVolumePtr  Pointer to the boot firmware volume.
  @param SecCoreFound           Pointer to the indicator to return whether the
                                SecCore is found or not, or NULL if no need to
                                find SecCore.
  @param PeiCoreFound           Pointer to the indicator to return whether the
                                PeiCore is found or not, or NULL if no need to
                                find PeiCore.
  @param PeiCoreEntryPoint      Pointer to the entry point of the PEI core, or
                                NULL if no need to find PeiCore or PeiCoreFound
                                is NULL.

**/
VOID
EFIAPI
FindAndReportSecAndPeiCore (
  IN  EFI_FIRMWARE_VOLUME_HEADER       *BootFirmwareVolumePtr,
  OUT BOOLEAN                          *SecCoreFound,     OPTIONAL
  OUT BOOLEAN                          *PeiCoreFound,     OPTIONAL
  OUT EFI_PEI_CORE_ENTRY_POINT         *PeiCoreEntryPoint OPTIONAL
  )
{
  EFI_STATUS                       Status;
  EFI_PHYSICAL_ADDRESS             SecCoreImageBase;
  EFI_PHYSICAL_ADDRESS             PeiCoreImageBase;
  PE_COFF_LOADER_IMAGE_CONTEXT     ImageContext;

  if ((SecCoreFound == NULL) && (PeiCoreFound == NULL)) {
    return;
  }

  if (SecCoreFound != NULL) {
    *SecCoreFound = FALSE;
  }

  if (PeiCoreFound != NULL) {
    *PeiCoreFound = FALSE;
    if (PeiCoreEntryPoint != NULL) {
      *PeiCoreEntryPoint = NULL;
    }
  }

  //
  // Find SEC Core and PEI Core image base
  //
  FindImageBase (
    BootFirmwareVolumePtr,
    SecCoreFound ? &SecCoreImageBase : NULL,
    PeiCoreFound ? &PeiCoreImageBase : NULL
    );

  ZeroMem ((VOID *) &ImageContext, sizeof (PE_COFF_LOADER_IMAGE_CONTEXT));

  if (SecCoreFound != NULL) {
    if (SecCoreImageBase != 0) {
      //
      // Report SEC Core debug information when remote debug is enabled
      //
      ImageContext.ImageAddress = SecCoreImageBase;
      ImageContext.PdbPointer = PeCoffLoaderGetPdbPointer ((VOID*) (UINTN) ImageContext.ImageAddress);
      PeCoffLoaderRelocateImageExtraAction (&ImageContext);
    }
  }

  if (PeiCoreFound != NULL) {
    if (PeiCoreImageBase != 0) {
      //
      // Report PEI Core debug information when remote debug is enabled
      //
      ImageContext.ImageAddress = PeiCoreImageBase;
      ImageContext.PdbPointer = PeCoffLoaderGetPdbPointer ((VOID*) (UINTN) ImageContext.ImageAddress);
      PeCoffLoaderRelocateImageExtraAction (&ImageContext);

      if (PeiCoreEntryPoint != NULL) {
        //
        // Find PEI Core entry point
        //
        Status = PeCoffLoaderGetEntryPoint ((VOID *) (UINTN) PeiCoreImageBase, (VOID **) PeiCoreEntryPoint);
        ASSERT_EFI_ERROR (Status);
      }
    }
  }

  return;
}
