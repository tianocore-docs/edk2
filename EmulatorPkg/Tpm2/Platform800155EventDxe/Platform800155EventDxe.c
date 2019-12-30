/** @file
  Report 800-155 event

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/TpmMeasurementLib.h>
#include <IndustryStandard/UefiTcgPlatform.h>

#define IANA_ID_INTEL  0x157

#define PLATFORM_MANUFACTURER_STR "UEFI Emulator"
#define PLATFORM_MODEL            "Emulator"
#define PLATFORM_VERSION          "0.3"
#define FIRMWARE_MANUFACTURER_STR "EDK II DXE"
#define FIRMWARE_VERSION          "0.7"

EFI_GUID  mReferenceManifestGuid = {0xdd11c26f, 0x21fd, 0x4dd3, {0x9c, 0xd9, 0xd2, 0xc4, 0x14, 0x1e, 0x97, 0x78}};


/**
  Main entry for this driver.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
Platform800155EventDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  TCG_Sp800_155_PlatformId_Event2  *TcgSp800155PlatformIdEventStruct;
  UINT32                           TcgSp800155PlatformIdEventStructSize;
  UINT8                            PlatformManufacturerStrSize;
  CHAR8                            *PlatformManufacturerStr;
  UINT8                            PlatformModelSize;
  CHAR8                            *PlatformModel;
  UINT8                            PlatformVersionSize;
  CHAR8                            *PlatformVersion;
  UINT8                            FirmwareManufacturerStrSize;
  CHAR8                            *FirmwareManufacturerStr;
  UINT8                            FirmwareVersionSize;
  CHAR8                            *FirmwareVersion;
  UINT32                           FirmwareManufacturerId;
  UINT32                           PlatformManufacturerId;
  UINT8                            *StrSize;
  UINT8                            *Str;
  UINT32                           *Id;

  PlatformManufacturerStr = PLATFORM_MANUFACTURER_STR;
  PlatformModel = PLATFORM_MODEL;
  PlatformVersion = PLATFORM_VERSION;
  FirmwareManufacturerStr = FIRMWARE_MANUFACTURER_STR;
  FirmwareVersion = FIRMWARE_VERSION;
  PlatformManufacturerId = IANA_ID_INTEL;
  FirmwareManufacturerId = IANA_ID_INTEL;

  PlatformManufacturerStrSize = (UINT8)AsciiStrSize(PlatformManufacturerStr);
  PlatformModelSize = (UINT8)AsciiStrSize(PlatformModel);
  PlatformVersionSize = (UINT8)AsciiStrSize(PlatformVersion);
  FirmwareManufacturerStrSize = (UINT8)AsciiStrSize(FirmwareManufacturerStr);
  FirmwareVersionSize = (UINT8)AsciiStrSize(FirmwareVersion);
  TcgSp800155PlatformIdEventStructSize = sizeof(TCG_Sp800_155_PlatformId_Event2) + 
                                         sizeof(UINT8) + PlatformManufacturerStrSize +
                                         sizeof(UINT8) + PlatformModelSize +
                                         sizeof(UINT8) + PlatformVersionSize +
                                         sizeof(UINT8) + FirmwareManufacturerStrSize +
                                         sizeof(UINT32) +
                                         sizeof(UINT8) + FirmwareVersionSize;

  TcgSp800155PlatformIdEventStruct = AllocateZeroPool (TcgSp800155PlatformIdEventStructSize);
  ASSERT(TcgSp800155PlatformIdEventStruct != NULL);

  CopyMem (TcgSp800155PlatformIdEventStruct->Signature, TCG_Sp800_155_PlatformId_Event2_SIGNATURE, sizeof(TCG_Sp800_155_PlatformId_Event2_SIGNATURE)-1);
  TcgSp800155PlatformIdEventStruct->VendorId = PlatformManufacturerId;
  CopyGuid (&TcgSp800155PlatformIdEventStruct->ReferenceManifestGuid, &mReferenceManifestGuid);

  StrSize = (UINT8 *)(TcgSp800155PlatformIdEventStruct + 1);
  Str = StrSize + 1;
  *StrSize = PlatformManufacturerStrSize;
  CopyMem (Str, PlatformManufacturerStr, *StrSize);

  StrSize = (UINT8 *)(Str + *StrSize);
  Str = StrSize + 1;
  *StrSize = PlatformModelSize;
  CopyMem (Str, PlatformModel, *StrSize);

  StrSize = (UINT8 *)(Str + *StrSize);
  Str = StrSize + 1;
  *StrSize = PlatformVersionSize;
  CopyMem (Str, PlatformVersion, *StrSize);

  StrSize = (UINT8 *)(Str + *StrSize);
  Str = StrSize + 1;
  *StrSize = FirmwareManufacturerStrSize;
  CopyMem (Str, FirmwareManufacturerStr, *StrSize);

  Id = (UINT32 *)(Str + *StrSize);
  *Id = FirmwareManufacturerId;
  
  StrSize = (UINT8 *)(Id + 1);
  Str = StrSize + 1;
  *StrSize = FirmwareVersionSize;
  CopyMem (Str, FirmwareVersion, *StrSize);

  TpmMeasureAndLogData (0, EV_NO_ACTION, TcgSp800155PlatformIdEventStruct, TcgSp800155PlatformIdEventStructSize, NULL, 0);

  return EFI_SUCCESS;
}
