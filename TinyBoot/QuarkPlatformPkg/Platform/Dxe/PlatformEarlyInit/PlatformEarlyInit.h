/** @file
  The header file of Platform PEIM.

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


#ifndef __PLATFORM_EARLY_INIT_H__
#define __PLATFORM_EARLY_INIT_H__

#define PEI_STALL_RESOLUTION            1
#define STALL_PEIM_SIGNATURE   SIGNATURE_32('p','p','u','s')

//
// USB Phy Registers
//
#define USB2_GLOBAL_PORT  0x4001
#define USB2_PLL1         0x7F02
#define USB2_PLL2         0x7F03
#define USB2_COMPBG       0x7F04

/** Return info derived from Installing Memory by MemoryInit.

  @param[out]      CmcBaseAddressPtr   Return CmcBaseAddress to this location.
  @param[out]      SmramDescriptorPtr  Return start of Smram descriptor list to this location.
  @param[out]      NumSmramRegionsPtr  Return numbers of Smram regions to this location.

  @return Address of CMC region at the top of available memory.
  @return List of Smram descriptors for each Smram region.
  @return Numbers of Smram regions.
**/
VOID
EFIAPI
InfoPostInstallMemory (
  OUT     UINT32                    *CmcBaseAddressPtr OPTIONAL,
  OUT     EFI_SMRAM_DESCRIPTOR      **SmramDescriptorPtr OPTIONAL,
  OUT     UINTN                     *NumSmramRegionsPtr OPTIONAL
  );

VOID
EFIAPI
InitializeUSBPhy (
    VOID
   );

/**
  This function provides early platform initialisation.
  It prints the Stage1 module that's running if this is
  a debug build.
**/
VOID
EFIAPI
EarlyPlatformInit (
  VOID
  );

/**
  This function provides early platform GPIO initialisation.
**/
VOID
EFIAPI
EarlyPlatformGpioInit (
  VOID
  );

#endif
