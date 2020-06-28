/** @file
  The module entry point for Tcg2 configuration module.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PcdLib.h>
#include <Guid/TpmInstance.h>
#include <Ppi/TpmInitialized.h>

CONST EFI_PEI_PPI_DESCRIPTOR gTpmSelectedPpi = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiTpmDeviceSelectedGuid,
  NULL
};

/**
  The entry point for Tcg2 configuration driver.

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS            Convert variable to PCD successfully.
  @retval Others                 Fail to convert variable to PCD.
**/
EFI_STATUS
EFIAPI
Tcg2ConfigPeimEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  UINTN                           Size;
  EFI_STATUS                      Status;

  Size = sizeof(gEfiTpmDeviceInstanceTpm20DtpmGuid);
  Status = PcdSetPtrS (PcdTpmInstanceGuid, &Size, &gEfiTpmDeviceInstanceTpm20DtpmGuid);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, "TpmDevice PCD: %g\n", &gEfiTpmDeviceInstanceTpm20DtpmGuid));

  //
  // Selection done
  //
  Status = PeiServicesInstallPpi (&gTpmSelectedPpi);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
