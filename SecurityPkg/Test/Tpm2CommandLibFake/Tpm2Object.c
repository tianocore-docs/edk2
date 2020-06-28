/** @file
  Implement TPM2 Object related command.

Copyright (c) 2017 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  This command allows access to the public area of a loaded object.

  @param[in]  ObjectHandle            TPM handle of an object
  @param[out] OutPublic               Structure containing the public area of an object
  @param[out] Name                    Name of the object
  @param[out] QualifiedName           The Qualified Name of the object

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ReadPublic (
  IN  TPMI_DH_OBJECT            ObjectHandle,
  OUT TPM2B_PUBLIC              *OutPublic,
  OUT TPM2B_NAME                *Name,
  OUT TPM2B_NAME                *QualifiedName
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}
