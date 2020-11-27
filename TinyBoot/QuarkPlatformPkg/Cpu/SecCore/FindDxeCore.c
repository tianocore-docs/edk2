/** @file
  Locate the entry point for the DXE Core

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
#include <Library/PeCoffGetEntryPointLib.h>

#include <Library/BaseLib.h>
#include <Library/SerialDebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/PeRelocationLib.h>
#include <Library/FvLib.h>

/**
  Find and return Pei Core entry point.

  It also find SEC and PEI Core file debug inforamtion. It will report them if
  remote debug is enabled.
  
  @param   BootFirmwareVolumePtr    Point to the boot firmware volume.
  @param   PeiCoreEntryPoint        The entry point of the PEI core.

**/
VOID
EFIAPI
FindAndReportEntryPoints (
  IN  EFI_FIRMWARE_VOLUME_HEADER       *BootFirmwareVolumePtr,
  OUT VOID                             **DxeCoreEntryPoint,
  OUT EFI_PHYSICAL_ADDRESS             *DxeCoreImageBase,
  OUT UINT64                           *DxeCoreLength
  )
{
  EFI_STATUS                       Status;

  //
  // Find DXE Core image base
  //
  DEBUG_WRITE_STRING ("FindDxeCoreImage - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)BootFirmwareVolumePtr);
  DEBUG_WRITE_STRING ("\n");
  Status = FindDxeCoreImage (BootFirmwareVolumePtr, DxeCoreImageBase, DxeCoreLength);
  if (EFI_ERROR (Status)) {
    DEBUG_WRITE_STRING ("FindDxeCoreImage - ");
    DEBUG_WRITE_UINT32 (Status);
    DEBUG_WRITE_STRING ("\n");
    CpuDeadLoop ();
  }
  DEBUG_WRITE_STRING ("DxeCoreImageBase - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)*DxeCoreImageBase);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("DxeCoreLength - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)*DxeCoreLength);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("RelocateImage ...\n");
  RelocateImage ((UINT8 *)(UINTN)*DxeCoreImageBase);
  DEBUG_WRITE_STRING ("RelocateImage Done\n");

  //
  // Find DXE Core entry point
  //
  Status = PeCoffLoaderGetEntryPoint ((VOID *) (UINTN) *DxeCoreImageBase, (VOID**) DxeCoreEntryPoint);
  if (EFI_ERROR (Status)) {
    *DxeCoreEntryPoint = NULL;
  }

  DEBUG_WRITE_STRING ("DxeCoreEntryPoint - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)*DxeCoreEntryPoint);
  DEBUG_WRITE_STRING ("\n");

  return;
}

