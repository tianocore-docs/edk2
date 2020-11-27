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

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/SerialDebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ExtractGuidedSectionLib.h>

/**
  Locates a section within a series of sections
  with the specified section type.

  @param[in]   Sections        The sections to search
  @param[in]   SizeOfSections  Total size of all sections
  @param[in]   SectionType     The section type to locate
  @param[out]  FoundSection    The FFS section if found

  @retval EFI_SUCCESS           The file and section was found
  @retval EFI_NOT_FOUND         The file and section was not found
  @retval EFI_VOLUME_CORRUPTED  The firmware volume was corrupted

**/
EFI_STATUS
FindFfsSectionInSections (
  IN  VOID                             *Sections,
  IN  UINTN                            SizeOfSections,
  IN  EFI_SECTION_TYPE                 SectionType,
  OUT EFI_COMMON_SECTION_HEADER        **FoundSection
  )
{
  EFI_PHYSICAL_ADDRESS        CurrentAddress;
  UINT32                      Size;
  EFI_PHYSICAL_ADDRESS        EndOfSections;
  EFI_COMMON_SECTION_HEADER   *Section;
  EFI_PHYSICAL_ADDRESS        EndOfSection;

  //
  // Loop through the FFS file sections within the PEI Core FFS file
  //
  EndOfSection = (EFI_PHYSICAL_ADDRESS)(UINTN) Sections;
  EndOfSections = EndOfSection + SizeOfSections;
  for (;;) {
    if (EndOfSection == EndOfSections) {
      break;
    }
    CurrentAddress = (EndOfSection + 3) & ~(3ULL);
    if (CurrentAddress >= EndOfSections) {
      return EFI_VOLUME_CORRUPTED;
    }

    Section = (EFI_COMMON_SECTION_HEADER*)(UINTN) CurrentAddress;
    DEBUG_WRITE_STRING ("Section->Type: ");
    DEBUG_WRITE_UINT32 (Section->Type);
    DEBUG_WRITE_STRING ("\n");

    Size = SECTION_SIZE (Section);
    if (Size < sizeof (*Section)) {
      return EFI_VOLUME_CORRUPTED;
    }

    EndOfSection = CurrentAddress + Size;
    if (EndOfSection > EndOfSections) {
      return EFI_VOLUME_CORRUPTED;
    }

    //
    // Look for the requested section type
    //
    if (Section->Type == SectionType) {
      *FoundSection = Section;
      return EFI_SUCCESS;
    }
    DEBUG_WRITE_STRING ("Section->Type (");
    DEBUG_WRITE_UINT32 (Section->Type);
    DEBUG_WRITE_STRING (") != SectionType (");
    DEBUG_WRITE_UINT32 (SectionType);
    DEBUG_WRITE_STRING (")\n");
  }

  return EFI_NOT_FOUND;
}

/**
  Locates a FFS file with the specified file type and a section
  within that file with the specified section type.

  @param[in]   Fv            The firmware volume to search
  @param[in]   FileType      The file type to locate
  @param[in]   SectionType   The section type to locate
  @param[out]  FoundSection  The FFS section if found

  @retval EFI_SUCCESS           The file and section was found
  @retval EFI_NOT_FOUND         The file and section was not found
  @retval EFI_VOLUME_CORRUPTED  The firmware volume was corrupted

**/
EFI_STATUS
EFIAPI
FindFfsFileAndSection (
  IN  EFI_FIRMWARE_VOLUME_HEADER       *Fv,
  IN  EFI_FV_FILETYPE                  FileType,
  IN  EFI_SECTION_TYPE                 SectionType,
  OUT EFI_COMMON_SECTION_HEADER        **FoundSection
  )
{
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        CurrentAddress;
  EFI_PHYSICAL_ADDRESS        EndOfFirmwareVolume;
  EFI_FFS_FILE_HEADER         *File;
  UINT32                      Size;
  EFI_PHYSICAL_ADDRESS        EndOfFile;

  if (Fv->Signature != EFI_FVH_SIGNATURE) {
    DEBUG_WRITE_STRING ("FV at ");
    DEBUG_WRITE_UINT32 ((UINT32)(UINTN)Fv);
    DEBUG_WRITE_STRING (" does not have FV header signature\n");
    return EFI_VOLUME_CORRUPTED;
  }

  CurrentAddress = (EFI_PHYSICAL_ADDRESS)(UINTN) Fv;
  EndOfFirmwareVolume = CurrentAddress + Fv->FvLength;

  //
  // Loop through the FFS files in the Boot Firmware Volume
  //
  for (EndOfFile = CurrentAddress + Fv->HeaderLength; ; ) {

    CurrentAddress = (EndOfFile + 7) & ~(7ULL);
    if (CurrentAddress > EndOfFirmwareVolume) {
      return EFI_VOLUME_CORRUPTED;
    }

    File = (EFI_FFS_FILE_HEADER*)(UINTN) CurrentAddress;
    Size = *(UINT32*) File->Size & 0xffffff;
    if (Size < (sizeof (*File) + sizeof (EFI_COMMON_SECTION_HEADER))) {
      return EFI_VOLUME_CORRUPTED;
    }
    DEBUG_WRITE_STRING ("File->Type: ");
    DEBUG_WRITE_UINT32 (File->Type);
    DEBUG_WRITE_STRING ("\n");

    EndOfFile = CurrentAddress + Size;
    if (EndOfFile > EndOfFirmwareVolume) {
      return EFI_VOLUME_CORRUPTED;
    }

    //
    // Look for the request file type
    //
    if (File->Type != FileType) {
      DEBUG_WRITE_STRING ("File->Type (");
      DEBUG_WRITE_UINT32 (File->Type);
      DEBUG_WRITE_STRING (") != FileType (");
      DEBUG_WRITE_UINT32 (FileType);
      DEBUG_WRITE_STRING (")\n");
      continue;
    }

    Status = FindFfsSectionInSections (
               (VOID*) (File + 1),
               (UINTN) EndOfFile - (UINTN) (File + 1),
               SectionType,
               FoundSection
               );
    if (!EFI_ERROR (Status) || (Status == EFI_VOLUME_CORRUPTED)) {
      return Status;
    }
  }
}

/**
  Locates the compressed main firmware volume and decompresses it.

  @param[in,out]  Fv            On input, the firmware volume to search
                                On output, the decompressed main FV

  @retval EFI_SUCCESS           The file and section was found
  @retval EFI_NOT_FOUND         The file and section was not found
  @retval EFI_VOLUME_CORRUPTED  The firmware volume was corrupted

**/
EFI_STATUS
EFIAPI
DecompressGuidedFv (
  IN OUT EFI_FIRMWARE_VOLUME_HEADER       **Fv,
  IN UINTN                                DecompressBase
  )
{
	EFI_STATUS                        Status;
  EFI_GUID_DEFINED_SECTION          *Section;
  UINT32                            OutputBufferSize;
  UINT32                            ScratchBufferSize;
  UINT16                            SectionAttribute;
  UINT32                            AuthenticationStatus;
  VOID                              *OutputBuffer;
  VOID                              *ScratchBuffer;
  EFI_FIRMWARE_VOLUME_IMAGE_SECTION *NewFvSection;
  EFI_FIRMWARE_VOLUME_HEADER        *NewFv;

  DEBUG_WRITE_STRING ("DecompressGuidedFv - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)*Fv);
  DEBUG_WRITE_STRING ("\n");
  
  DEBUG_WRITE_STRING ("FindFfsFileAndSection ...\n");

	Status = FindFfsFileAndSection (
             *Fv,
             EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE,
             EFI_SECTION_GUID_DEFINED,
             (EFI_COMMON_SECTION_HEADER**) &Section
             );
  if (EFI_ERROR (Status)) {
    DEBUG_WRITE_STRING ("Unable to find GUID defined section\n");
    return Status;
  }
  
  DEBUG_WRITE_STRING ("ExtractGuidedSectionGetInfo ...\n");

  Status = ExtractGuidedSectionGetInfo (
             Section,
             &OutputBufferSize,
             &ScratchBufferSize,
             &SectionAttribute
             );
  if (EFI_ERROR (Status)) {
    DEBUG_WRITE_STRING ("Unable to GetInfo for GUIDed section\n");
    return Status;
  }
  
  OutputBuffer = (VOID *)(DecompressBase + 0x20000);
  
  ScratchBuffer = (VOID *)(DecompressBase);
  
  DEBUG_WRITE_STRING ("ExtractGuidedSectionDecode ...\n");
  
  Status = ExtractGuidedSectionDecode (
             Section,
             &OutputBuffer,
             ScratchBuffer,
             &AuthenticationStatus
             );
  if (EFI_ERROR (Status)) {
    DEBUG_WRITE_STRING ("Error during GUID section decode\n");
    return Status;
  }
  
  DEBUG_WRITE_STRING ("FindFfsSectionInSections ...\n");
  
  Status = FindFfsSectionInSections (
             OutputBuffer,
             OutputBufferSize,
             EFI_SECTION_FIRMWARE_VOLUME_IMAGE,
             (EFI_COMMON_SECTION_HEADER**) &NewFvSection
             );
  if (EFI_ERROR (Status)) {
    DEBUG_WRITE_STRING ("Unable to find FV image in extracted data\n");
    return Status;
  }
  
  NewFv = (EFI_FIRMWARE_VOLUME_HEADER *)(NewFvSection + 1);

  DEBUG_WRITE_STRING ("Find NewFv - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)NewFv);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("  FvLength - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)NewFv->FvLength);
  DEBUG_WRITE_STRING ("\n");

  CopyMem (ScratchBuffer, NewFv, (UINTN)NewFv->FvLength);
  
  *Fv = (EFI_FIRMWARE_VOLUME_HEADER *)ScratchBuffer;
  
	return EFI_SUCCESS;
}

/**
  Find core image base.
  
  @param   BootFirmwareVolumePtr    Point to the boot firmware volume.
  @param   SecCoreImageBase         The base address of the SEC core image.
  @param   PeiCoreImageBase         The base address of the PEI core image.

**/
EFI_STATUS
EFIAPI
FindDxeCoreImage (
  IN  EFI_FIRMWARE_VOLUME_HEADER       *BootFirmwareVolumePtr,
  OUT EFI_PHYSICAL_ADDRESS             *DxeCoreImageBase,
  OUT UINT64                           *DxeCoreLength
  )
{
  EFI_PHYSICAL_ADDRESS        CurrentAddress;
  EFI_PHYSICAL_ADDRESS        EndOfFirmwareVolume;
  EFI_FFS_FILE_HEADER         *File;
  UINT32                      Size;
  EFI_PHYSICAL_ADDRESS        EndOfFile;
  EFI_COMMON_SECTION_HEADER   *Section;
  EFI_PHYSICAL_ADDRESS        EndOfSection;

  *DxeCoreImageBase = 0;

  CurrentAddress = (EFI_PHYSICAL_ADDRESS)(UINTN) BootFirmwareVolumePtr;
  EndOfFirmwareVolume = CurrentAddress + BootFirmwareVolumePtr->FvLength;

  //
  // Loop through the FFS files in the Boot Firmware Volume
  //
  for (EndOfFile = CurrentAddress + BootFirmwareVolumePtr->HeaderLength; ; ) {

    CurrentAddress = (EndOfFile + 7) & 0xfffffffffffffff8ULL;
    if (CurrentAddress > EndOfFirmwareVolume) {
      return EFI_NOT_FOUND;
    }

    File = (EFI_FFS_FILE_HEADER*)(UINTN) CurrentAddress;
    Size = FFS_FILE_SIZE (File);
    if (Size < sizeof (EFI_FFS_FILE_HEADER)) {
      return EFI_NOT_FOUND;
    }

    EndOfFile = CurrentAddress + Size;
    if (EndOfFile > EndOfFirmwareVolume) {
      return EFI_NOT_FOUND;
    }

    //
    // Look for SEC Core / PEI Core files
    //
    if (File->Type != EFI_FV_FILETYPE_DXE_CORE) {
      continue;
    }

    DEBUG_WRITE_STRING ("DxeFile - ");
    DEBUG_WRITE_UINT32 ((UINT32)(UINTN)File);
    DEBUG_WRITE_STRING ("\n");

    //
    // Loop through the FFS file sections within the FFS file
    //
    EndOfSection = (EFI_PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) File + sizeof (EFI_FFS_FILE_HEADER));
    for (;;) {
      CurrentAddress = (EndOfSection + 3) & 0xfffffffffffffffcULL;
      Section = (EFI_COMMON_SECTION_HEADER*)(UINTN) CurrentAddress;

      Size = SECTION_SIZE (Section);
      if (Size < sizeof (EFI_COMMON_SECTION_HEADER)) {
        return EFI_NOT_FOUND;
      }

      EndOfSection = CurrentAddress + Size;
      if (EndOfSection > EndOfFile) {
        return EFI_NOT_FOUND;
      }

      //
      // Look for executable sections
      //
      if (Section->Type == EFI_SECTION_PE32 || Section->Type == EFI_SECTION_TE) {
        DEBUG_WRITE_STRING ("DxeSection - ");
        DEBUG_WRITE_UINT32 ((UINT32)(UINTN)Section);
        DEBUG_WRITE_STRING ("\n");
        *DxeCoreImageBase = (PHYSICAL_ADDRESS) (UINTN) ((UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER));
        *DxeCoreLength = Size - sizeof (EFI_COMMON_SECTION_HEADER);
        break;
      }
    }

    //
    // Both SEC Core and PEI Core images found
    //
    if (*DxeCoreImageBase != 0) {
      return EFI_SUCCESS;
    }
  }
}

VOID *
FindFirstImageInFv (
  IN  EFI_FIRMWARE_VOLUME_HEADER *FvHeader,
  OUT UINTN                      *Size
  )
{
  EFI_FFS_FILE_HEADER            *FfsHeader;
  EFI_PE32_SECTION               *SectionHeader;
  EFI_FIRMWARE_VOLUME_EXT_HEADER *ExtHeader;

  if (FvHeader->ExtHeaderOffset == 0) {
    FfsHeader = (EFI_FFS_FILE_HEADER *)((UINTN)FvHeader + FvHeader->HeaderLength);
  } else {
    ExtHeader = (EFI_FIRMWARE_VOLUME_EXT_HEADER *)((UINTN)FvHeader + FvHeader->ExtHeaderOffset);
    FfsHeader = (EFI_FFS_FILE_HEADER *)((UINTN)ExtHeader + ExtHeader->ExtHeaderSize);
    FfsHeader = (EFI_FFS_FILE_HEADER *)(((UINTN)FfsHeader + 7) & (~7UL));
  }
  SectionHeader = (EFI_PE32_SECTION *)(FfsHeader + 1);

  if ((SectionHeader->Type != EFI_SECTION_TE) && (SectionHeader->Type != EFI_SECTION_PE32)) {
    DEBUG_WRITE_STRING ("SectionHeader->Type Incorrect\n");
    CpuDeadLoop ();
  }

  *Size = SECTION_SIZE(SectionHeader) - sizeof(EFI_COMMON_SECTION_HEADER);

  return (VOID *)(SectionHeader + 1);
}
