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

/**
  Extract the authenticated image from an FMP capsule image.

  Caution: This function may receive untrusted input.

  @param[in]  Image                   The FMP capsule image, including EFI_FIRMWARE_IMAGE_AUTHENTICATION.
  @param[in]  ImageSize               The size of FMP capsule image in bytes.
  @param[out] LastAttemptStatus       The last attempt status, which will be recorded in ESRT and FMP EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  @param[out] AuthenticatedImage      The authenticated capsule image, excluding EFI_FIRMWARE_IMAGE_AUTHENTICATION.
  @param[out] AuthenticatedImageSize  The size of the authenticated capsule image in bytes.

  @retval TRUE  The authenticated image is extracted.
  @retval FALSE The authenticated image is not extracted.
**/
BOOLEAN
EFIAPI
ExtractAuthenticatedImage (
  IN VOID                         *Image,
  IN UINTN                        ImageSize,
  OUT UINT32                      *LastAttemptStatus,
  OUT VOID                        **AuthenticatedImage,
  OUT UINTN                       *AuthenticatedImageSize
  )
{
  EFI_FIRMWARE_IMAGE_AUTHENTICATION         *ImageAuth;
  EFI_STATUS                                Status;
  GUID                                      *CertType;
  VOID                                      *PublicKeyData;
  UINTN                                     PublicKeyDataLength;

  DEBUG((DEBUG_INFO, "ExtractAuthenticatedImage - Image: 0x%08x - 0x%08x\n", (UINTN)Image, (UINTN)ImageSize));

  *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INVALID_FORMAT;
  if ((Image == NULL) || (ImageSize == 0)) {
    return FALSE;
  }

  ImageAuth = (EFI_FIRMWARE_IMAGE_AUTHENTICATION *)Image;
  if (ImageSize < sizeof(EFI_FIRMWARE_IMAGE_AUTHENTICATION)) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - ImageSize too small\n"));
    return FALSE;
  }
  if (ImageAuth->AuthInfo.Hdr.dwLength <= OFFSET_OF(WIN_CERTIFICATE_UEFI_GUID, CertData)) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - dwLength too small\n"));
    return FALSE;
  }
  if ((UINTN) ImageAuth->AuthInfo.Hdr.dwLength > MAX_UINTN - sizeof(UINT64)) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - dwLength too big\n"));
    return FALSE;
  }
  if (ImageSize <= sizeof(ImageAuth->MonotonicCount) + ImageAuth->AuthInfo.Hdr.dwLength) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - ImageSize too small\n"));
    return FALSE;
  }
  if (ImageAuth->AuthInfo.Hdr.wRevision != 0x0200) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - wRevision: 0x%02x, expect - 0x%02x\n", (UINTN)ImageAuth->AuthInfo.Hdr.wRevision, (UINTN)0x0200));
    return FALSE;
  }
  if (ImageAuth->AuthInfo.Hdr.wCertificateType != WIN_CERT_TYPE_EFI_GUID) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - wCertificateType: 0x%02x, expect - 0x%02x\n", (UINTN)ImageAuth->AuthInfo.Hdr.wCertificateType, (UINTN)WIN_CERT_TYPE_EFI_GUID));
    return FALSE;
  }

  CertType = &ImageAuth->AuthInfo.CertType;
  DEBUG((DEBUG_INFO, "ExtractAuthenticatedImage - CertType: %g\n", CertType));

  if (CompareGuid(&gEfiCertPkcs7Guid, CertType)) {
    PublicKeyData   = PcdGetPtr(PcdPkcs7CertBuffer);
    PublicKeyDataLength = PcdGetSize(PcdPkcs7CertBuffer);
  } else if (CompareGuid(&gEfiCertTypeRsa2048Sha256Guid, CertType)) {
    PublicKeyData = PcdGetPtr(PcdRsa2048Sha256PublicKeyBuffer);
    PublicKeyDataLength = PcdGetSize(PcdRsa2048Sha256PublicKeyBuffer);
  } else {
    return FALSE;
  }
  ASSERT (PublicKeyData != NULL);
  ASSERT (PublicKeyDataLength != 0);

  Status = AuthenticateFmpImage(
             ImageAuth,
             ImageSize,
             PublicKeyData,
             PublicKeyDataLength
             );
  switch (Status) {
  case RETURN_SUCCESS:
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_SUCCESS;
    break;
  case RETURN_SECURITY_VIOLATION:
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_AUTH_ERROR;
    break;
  case RETURN_INVALID_PARAMETER:
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INVALID_FORMAT;
    break;
  case RETURN_UNSUPPORTED:
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INVALID_FORMAT;
    break;
  case RETURN_OUT_OF_RESOURCES:
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES;
    break;
  default:
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_UNSUCCESSFUL;
    break;
  }
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  if (AuthenticatedImage != NULL) {
    *AuthenticatedImage = (UINT8 *)ImageAuth + ImageAuth->AuthInfo.Hdr.dwLength + sizeof(ImageAuth->MonotonicCount);
  }
  if (AuthenticatedImageSize != NULL) {
    *AuthenticatedImageSize = ImageSize - ImageAuth->AuthInfo.Hdr.dwLength - sizeof(ImageAuth->MonotonicCount);
  }
  return TRUE;
}

EFI_STATUS
GetFspImageOnType (
  IN FSP_TYPE         FspType,
  IN VOID             *AuthenticatedImage,
  IN UINTN            AuthenticatedImageSize,
  OUT VOID            **FspImage,
  OUT UINTN           *FspImageSize
  )
{
  EFI_FIRMWARE_VOLUME_HEADER      *FvHeader;
  UINTN                           FvIndex;
  UINTN                           Index;
  FSP_INFO_HEADER                 *FspInfoHeader;

  DEBUG((DEBUG_INFO, "GetFspImageOnType - %x\n", FspType));

  switch(FspType) {
  case FspTypeFspt:
    FvIndex = 2;
    break;
  case FspTypeFspm:
    FvIndex = 1;
    break;
  case FspTypeFsps:
    FvIndex = 0;
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  DEBUG((DEBUG_INFO, "FvIndex - %x\n", FvIndex));

  Index = 0;
  FvHeader = AuthenticatedImage;

  while ((UINTN)FvHeader < (UINTN)AuthenticatedImage + AuthenticatedImageSize) {
    if (FvHeader->Signature != EFI_FVH_SIGNATURE) {
      DEBUG((DEBUG_ERROR, "Error: Signature 0x%x is not EFI_FVH_SIGNATURE!\n", FvHeader->Signature));
      return EFI_UNSUPPORTED;
    }
    if (Index == FvIndex) {
      DEBUG((DEBUG_INFO, "FvIndex Found!\n"));
      break;
    }
    FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)((UINT8 *)FvHeader + (UINTN)FvHeader->FvLength);
    Index++;
  }

  if ((UINTN)FvHeader >= (UINTN)AuthenticatedImage + AuthenticatedImageSize) {
    DEBUG((DEBUG_ERROR, "Error: FvIndex NotFound!\n"));
    return EFI_UNSUPPORTED;
  }

  FspInfoHeader = FspFindFspHeader ((UINTN)FvHeader);
  if (FspInfoHeader == NULL) {
    DEBUG((DEBUG_ERROR, "Error: FspInfoHeader not found\n"));
    return EFI_UNSUPPORTED;
  }

  *FspImage = FvHeader;
  *FspImageSize = (UINTN)FvHeader->FvLength;
  
  return EFI_SUCCESS;
}

EFI_STATUS
VerifyFsp(
  IN VOID    *Image,
  IN UINTN   ImageSize,
  OUT UINT32 *LastAttemptVersion,
  OUT UINT32 *LastAttemptStatus,
  OUT VOID   **AuthenticatedImage,
  OUT UINTN  *AuthenticatedImageSize
  )
{
  BOOLEAN          Result;
  FSP_INFO_HEADER  *FspHeader;
  FSP_INFO_HEADER  *CurrentFspHeader;
  UINT32           FspStorageBase;
  UINT32           FspStorageSize;
  UINT32           *ImageVersionPtr;
  UINT32           *CurrentImageVersionPtr;
  UINT32           *CurrentLowestSupportedImageVersionPtr;
  FSP_TYPE         FspType;
  VOID             *FspImage;
  UINTN            FspImageSize;
  EFI_STATUS       Status;

  *LastAttemptVersion = 0;
  Result = ExtractAuthenticatedImage((VOID *)Image, ImageSize, LastAttemptStatus, AuthenticatedImage, AuthenticatedImageSize);
  if (!Result) {
    DEBUG((DEBUG_ERROR, "ExtractAuthenticatedImage - fail\n"));
    return EFI_SECURITY_VIOLATION;
  }

  for (FspType = FspTypeFspt; FspType <= FspTypeFsps; FspType++) {
    DEBUG((DEBUG_INFO, "Checking FspType - %d\n", FspType));
    switch (FspType) {
    case FspTypeFspt:
      FspStorageBase = PcdGet32 (PcdFsptBaseAddress);
      FspStorageSize = PcdGet32 (PcdFsptFlashSize);
      break;
    case FspTypeFspm:
      FspStorageBase = PcdGet32 (PcdFspmBaseAddress);
      FspStorageSize = PcdGet32 (PcdFspmFlashSize);
      break;
    case FspTypeFsps:
      FspStorageBase = PcdGet32 (PcdFspsBaseAddress);
      FspStorageSize = PcdGet32 (PcdFspsFlashSize);
      break;
    default:
      ASSERT(FALSE);
      break;
    }
    CurrentFspHeader = FspFindFspHeader (FspStorageBase);
    if (CurrentFspHeader == NULL) {
      *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES;
      DEBUG((DEBUG_ERROR, "FspFindFspHeader (Current) - fail\n"));
      return EFI_OUT_OF_RESOURCES;
    }
    DEBUG((DEBUG_INFO, "FspStorageBase - 0x%x, CurrentFspHeader - 0x%x\n", FspStorageBase, CurrentFspHeader));

    Status = GetFspImageOnType (FspType, *AuthenticatedImage, *AuthenticatedImageSize, &FspImage, &FspImageSize);
    if (EFI_ERROR(Status)) {
      *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES;
      DEBUG((DEBUG_ERROR, "GetFspImageOnType - %r\n", Status));
      return EFI_OUT_OF_RESOURCES;
    }

    if (FspImageSize > FspStorageSize) {
      *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES;
      DEBUG((DEBUG_ERROR, "ImageSize size (0x%x) is too big. Max - (0x%x)\n", FspImageSize, FspStorageSize));
      return EFI_OUT_OF_RESOURCES;
    }

    FspHeader = FspFindFspHeader ((UINTN)FspImage);
    if (FspHeader == NULL) {
      *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES;
      DEBUG((DEBUG_ERROR, "FspFindFspHeader (new) - fail\n"));
      return EFI_OUT_OF_RESOURCES;
    }
    ImageVersionPtr = &FspHeader->ImageRevision;
    CurrentImageVersionPtr = &CurrentFspHeader->ImageRevision;
    CurrentLowestSupportedImageVersionPtr = CurrentImageVersionPtr;

    *LastAttemptVersion = *ImageVersionPtr;

    if (*ImageVersionPtr < *CurrentLowestSupportedImageVersionPtr) {
      *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INCORRECT_VERSION;
      DEBUG((DEBUG_INFO, "LowestSupportedImageVersion check - fail\n"));
      return EFI_SECURITY_VIOLATION;
    }
  }

  *LastAttemptStatus = LAST_ATTEMPT_STATUS_SUCCESS;

  return EFI_SUCCESS;
}

EFI_STATUS
UpdateFsp(
  IN UINT64  Address,
  IN VOID    *Image,
  IN UINTN   ImageSize,
  OUT UINT32 *LastAttemptStatus
  )
{
  EFI_STATUS  Status;

  DEBUG((DEBUG_INFO, "PlatformUpdate:"));
  DEBUG((DEBUG_INFO, "  Address - 0x%lx,", Address));
  DEBUG((DEBUG_INFO, "  Legnth - 0x%x\n", ImageSize));

  Status = FspFlashWrite (
             Address,
             Image,
             ImageSize
             );
  if (!EFI_ERROR(Status)) {
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_SUCCESS;
  } else {
    *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_UNSUCCESSFUL;
  }
  return Status;
}

EFI_STATUS
FspWrite(
  IN  UINTN  ImageIndex,
  IN  VOID   *Image,
  IN  UINTN  ImageSize,
  OUT UINT32 *LastAttemptVersion,
  OUT UINT32 *LastAttemptStatus
  )
{
  VOID             *AuthenticatedImage;
  UINTN            AuthenticatedImageSize;
  UINT32           FspStorageBase;
  FSP_TYPE         FspType;
  VOID             *FspImage;
  UINTN            FspImageSize;
  EFI_STATUS       Status;

  Status = VerifyFsp(Image, ImageSize, LastAttemptVersion, LastAttemptStatus, &AuthenticatedImage, &AuthenticatedImageSize);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Fail to verify FSP image\n"));
    return Status;
  }
  DEBUG((DEBUG_INFO, "Pass VerifyFsp\n"));

  for (FspType = FspTypeFspt; FspType <= FspTypeFsps; FspType++) {
    DEBUG((DEBUG_INFO, "Updating FspType - %d\n", FspType));
    switch (FspType) {
    case FspTypeFspt:
      FspStorageBase = PcdGet32(PcdFsptFlashBaseAddress);
      break;
    case FspTypeFspm:
      FspStorageBase = PcdGet32(PcdFspmFlashBaseAddress);
      break;
    case FspTypeFsps:
      FspStorageBase = PcdGet32(PcdFspsFlashBaseAddress);
      break;
    default:
      ASSERT(FALSE);
      break;
    }

    Status = GetFspImageOnType (FspType, AuthenticatedImage, AuthenticatedImageSize, &FspImage, &FspImageSize);
    if (EFI_ERROR(Status)) {
      *LastAttemptStatus = LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES;
      DEBUG((DEBUG_ERROR, "GetFspImageOnType - %r\n", Status));
      break;
    }

    Status = UpdateFsp(FspStorageBase, FspImage, FspImageSize, LastAttemptStatus);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Fail to update FSP image\n"));
      break;
    }
  }

  return Status;
}


