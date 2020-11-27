/** @file
  Relocate TE image.

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

#include "Uefi.h"
#include <IndustryStandard/PeImage.h>
#include <Library/DebugLib.h>

/**

  This function relocate image at ImageBase.

  @param ImageBase   Image base
  @param PeImageBase Image base field in PE/COFF header

**/
VOID
PeCoffRelocateImageOnTheSpot (
  IN  UINTN        ImageBase,
  IN  UINTN        PeImageBase
  )
{
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION Hdr;
  EFI_IMAGE_DATA_DIRECTORY            *RelocDir;
  EFI_IMAGE_BASE_RELOCATION           *RelocBase;
  EFI_IMAGE_BASE_RELOCATION           *RelocBaseEnd;
  UINT16                              *Reloc;
  UINT16                              *RelocEnd;
  CHAR8                               *Fixup;
  CHAR8                               *FixupBase;
  UINT16                              *Fixup16;
  UINT32                              *Fixup32;
  UINT64                              *Fixup64;
  UINTN                               Adjust;
  UINT16                              TeStrippedOffset;

  //
  // Find the image's relocate dir info
  //
  Hdr.Te = (EFI_TE_IMAGE_HEADER *)ImageBase;
  if (Hdr.Te->Signature != EFI_TE_IMAGE_HEADER_SIGNATURE) {
    //
    // Not a valid PE image so Exit
    //
    return ;
  }
  TeStrippedOffset   = (UINT32)Hdr.Te->StrippedSize - sizeof (EFI_TE_IMAGE_HEADER);
  Adjust             = (ImageBase - PeImageBase - TeStrippedOffset);

  //
  // Find the relocation block
  //
  RelocDir = &Hdr.Te->DataDirectory[0];

  RelocBase     = (EFI_IMAGE_BASE_RELOCATION *)(UINTN)(ImageBase + RelocDir->VirtualAddress - TeStrippedOffset);
  RelocBaseEnd  = (EFI_IMAGE_BASE_RELOCATION *)(UINTN)(ImageBase + RelocDir->VirtualAddress + RelocDir->Size - TeStrippedOffset);
  
  //
  // ASSERT for the invalid image when RelocBase and RelocBaseEnd are both NULL.
  //
  ASSERT (RelocBase != NULL && RelocBaseEnd != NULL);

  //
  // Run the whole relocation block. And re-fixup data that has not been
  // modified. The FixupData is used to see if the image has been modified
  // since it was relocated. This is so data sections that have been updated
  // by code will not be fixed up, since that would set them back to
  // defaults.
  //
  while (RelocBase < RelocBaseEnd) {

    Reloc     = (UINT16 *) ((UINT8 *) RelocBase + sizeof (EFI_IMAGE_BASE_RELOCATION));
    RelocEnd  = (UINT16 *) ((UINT8 *) RelocBase + RelocBase->SizeOfBlock);
    FixupBase = (CHAR8 *) ((UINTN)ImageBase) + RelocBase->VirtualAddress - TeStrippedOffset;

    //
    // Run this relocation record
    //
    while (Reloc < RelocEnd) {

      Fixup = FixupBase + (*Reloc & 0xFFF);
      switch ((*Reloc) >> 12) {

      case EFI_IMAGE_REL_BASED_ABSOLUTE:
        break;

      case EFI_IMAGE_REL_BASED_HIGH:
        Fixup16  = (UINT16 *) Fixup;
        *Fixup16 = (UINT16) (*Fixup16 + ((UINT16) ((UINT32) Adjust >> 16)));
        break;

      case EFI_IMAGE_REL_BASED_LOW:
        Fixup16  = (UINT16 *) Fixup;
        *Fixup16 = (UINT16) (*Fixup16 + ((UINT16) Adjust & 0xffff));
        break;

      case EFI_IMAGE_REL_BASED_HIGHLOW:
        Fixup32  = (UINT32 *) Fixup;
        *Fixup32 = *Fixup32 + (UINT32) Adjust;
        break;

      case EFI_IMAGE_REL_BASED_DIR64:
        Fixup64  = (UINT64 *) Fixup;
        *Fixup64 = *Fixup64 + (UINT64)Adjust;
        break;

      case EFI_IMAGE_REL_BASED_HIGHADJ:
        //
        // Not valid Relocation type for UEFI image, ASSERT
        //
        ASSERT (FALSE);
        break;

      default:
        //
        // Only Itanium requires ConvertPeImage_Ex
        //
        ASSERT (FALSE);
        break;
      }
      //
      // Next relocation record
      //
      Reloc += 1;
    }
    //
    // next reloc block
    //
    RelocBase = (EFI_IMAGE_BASE_RELOCATION *) RelocEnd;
  }
}

/**

  This function relocate this image.

**/
VOID
RelocateImage (
  IN UINT8   *ImageBase
  )
{
  UINTN                               PeImageBase;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION Hdr;
  UINT16                              TeStrippedOffset;
  
  //
  // Find the image's relocate dir info
  //
  Hdr.Te = (EFI_TE_IMAGE_HEADER *)ImageBase;
  if (Hdr.Te->Signature != EFI_TE_IMAGE_HEADER_SIGNATURE) {
    //
    // Not a valid PE image so Exit
    //
    return ;
  }

  TeStrippedOffset = Hdr.Te->StrippedSize - sizeof (EFI_TE_IMAGE_HEADER);
  PeImageBase = (UINTN)Hdr.Te->ImageBase;

  //
  // This is self-contain PE-COFF loader.
  //
  PeCoffRelocateImageOnTheSpot ((UINTN)ImageBase, PeImageBase);

  AsmWbinvd ();

  //
  // Set value indicate we have already relocated
  //
  Hdr.Te->ImageBase = (UINTN)ImageBase - TeStrippedOffset;

  return ;
}