/** @file

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "FspUpdate.h"

#define FSP_FMP_PRIVATE_DATA_SIGNATURE  SIGNATURE_32('S', 'T', 'M', 'F')

//
// FSP FMP private data structure.
//

typedef struct {
  UINT32 LastAttemptVersion;
  UINT32 LastAttemptStatus;
} FSP_FMP_LAST_ATTEMPT_VARIABLE;

struct _FSP_FMP_PRIVATE_DATA {
  UINT32                               Signature;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL     Fmp;
  EFI_HANDLE                           Handle;
  UINT8                                DescriptorCount;
  EFI_FIRMWARE_IMAGE_DESCRIPTOR        *ImageDescriptor;
  UINT32                               PackageVersion;
  CHAR16                               *PackageVersionName;
  FSP_FMP_LAST_ATTEMPT_VARIABLE        LastAttempt;
};

typedef struct _FSP_FMP_PRIVATE_DATA  FSP_FMP_PRIVATE_DATA;

#define FSP_FMP_LAST_ATTEMPT_VARIABLE_NAME  L"FspLastAttempVar"

/**
  Returns a pointer to the FSP_FMP_PRIVATE_DATA structure from the input a as Fmp.
  
  If the signatures matches, then a pointer to the data structure that contains 
  a specified field of that data structure is returned.
   
  @param  a              Pointer to the field specified by ServiceBinding within 
                         a data structure of type FSP_FMP_PRIVATE_DATA.
 
**/
#define FSP_FMP_PRIVATE_DATA_FROM_FMP(a) \
  CR ( \
  (a), \
  FSP_FMP_PRIVATE_DATA, \
  Fmp, \
  FSP_FMP_PRIVATE_DATA_SIGNATURE \
  )

//
// FspFmp driver private data
//
FSP_FMP_PRIVATE_DATA *mFspFmpPrivate = NULL;

EFI_FIRMWARE_MANAGEMENT_PROTOCOL mFirmwareManagementProtocol = {
  FmpGetImageInfo,
  FmpGetImage,
  FmpSetImage,
  FmpCheckImage,
  FmpGetPackageInfo,
  FmpSetPackageInfo
};

/**
  Returns information about the current firmware image(s) of the device.

  This function allows a copy of the current firmware image to be created and saved.
  The saved copy could later been used, for example, in firmware image recovery or rollback.

  @param[in]      This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in, out] ImageInfoSize      A pointer to the size, in bytes, of the ImageInfo buffer. 
                                     On input, this is the size of the buffer allocated by the caller.
                                     On output, it is the size of the buffer returned by the firmware 
                                     if the buffer was large enough, or the size of the buffer needed
                                     to contain the image(s) information if the buffer was too small.
  @param[in, out] ImageInfo          A pointer to the buffer in which firmware places the current image(s)
                                     information. The information is an array of EFI_FIRMWARE_IMAGE_DESCRIPTORs.
  @param[out]     DescriptorVersion  A pointer to the location in which firmware returns the version number
                                     associated with the EFI_FIRMWARE_IMAGE_DESCRIPTOR. 
  @param[out]     DescriptorCount    A pointer to the location in which firmware returns the number of
                                     descriptors or firmware images within this device.
  @param[out]     DescriptorSize     A pointer to the location in which firmware returns the size, in bytes,
                                     of an individual EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  @param[out]     PackageVersion     A version number that represents all the firmware images in the device.
                                     The format is vendor specific and new version must have a greater value
                                     than the old version. If PackageVersion is not supported, the value is
                                     0xFFFFFFFF. A value of 0xFFFFFFFE indicates that package version comparison
                                     is to be performed using PackageVersionName. A value of 0xFFFFFFFD indicates
                                     that package version update is in progress.
  @param[out]     PackageVersionName A pointer to a pointer to a null-terminated string representing the
                                     package version name. The buffer is allocated by this function with 
                                     AllocatePool(), and it is the caller's responsibility to free it with a call
                                     to FreePool().

  @retval EFI_SUCCESS                The device was successfully updated with the new image.
  @retval EFI_BUFFER_TOO_SMALL       The ImageInfo buffer was too small. The current buffer size 
                                     needed to hold the image(s) information is returned in ImageInfoSize.                                               
  @retval EFI_INVALID_PARAMETER      ImageInfoSize is NULL.
  @retval EFI_DEVICE_ERROR           Valid information could not be returned. Possible corrupted image.

**/
EFI_STATUS
EFIAPI
FmpGetImageInfo (
  IN        EFI_FIRMWARE_MANAGEMENT_PROTOCOL*This,
  IN OUT    UINTN                           *ImageInfoSize,
  IN OUT    EFI_FIRMWARE_IMAGE_DESCRIPTOR   *ImageInfo,
  OUT       UINT32                          *DescriptorVersion,
  OUT       UINT8                           *DescriptorCount,
  OUT       UINTN                           *DescriptorSize,
  OUT       UINT32                          *PackageVersion,
  OUT       CHAR16                          **PackageVersionName
  ) 
{
  FSP_FMP_PRIVATE_DATA *FspFmpPrivate;
  UINTN                Index;

  FspFmpPrivate = FSP_FMP_PRIVATE_DATA_FROM_FMP(This);

  if(ImageInfoSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*ImageInfoSize < sizeof(EFI_FIRMWARE_IMAGE_DESCRIPTOR) * FspFmpPrivate->DescriptorCount) {
    *ImageInfoSize = sizeof(EFI_FIRMWARE_IMAGE_DESCRIPTOR) * FspFmpPrivate->DescriptorCount;
    return EFI_BUFFER_TOO_SMALL;
  }

  if (ImageInfo == NULL ||
      DescriptorVersion == NULL ||
      DescriptorCount == NULL ||
      DescriptorSize == NULL ||
      PackageVersion == NULL ||
      PackageVersionName == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *ImageInfoSize      = sizeof(EFI_FIRMWARE_IMAGE_DESCRIPTOR) * FspFmpPrivate->DescriptorCount;
  *DescriptorSize     = sizeof(EFI_FIRMWARE_IMAGE_DESCRIPTOR);
  *DescriptorCount    = FspFmpPrivate->DescriptorCount;
  *DescriptorVersion  = EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION;

  //
  // supports 1 ImageInfo descriptor
  //
  CopyMem(&ImageInfo[0], FspFmpPrivate->ImageDescriptor, sizeof(EFI_FIRMWARE_IMAGE_DESCRIPTOR) * FspFmpPrivate->DescriptorCount);
  for (Index = 0; Index < FspFmpPrivate->DescriptorCount; Index++) {
    if ((ImageInfo[Index].AttributesSetting & IMAGE_ATTRIBUTE_IN_USE) != 0) {
      ImageInfo[Index].LastAttemptVersion = FspFmpPrivate->LastAttempt.LastAttemptVersion;
      ImageInfo[Index].LastAttemptStatus = FspFmpPrivate->LastAttempt.LastAttemptStatus;
    }
  }

  //
  // package version
  //
  *PackageVersion = FspFmpPrivate->PackageVersion;
  if (FspFmpPrivate->PackageVersionName != NULL) {
    *PackageVersionName = AllocateCopyPool(StrSize(FspFmpPrivate->PackageVersionName), FspFmpPrivate->PackageVersionName);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FmpGetImage(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL  *This,
  IN  UINT8                             ImageIndex,
  IN  OUT  VOID                         *Image,
  IN  OUT  UINTN                        *ImageSize
  )
{
  return EFI_UNSUPPORTED;
}

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
  )
{
  EFI_STATUS                 Status;
  EFI_STATUS                 VarStatus;
  FSP_FMP_PRIVATE_DATA       *FspFmpPrivate;

  if (Image == NULL || AbortReason == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  FspFmpPrivate = FSP_FMP_PRIVATE_DATA_FROM_FMP(This);
  *AbortReason     = NULL;

  if (ImageIndex == 0 || ImageIndex > FspFmpPrivate->DescriptorCount || Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FspWrite(ImageIndex, (VOID *)Image, ImageSize, &FspFmpPrivate->LastAttempt.LastAttemptVersion, &FspFmpPrivate->LastAttempt.LastAttemptStatus);
  DEBUG((DEBUG_INFO, "SetImage - LastAttemp Version - 0x%x, State - 0x%x\n", FspFmpPrivate->LastAttempt.LastAttemptVersion, FspFmpPrivate->LastAttempt.LastAttemptStatus));
  VarStatus = gRT->SetVariable(
                     FSP_FMP_LAST_ATTEMPT_VARIABLE_NAME,
                     &gEfiCallerIdGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                     sizeof(FspFmpPrivate->LastAttempt),
                     &FspFmpPrivate->LastAttempt
                     );
  DEBUG((DEBUG_INFO, "SetLastAttemp - %r\n", VarStatus));

  return Status;
}

EFI_STATUS
EFIAPI
FmpCheckImage(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL  *This,
  IN  UINT8                             ImageIndex,
  IN  CONST VOID                        *Image,
  IN  UINTN                             ImageSize,
  OUT UINT32                            *ImageUpdatable
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
FmpGetPackageInfo(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL *This,
  OUT UINT32                           *PackageVersion,
  OUT CHAR16                           **PackageVersionName,
  OUT UINT32                           *PackageVersionNameMaxLen,
  OUT UINT64                           *AttributesSupported,
  OUT UINT64                           *AttributesSetting
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
FmpSetPackageInfo(
  IN  EFI_FIRMWARE_MANAGEMENT_PROTOCOL   *This,
  IN  CONST VOID                         *Image,
  IN  UINTN                              ImageSize,
  IN  CONST VOID                         *VendorCode,
  IN  UINT32                             PackageVersion,
  IN  CONST CHAR16                       *PackageVersionName
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
InitializeFspDescriptor (
  IN FSP_FMP_PRIVATE_DATA *FspFmpPrivate
  )
{
  FSP_INFO_HEADER                      *FspHeader;
  EFI_FIRMWARE_IMAGE_DESCRIPTOR        *ImageDescriptor;
  UINT32                               *ImageVersionPtr;
  UINT32                               *LowestSupportedImageVersionPtr;

  FspFmpPrivate->DescriptorCount = 1;
  FspFmpPrivate->ImageDescriptor = AllocateZeroPool(FspFmpPrivate->DescriptorCount * sizeof(EFI_FIRMWARE_IMAGE_DESCRIPTOR));
  if (FspFmpPrivate->ImageDescriptor == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Construct FSP image info
  // Just use FSPS, because it is the first one.
  //
  FspHeader = FspFindFspHeader (PcdGet32(PcdFspsBaseAddress));
  if (FspHeader == NULL) {
    DEBUG((DEBUG_ERROR, "FspFindFspHeader (Current) - fail\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  ImageVersionPtr = &FspHeader->ImageRevision;
  LowestSupportedImageVersionPtr = ImageVersionPtr;

  ImageDescriptor = FspFmpPrivate->ImageDescriptor;
  ImageDescriptor->ImageIndex = 1;
  CopyGuid(&ImageDescriptor->ImageTypeId, &gFspFmpImageTypeIdGuid);
  ImageDescriptor->ImageId = *(UINT64 *)FspHeader->ImageId;
  ImageDescriptor->ImageIdName = NULL;
  ImageDescriptor->Version = *ImageVersionPtr;
  ImageDescriptor->VersionName = NULL;
  ImageDescriptor->Size = FspHeader->ImageSize;
  ImageDescriptor->AttributesSupported = IMAGE_ATTRIBUTE_IMAGE_UPDATABLE | IMAGE_ATTRIBUTE_RESET_REQUIRED | IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED | IMAGE_ATTRIBUTE_IN_USE;
  ImageDescriptor->AttributesSetting = ImageDescriptor->AttributesSupported;
  ImageDescriptor->Compatibilities = 0;
  ImageDescriptor->LowestSupportedImageVersion = *LowestSupportedImageVersionPtr;
  ImageDescriptor->LastAttemptVersion = FspFmpPrivate->LastAttempt.LastAttemptVersion;
  ImageDescriptor->LastAttemptStatus = FspFmpPrivate->LastAttempt.LastAttemptStatus;
  ImageDescriptor->HardwareInstance = 0;

  return EFI_SUCCESS;
}

/*++
  
  Routine Description:
  
     Initialize FspFmpDriver private data structure.
  
  Arguments:
  

     FspFmpPrivate     - private data structure to be initialized.
  
  Returns:

  --*/
EFI_STATUS
InitializePrivateData(
  IN FSP_FMP_PRIVATE_DATA *FspFmpPrivate
  )
{
  EFI_STATUS       Status;
  EFI_STATUS       VarStatus;
  UINTN            VarSize;

  FspFmpPrivate->Signature       = FSP_FMP_PRIVATE_DATA_SIGNATURE;
  FspFmpPrivate->Handle          = NULL;
  CopyMem(&FspFmpPrivate->Fmp, &mFirmwareManagementProtocol, sizeof(EFI_FIRMWARE_MANAGEMENT_PROTOCOL));

  FspFmpPrivate->PackageVersion = 0x1;
  FspFmpPrivate->PackageVersionName = L"Fsp";

  FspFmpPrivate->LastAttempt.LastAttemptVersion = 0x0;
  FspFmpPrivate->LastAttempt.LastAttemptStatus = 0x0;
  VarSize = sizeof(FspFmpPrivate->LastAttempt);
  VarStatus = gRT->GetVariable(
                     FSP_FMP_LAST_ATTEMPT_VARIABLE_NAME,
                     &gEfiCallerIdGuid,
                     NULL,
                     &VarSize,
                     &FspFmpPrivate->LastAttempt
                     );
  DEBUG((DEBUG_INFO, "GetLastAttemp - %r\n", VarStatus));
  DEBUG((DEBUG_INFO, "GetLastAttemp Version - 0x%x, State - 0x%x\n", FspFmpPrivate->LastAttempt.LastAttemptVersion, FspFmpPrivate->LastAttempt.LastAttemptStatus));

  Status = InitializeFspDescriptor(FspFmpPrivate);

  return Status;
}

EFI_STATUS
EFIAPI
FspFmpMain (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;

  //
  // Initialize FspFmpPrivateData
  //
  mFspFmpPrivate = AllocateZeroPool (sizeof(FSP_FMP_PRIVATE_DATA));
  if (mFspFmpPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = InitializePrivateData(mFspFmpPrivate);
  if (EFI_ERROR(Status)) {
    FreePool(mFspFmpPrivate);
    mFspFmpPrivate = NULL;
  }

  //
  // Install FMP protocol.
  //
  Status = gBS->InstallProtocolInterface (
                  &mFspFmpPrivate->Handle,
                  &gEfiFirmwareManagementProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mFspFmpPrivate->Fmp
                  );
  if (EFI_ERROR (Status)) {
    FreePool(mFspFmpPrivate);
    mFspFmpPrivate = NULL;
    return Status;
  }

  return Status;
}
