/** @file
  Null instance of Sec Platform Hook Lib.

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


**/

#ifndef  _QUARK_SEC_LIB_H_
#define  _QUARK_SEC_LIB_H_

#include <PiPei.h>

#include <Ppi/SecPlatformInformation.h>
#include <Ppi/TemporaryRamSupport.h>
#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/SerialDebugLib.h>
#include <Library/BaseMemoryLib.h>

#include <Ppi/CltMemoryInit.h>
#include <Ppi/Pcd.h>
#include <Ppi/PiPcd.h>
#include <Library/PciLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/FvLib.h>

#include <Library/MtrrLib.h>

#include <QuarkNcSocId.h>
#include <Library/QNCAccessLib.h>
#include <Library/IntelQNCLib.h>

typedef struct {
  EFI_FIRMWARE_VOLUME_HEADER *BootFirmwareVolumeBase;
  VOID                  *MemoryTop;
  VOID                  *DxeCoreEntryPoint;
  EFI_PHYSICAL_ADDRESS  DxeCoreImageBase;
  UINT64                DxeCoreLength;
} HAND_OFF_INFO;

VOID *
EFIAPI
SecCpuInitTimer (
  VOID
  );

VOID
CopyRomFv (
  IN OUT   VOID        *SecCoreData
  );

RETURN_STATUS
EFIAPI
IntelQNCAcpiTimerLibConstructor (
   VOID
   );

void Mrc( MRCParams_t *mrc_params);

#endif







