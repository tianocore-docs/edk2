/*++

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

  PlatformInitDxe.c

Abstract:

  Platform init DXE driver for this platform.

--*/

//
// Statements that include other files
//
#include "PlatformInitDxe.h"

STATIC
VOID
EFIAPI
LegacySpiProtect (
  VOID
  )
{
  UINT32                            RegVal;

  RegVal = FixedPcdGet32 (PcdLegacyProtectedBIOSRange0Pei);
  if (RegVal != 0) {

    PlatformWriteFirstFreeSpiProtect (
      RegVal,
      0,
      0
      );

  }
  RegVal = FixedPcdGet32 (PcdLegacyProtectedBIOSRange1Pei);
  if (RegVal != 0) {
    PlatformWriteFirstFreeSpiProtect (
      RegVal,
      0,
      0
      );
  }
  RegVal = FixedPcdGet32 (PcdLegacyProtectedBIOSRange2Pei);
  if (RegVal != 0) {
    PlatformWriteFirstFreeSpiProtect (
      RegVal,
      0,
      0
      );
  }

  //
  // Make legacy SPI READ/WRITE enabled if not a secure build
  //
  if(FeaturePcdGet (PcdEnableSecureLock)) {
    LpcPciCfg32And (R_QNC_LPC_BIOS_CNTL, ~B_QNC_LPC_BIOS_CNTL_BIOSWE);
  } else {
    LpcPciCfg32Or (R_QNC_LPC_BIOS_CNTL, B_QNC_LPC_BIOS_CNTL_BIOSWE);
  }

}

EFI_STATUS
EFIAPI
PlatformInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
/*++

Routine Description:
  Entry point for the driver.

Arguments:

  ImageHandle  -  Image Handle.
  SystemTable  -  EFI System Table.
  
Returns:

  EFI_SUCCESS  -  Function has completed successfully.

--*/
{
  //
  // Protect areas specified by PCDs.
  //
  LegacySpiProtect ();

  //
  // Create events for configuration callbacks.
  //
  CreateConfigEvents ();

  return EFI_SUCCESS;
}

