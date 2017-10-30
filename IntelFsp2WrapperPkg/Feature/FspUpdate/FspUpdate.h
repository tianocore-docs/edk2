/** @file

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FSP_UPDATE_H_
#define _FSP_UPDATE_H_

#include <PiDxe.h>

#include <Guid/SystemResourceTable.h>
#include <Guid/FspFmp.h>
#include <Guid/FirmwareFileSystem3.h>
#include <Guid/FirmwareFileSystem2.h>

#include <Protocol/FirmwareManagement.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/DxeServicesLib.h>
#include <Library/FmpAuthenticationLib.h>
#include <Library/FspWrapperApiLib.h>
#include <Library/FspFlashAccessLib.h>

#include <FspEas.h>

typedef enum {
  FspTypeFspt,
  FspTypeFspm,
  FspTypeFsps,
} FSP_TYPE;

EFI_STATUS
FspWrite(
  IN  UINTN  ImageIndex,
  IN  VOID   *Image,
  IN  UINTN  ImageSize,
  OUT UINT32 *LastAttemptVersion,
  OUT UINT32 *LastAttemptStatus
  );

EFI_STATUS
EFIAPI
FmpGetImageInfo(
  IN EFI_FIRMWARE_MANAGEMENT_PROTOCOL       *This,
  IN OUT    UINTN                           *ImageInfoSize,
  IN OUT    EFI_FIRMWARE_IMAGE_DESCRIPTOR   *ImageInfo,
  OUT       UINT32                          *DescriptorVersion,
  OUT       UINT8                           *DescriptorCount,
  OUT       UINTN                           *DescriptorSize,
  OUT       UINT32                          *PackageVersion,
  OUT       CHAR16                          **PackageVersionName
  );

EFI_STATUS
EFIAPI
FmpGetImage(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL  *This,
  IN  UINT8                             ImageIndex,
  IN  OUT  VOID                         *Image,
  IN  OUT  UINTN                        *ImageSize
  );

EFI_STATUS
EFIAPI
FmpSetImage(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL                 *This,
  IN  UINT8                                            ImageIndex,
  IN  CONST VOID                                       *Image,
  IN  UINTN                                            ImageSize,
  IN  CONST VOID                                       *VendorCode,
  IN  EFI_FIRMWARE_MANAGEMENT_UPDATE_IMAGE_PROGRESS    Progress,
  OUT CHAR16                                           **AbortReason
  );

EFI_STATUS
EFIAPI
FmpCheckImage(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL  *This,
  IN  UINT8                             ImageIndex,
  IN  CONST VOID                        *Image,
  IN  UINTN                             ImageSize,
  OUT UINT32                            *ImageUpdatable
  );

EFI_STATUS
EFIAPI
FmpGetPackageInfo(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL *This,
  OUT UINT32                           *PackageVersion,
  OUT CHAR16                           **PackageVersionName,
  OUT UINT32                           *PackageVersionNameMaxLen,
  OUT UINT64                           *AttributesSupported,
  OUT UINT64                           *AttributesSetting
  );

EFI_STATUS
EFIAPI
FmpSetPackageInfo(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL   *This,
  IN  CONST VOID                         *Image,
  IN  UINTN                              ImageSize,
  IN  CONST VOID                         *VendorCode,
  IN  UINT32                             PackageVersion,
  IN  CONST CHAR16                       *PackageVersionName
  );

#endif

