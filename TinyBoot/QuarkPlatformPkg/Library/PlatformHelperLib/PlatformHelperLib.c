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

  PlatformHelperLib.c

Abstract:

  Helper routines with common PEI / DXE implementation.

--*/

#include "CommonHeader.h"

//
// Routines defined in other source modules of this component.
//

//
// Routines local to this source module.
//

//
// Routines shared with other souce modules in this component.
//

EFI_STATUS
WriteFirstFreeSpiProtect (
  IN CONST UINT32                         PchRootComplexBar,
  IN CONST UINT32                         DirectValue,
  IN CONST UINT32                         BaseAddress,
  IN CONST UINT32                         Length,
  OUT UINT32                              *OffsetPtr
  )
{
  UINT32                            RegVal;
  UINT32                            Offset;
  UINT32                            StepLen;

  ASSERT (PchRootComplexBar > 0);

  Offset = 0;
  if (OffsetPtr != NULL) {
    *OffsetPtr = Offset;
  }
  if (MmioRead32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR0) == 0) {
    Offset = R_QNC_RCRB_SPIPBR0;
  } else {
    if (MmioRead32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR1) == 0) {
      Offset = R_QNC_RCRB_SPIPBR1;
    } else {
      if (MmioRead32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR2) == 0) {
        Offset = R_QNC_RCRB_SPIPBR2;
      }
    }
  }
  if (Offset != 0) {
    if (DirectValue == 0) {
      StepLen = ALIGN_LENGTH (Length,0x1000);   // Bring up to 4K boundary.
      RegVal = BaseAddress + StepLen - 1;
      RegVal &= 0x00FFF000;                     // Set EDS Protected Range Limit (PRL).
      RegVal |= ((BaseAddress >> 12) & 0xfff);  // or in EDS Protected Range Base (PRB).
    } else {
      RegVal = DirectValue;
    }
    //
    // Enable protection.
    //
    RegVal |= B_QNC_RCRB_SPIPBRn_WPE;
    MmioWrite32 (PchRootComplexBar + Offset, RegVal);
    if (RegVal == MmioRead32 (PchRootComplexBar + Offset)) {
      if (OffsetPtr != NULL) {
        *OffsetPtr = Offset;
      }
      return EFI_SUCCESS;
    }
    return EFI_DEVICE_ERROR;
  }
  return EFI_NOT_FOUND;
}

//
// Routines exported by this component.
//

/**
  Platform assert PCI express PERST# signal.

**/
VOID
EFIAPI
PlatformPERSTAssert (
  VOID
  )
{
  UINT32                            RegValue;
  UINT32                            GpioBaseAddress;

  GpioBaseAddress = LpcPciCfg32 (R_QNC_LPC_GBA_BASE) & B_QNC_LPC_GPA_BASE_MASK;
  ASSERT (GpioBaseAddress > 0);

  RegValue = IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGLVL_RESUME_WELL);

  RegValue &= ~(1 << PCIEXP_PERST_RESUMEWELL_GPIO);

  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGLVL_RESUME_WELL, RegValue);
}

/**
  Platform de assert PCI express PERST# signal.

**/
VOID
EFIAPI
PlatformPERSTDeAssert (
  VOID
  )
{
  UINT32                            RegValue;
  UINT32                            GpioBaseAddress;

  GpioBaseAddress = LpcPciCfg32 (R_QNC_LPC_GBA_BASE) & B_QNC_LPC_GPA_BASE_MASK;
  ASSERT (GpioBaseAddress > 0);

  RegValue = IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGLVL_RESUME_WELL);

  RegValue |= (1 << PCIEXP_PERST_RESUMEWELL_GPIO);

  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGLVL_RESUME_WELL, RegValue);
}

/**
  Clear SPI Protect registers.

  @retval EFI_SUCCESS        SPI protect registers cleared.
  @retval EFI_ACCESS_DENIED  Unable to clear SPI protect registers.
**/

EFI_STATUS
EFIAPI
PlatformClearSpiProtect (
  VOID
  )
{
  UINT32                            PchRootComplexBar;

  PchRootComplexBar = QNC_RCRB_BASE;
  //
  // Check if the SPI interface has been locked-down.
  //
  if ((MmioRead16 (PchRootComplexBar + R_QNC_RCRB_SPIS) & B_QNC_RCRB_SPIS_SCL) != 0) {
    return EFI_ACCESS_DENIED;
  }
  MmioWrite32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR0, 0);
  if (MmioRead32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR0) != 0) {
    return EFI_ACCESS_DENIED;
  }
  MmioWrite32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR1, 0);
  if (MmioRead32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR0) != 0) {
    return EFI_ACCESS_DENIED;
  }
  MmioWrite32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR2, 0);
  if (MmioRead32 (PchRootComplexBar + R_QNC_RCRB_SPIPBR0) != 0) {
    return EFI_ACCESS_DENIED;
  }
  return EFI_SUCCESS;
}

/**
  Determine if an SPI address range is protected.

  @param  SpiBaseAddress  Base of SPI range.
  @param  Length          Length of SPI range.

  @retval TRUE       Range is protected.
  @retval FALSE      Range is not protected.
**/
BOOLEAN
EFIAPI
PlatformIsSpiRangeProtected (
  IN CONST UINT32                         SpiBaseAddress,
  IN CONST UINT32                         Length
  )
{
  UINT32                            RegVal;
  UINT32                            Offset;
  UINT32                            Limit;
  UINT32                            ProtectedBase;
  UINT32                            ProtectedLimit;
  UINT32                            PchRootComplexBar;

  PchRootComplexBar = QNC_RCRB_BASE;

  if (Length > 0) {
    Offset = R_QNC_RCRB_SPIPBR0;
    Limit = SpiBaseAddress + (Length - 1);
    do {
      RegVal = MmioRead32 (PchRootComplexBar + Offset);
      if ((RegVal & B_QNC_RCRB_SPIPBRn_WPE) != 0) {
        ProtectedBase = (RegVal & 0xfff) << 12;
        ProtectedLimit = (RegVal & 0x00fff000) + 0xfff;
        if (SpiBaseAddress >= ProtectedBase && Limit <= ProtectedLimit) {
          return TRUE;
        }
      }
      if (Offset == R_QNC_RCRB_SPIPBR0) {
        Offset = R_QNC_RCRB_SPIPBR1;
      } else if (Offset == R_QNC_RCRB_SPIPBR1) {
        Offset = R_QNC_RCRB_SPIPBR2;
      } else {
        break;
      }
    } while (TRUE);
  }
  return FALSE;
}
