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

#include <Ppi/SecPlatformInformation.h>
#include <Ppi/TemporaryRamSupport.h>

#include <Library/BaseLib.h>
#include <Library/SerialDebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PlatformSecLib.h>
#include <Library/UefiCpuLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PeCoffExtraActionLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PeRelocationLib.h>
#include <Library/FvLib.h>

EFI_STATUS
LzmaArchDecompressLibConstructor (
  VOID
  );

typedef VOID (EFIAPI *PLATFORM_BOOT_IMAGE) (VOID);

/** Copy boot image to sram and call it.

**/
STATIC
VOID
SramCall (
  VOID
  )
{
  UINT8                             *ImageBase;
  UINTN                             ImageSize;
  PLATFORM_BOOT_IMAGE               BootImage;

  ImageBase = FindFirstImageInFv (
                (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)(FixedPcdGet32 (PcdEsramStage1Base) + FixedPcdGet32(PcdPlatformFlashFvBootRomSize)),
                &ImageSize
                );

  DEBUG_WRITE_STRING ("SramTrampolineCall - \n");
  DEBUG_WRITE_STRING ("FvBase - ");
  DEBUG_WRITE_UINT32 (FixedPcdGet32 (PcdEsramStage1Base) + FixedPcdGet32(PcdPlatformFlashFvBootRomSize));
  DEBUG_WRITE_STRING ("\n");
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

/**

  Entry point to the C language phase of SEC. After the SEC assembly
  code has initialized some temporary memory and set up the stack,
  the control is transferred to this function.


  @param SizeOfRam           Size of the temporary memory available for use.
  @param TempRamBase         Base address of tempory ram
  @param BootFirmwareVolume  Base address of the Boot Firmware Volume.
**/
VOID
EFIAPI
SecStartup (
  IN UINT32                   SizeOfRam,
  IN UINT32                   TempRamBase,
  IN VOID                     *BootFirmwareVolume
  )
{
  EFI_FIRMWARE_VOLUME_HEADER       *Fv;
  EFI_STATUS                       Status;

  DEBUG_WRITE_STRING ("SecTrampoline\n");

  LzmaArchDecompressLibConstructor ();

  Fv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)BootFirmwareVolume;
  Status = DecompressGuidedFv (&Fv, (UINTN) FixedPcdGet32(PcdEsramStage1Base) + FixedPcdGet32(PcdPlatformFlashFvBootRomSize));
  if (EFI_ERROR (Status)) {
    CpuDeadLoop ();
  }

  //
  // Copy MFH specified image to SRAM and call it.
  //
  SramCall ();
}

VOID
EFIAPI
DummyEntrypoint (
  IN CONST EFI_SEC_PEI_HAND_OFF        *SecCoreDataPtr,
  IN CONST EFI_PEI_PPI_DESCRIPTOR      *PpiList,
  IN VOID                              *Data
  )
{
}