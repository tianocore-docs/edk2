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

  DxeFvFile.c

Abstract:

  Implementation of helper routines for DXE enviroment.

--*/

#include <PiDxe.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/DxeServicesLib.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/Spi.h>

#include "CommonHeader.h"

//
// Global variables.
//
EFI_SPI_PROTOCOL                    *mPlatHelpSpiProtocolRef = NULL;

//
// Routines defined in other source modules of this component.
//

//
// Routines local to this component.
//


EFI_SPI_PROTOCOL *
LocateSpiProtocol (
  IN  EFI_SMM_SYSTEM_TABLE2             *Smst
  )
{
  if (mPlatHelpSpiProtocolRef == NULL) {
    if (Smst != NULL) {
      Smst->SmmLocateProtocol (
              &gEfiSmmSpiProtocolGuid,
              NULL,
              (VOID **) &mPlatHelpSpiProtocolRef
              );
    } else {
      gBS->LocateProtocol (
             &gEfiSpiProtocolGuid,
             NULL,
             (VOID **) &mPlatHelpSpiProtocolRef
             );
    }
    ASSERT (mPlatHelpSpiProtocolRef != NULL);
  }
  return mPlatHelpSpiProtocolRef;
}

//
// Routines exported by this source module.
//

/**
  Find free spi protect register and write to it to protect a flash region.

  @param   DirectValue      Value to directly write to register.
                            if DirectValue == 0 the use Base & Length below.
  @param   BaseAddress      Base address of region in Flash Memory Map.
  @param   Length           Length of region to protect.

  @retval  EFI_SUCCESS      Free spi protect register found & written.
  @retval  EFI_NOT_FOUND    Free Spi protect register not found.
  @retval  EFI_DEVICE_ERROR Unable to write to spi protect register.
**/
EFI_STATUS
EFIAPI
PlatformWriteFirstFreeSpiProtect (
  IN CONST UINT32                         DirectValue,
  IN CONST UINT32                         BaseAddress,
  IN CONST UINT32                         Length
  )
{
  UINT32                            FreeOffset;
  UINT32                            PchRootComplexBar;
  EFI_STATUS                        Status;

  PchRootComplexBar = QNC_RCRB_BASE;

  Status = WriteFirstFreeSpiProtect (
             PchRootComplexBar,
             DirectValue,
             BaseAddress,
             Length,
             &FreeOffset
             );

  if (!EFI_ERROR (Status)) {
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
        (UINTN) (PchRootComplexBar + FreeOffset),
        1,
        (VOID *) (UINTN) (PchRootComplexBar + FreeOffset)
        );
  }

  return Status;
}

/**
  Lock legacy SPI static configuration information.

  Function will assert if unable to lock config.

**/
VOID
EFIAPI
PlatformFlashLockConfig (
  VOID
  )
{
  EFI_STATUS        Status;
  EFI_SPI_PROTOCOL  *SpiProtocol;

  //
  // Enable lock of legacy SPI static configuration information.
  //

  SpiProtocol = LocateSpiProtocol (NULL);  // This routine will not be called in SMM.
  ASSERT_EFI_ERROR (SpiProtocol != NULL);
  if (SpiProtocol != NULL) {
    Status = SpiProtocol->Lock (SpiProtocol);

    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Platform: Spi Config Locked Down\n"));
    } else if (Status == EFI_ACCESS_DENIED) {
      DEBUG ((EFI_D_INFO, "Platform: Spi Config already locked down\n"));
    } else {
      ASSERT_EFI_ERROR (Status);
    }
  }
}

/**
  Lock regions and config of SPI flash given the policy for this platform.

  Function will assert if unable to lock regions or config.

  @param   PreBootPolicy    If TRUE do Pre Boot Flash Lock Policy.

**/
VOID
EFIAPI
PlatformFlashLockPolicy (
  IN CONST BOOLEAN                        PreBootPolicy
  )
{
  EFI_STATUS                        Status;
  UINT64                            CpuAddressNvStorage;
  UINT64                            CpuAddressFlashDevice;
  UINT64                            SpiAddress;

  CpuAddressFlashDevice = SIZE_4GB - ((UINT64) PcdGet32(PcdFlashAreaSize));
  DEBUG (
      (EFI_D_INFO,
      "Platform:FlashDeviceSize = 0x%08x Bytes\n",
      (UINTN) PcdGet32(PcdFlashAreaSize))
      );

  //
  // Lock regions if Secure lock down.
  //

  if (FeaturePcdGet (PcdEnableSecureLock)) {

    CpuAddressNvStorage = (UINT64) PcdGet32 (PcdFlashNvStorageBase);

    //
    // Lock from start of flash device up to BIOS NV storage area.
    //
    SpiAddress = 0;
    if (!PlatformIsSpiRangeProtected ((UINT32) SpiAddress, (UINT32) (CpuAddressNvStorage - CpuAddressFlashDevice))) {
      DEBUG (
        (EFI_D_INFO,
        "Platform: Protect Region Base:Len 0x%08x:0x%08x\n",
        (UINTN) SpiAddress, (UINTN)(CpuAddressNvStorage - CpuAddressFlashDevice))
        );
      Status = PlatformWriteFirstFreeSpiProtect (
                 0,
                 (UINT32) SpiAddress,
                 (UINT32) (CpuAddressNvStorage - CpuAddressFlashDevice)
                 );

      ASSERT_EFI_ERROR (Status);
    }
    //
    // Move Spi Address to after BIOS NV Storage and OEM Area.
    //
    SpiAddress = CpuAddressNvStorage - CpuAddressFlashDevice;
    SpiAddress += ((UINT64) PcdGet32 (PcdFlashNvStorageSize)) + ((UINT64) FLASH_REGION_OEM_NV_STORAGE_SIZE);

    //
    // Lock from end of OEM area to end of flash part.
    //
    if (!PlatformIsSpiRangeProtected ((UINT32) SpiAddress, PcdGet32(PcdFlashAreaSize) - ((UINT32) SpiAddress))) {
      DEBUG (
        (EFI_D_INFO,
        "Platform: Protect Region Base:Len 0x%08x:0x%08x\n",
        (UINTN) SpiAddress,
        (UINTN) (PcdGet32(PcdFlashAreaSize) - ((UINT32) SpiAddress)))
        );
      ASSERT (SpiAddress < ((UINT64) PcdGet32(PcdFlashAreaSize)));
      Status = PlatformWriteFirstFreeSpiProtect (
                 0,
                 (UINT32) SpiAddress,
                 PcdGet32(PcdFlashAreaSize) - ((UINT32) SpiAddress)
                 );

      ASSERT_EFI_ERROR (Status);
    }
  } else {

    //
    // Move Spi Address to beginning of BOOTROM override area.
    //

    SpiAddress = FLASH_REGION_BOOTROM_OVERRIDE_BASE - CpuAddressFlashDevice;

    //
    // Lock BOOTROM override area.
    //
    if (!PlatformIsSpiRangeProtected ((UINT32) SpiAddress,FLASH_REGION_BOOTROM_OVERRIDE_MAX_SIZE)) {
      DEBUG (
          (EFI_D_INFO,
          "Platform: Protect Region Base:Len 0x%08x:0x%08x\n",
          (UINTN) SpiAddress,
          (UINTN) FLASH_REGION_BOOTROM_OVERRIDE_MAX_SIZE)
          );
      ASSERT (SpiAddress < ((UINT64) SIZE_4MB));
      Status = PlatformWriteFirstFreeSpiProtect (
          0,
          (UINT32) SpiAddress,
          FLASH_REGION_BOOTROM_OVERRIDE_MAX_SIZE
          );

      ASSERT_EFI_ERROR (Status);
    }
  }

  //
  // Always Lock flash config registers if about to boot a boot option
  // else lock depending on boot mode.
  //
  if (PreBootPolicy) {
    PlatformFlashLockConfig ();
  }
}

/**
  Erase and Write to platform flash.

  Routine accesses one flash block at a time, each access consists
  of an erase followed by a write of FLASH_BLOCK_SIZE. One or both
  of DoErase & DoWrite params must be TRUE.

  Limitations:-
    CpuWriteAddress must be aligned to FLASH_BLOCK_SIZE.
    DataSize must be a multiple of FLASH_BLOCK_SIZE.

  @param   Smst                   If != NULL then InSmm and use to locate
                                  SpiProtocol.
  @param   CpuWriteAddress        Address in CPU memory map of flash region.
  @param   Data                   The buffer containing the data to be written.
  @param   DataSize               Amount of data to write.
  @param   DoErase                Earse each block.
  @param   DoWrite                Write to each block.

  @retval  EFI_SUCCESS            Operation successful.
  @retval  EFI_NOT_READY          Required resources not setup.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
  @retval  Others                 Unexpected error happened.

**/
EFI_STATUS
EFIAPI
PlatformFlashEraseWrite (
  IN  VOID                              *Smst,
  IN  UINTN                             CpuWriteAddress,
  IN  UINT8                             *Data,
  IN  UINTN                             DataSize,
  IN  BOOLEAN                           DoErase,
  IN  BOOLEAN                           DoWrite
  )
{
  EFI_STATUS                        Status;
  UINT64                            CpuBaseAddress;
  SPI_INIT_INFO                     *SpiInfo;
  UINT8                             *WriteBuf;
  UINTN                             Index;
  UINTN                             SpiWriteAddress;
  EFI_SPI_PROTOCOL                  *SpiProtocol;

  if (!DoErase && !DoWrite) {
    return EFI_INVALID_PARAMETER;
  }
  if (DoWrite && Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if ((CpuWriteAddress % FLASH_BLOCK_SIZE) != 0) {
    return EFI_INVALID_PARAMETER;
  }
  if ((DataSize % FLASH_BLOCK_SIZE) != 0) {
    return EFI_INVALID_PARAMETER;
  }
  SpiProtocol = LocateSpiProtocol ((EFI_SMM_SYSTEM_TABLE2 *)Smst);
  if (SpiProtocol == NULL) {
    return EFI_NOT_READY;
  }

  //
  // Find info to allow usage of SpiProtocol->Execute.
  //
  Status = SpiProtocol->Info (
             SpiProtocol,
             &SpiInfo
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  ASSERT (SpiInfo->InitTable != NULL);
  ASSERT (SpiInfo->EraseOpcodeIndex < SPI_NUM_OPCODE);
  ASSERT (SpiInfo->ProgramOpcodeIndex < SPI_NUM_OPCODE);

  CpuBaseAddress = PcdGet32 (PcdFlashAreaBaseAddress) - (UINT32)SpiInfo->InitTable->BiosStartOffset;
  ASSERT(CpuBaseAddress >= (SIZE_4GB - SIZE_8MB));
  if (CpuWriteAddress < CpuBaseAddress) {
    return (EFI_INVALID_PARAMETER);
  }

  SpiWriteAddress = CpuWriteAddress - ((UINTN) CpuBaseAddress);
  WriteBuf = Data;
  DEBUG (
    (EFI_D_INFO, "PlatformFlashWrite:SpiWriteAddress=%08x EraseIndex=%d WriteIndex=%d\n", 
    SpiWriteAddress,
    (UINTN) SpiInfo->EraseOpcodeIndex,
    (UINTN) SpiInfo->ProgramOpcodeIndex
    ));
  for (Index =0; Index < DataSize / FLASH_BLOCK_SIZE; Index++) {
    if (DoErase) {
      DEBUG (
        (EFI_D_INFO, "PlatformFlashWrite:Erase[%04x] SpiWriteAddress=%08x\n",
        Index,
        SpiWriteAddress
        ));
      Status = SpiProtocol->Execute (
                              SpiProtocol,
                              SpiInfo->EraseOpcodeIndex,// OpcodeIndex
                              0,                        // PrefixOpcodeIndex
                              FALSE,                    // DataCycle
                              TRUE,                     // Atomic
                              FALSE,                    // ShiftOut
                              SpiWriteAddress,          // Address
                              0,                        // Data Number
                              NULL,
                              EnumSpiRegionAll          // SPI_REGION_TYPE
                              );
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }

    if (DoWrite) {
      DEBUG (
        (EFI_D_INFO, "PlatformFlashWrite:Write[%04x] SpiWriteAddress=%08x\n",
        Index,
        SpiWriteAddress
        ));
      Status = SpiProtocol->Execute (
                              SpiProtocol,
                              SpiInfo->ProgramOpcodeIndex,   // OpcodeIndex
                              0,                             // PrefixOpcodeIndex
                              TRUE,                          // DataCycle
                              TRUE,                          // Atomic
                              TRUE,                          // ShiftOut
                              SpiWriteAddress,               // Address
                              FLASH_BLOCK_SIZE,              // Data Number
                              WriteBuf,
                              EnumSpiRegionAll
                              );
      if (EFI_ERROR(Status)) {
        return Status;
      }
      WriteBuf+=FLASH_BLOCK_SIZE;
    }
    SpiWriteAddress+=FLASH_BLOCK_SIZE;
  }
  return EFI_SUCCESS;
}
