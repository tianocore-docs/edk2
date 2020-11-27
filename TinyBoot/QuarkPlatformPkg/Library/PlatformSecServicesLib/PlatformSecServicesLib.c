/** @file

Copyright(c) 2013 Intel Corporation. All rights reserved.

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

Module Name:

  PlatformSecServicesLib.c

Abstract:

  Provide platform specific services to QuarkResetVector & QuarkSecLib.

--*/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/IntelQNCLib.h>
#include <Library/QNCAccessLib.h>
#include <Platform.h>
#include <Library/SerialDebugLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeRelocationLib.h>
#include <Library/FvLib.h>

typedef VOID (EFIAPI *PLATFORM_BOOT_IMAGE) (VOID);

//
// Routines local to this source module.
//

/** Copy boot image to sram and call it.

**/
STATIC
VOID
CopyImageToSramAndCall (
  IN CONST UINT32                         FlashAddress,
  IN CONST UINT32                         LengthBytes
  )
{
  UINT32                            *Dest;
  UINT32                            *Src;
  UINT32                            SramStage1Base;
  UINT8                             *ImageBase;
  UINTN                             ImageSize;
  PLATFORM_BOOT_IMAGE               BootImage;

  //
  // Copy to SRAM
  //
  SramStage1Base = FixedPcdGet32 (PcdEsramStage1Base);
  Dest = (UINT32 *) SramStage1Base;
  Src = (UINT32 *) FlashAddress;
  CopyMem (Dest, Src, LengthBytes);

  //
  // Prepare to jump
  //
  ImageBase = FindFirstImageInFv (
                (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)(FixedPcdGet32 (PcdEsramStage1Base)),
                &ImageSize
                );
  DEBUG_WRITE_STRING ("Image - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)ImageBase);
  DEBUG_WRITE_STRING (" - ");
  DEBUG_WRITE_UINT32 (ImageSize);
  DEBUG_WRITE_STRING ("\n");

  PeCoffLoaderGetEntryPoint (ImageBase, (VOID**)&BootImage);

  if (FeaturePcdGet (PcdNoToolRebase)) {
    DEBUG_WRITE_STRING ("RelocateImage ...\n");
    RelocateImage (ImageBase);
    DEBUG_WRITE_STRING ("RelocateImage Done\n");
  }

  DEBUG_WRITE_STRING ("BootImage - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)BootImage);
  DEBUG_WRITE_STRING ("\n");

  //
  // Boot to image at FlashAddress.
  //
  BootImage ();

  //
  // We should never return if we do then loop forever.
  //
  CpuDeadLoop();
}

//
// Routines exported by this source module.
//

/** Copy boot stage1 image to sram and Call it.

  This routine is called from the QuarkResetVector to copy stage1 image to SRAM
  the routine should NOT be called from QuarkSecLib.

**/
VOID
EFIAPI
PlatformCopyBootStage1ImageToSramAndCall (
  VOID
  )
{
  DEBUG_WRITE_STRING ("PlatformCopyBootStage1ImageToSramAndCall\n");

  //
  // Copy MFH specified image to SRAM and call it.
  //
  CopyImageToSramAndCall (
    FixedPcdGet32(PcdPlatformFlashFvBootRomBase),
    FixedPcdGet32(PcdPlatformFlashFvBootRomSize)
    );
}
