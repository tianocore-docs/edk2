/** @file

Copyright (c) 2015 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Tcg2Acpi.h"

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  // Flags field is replaced in version 4 and above
  //    BIT0~15:  PlatformClass      This field is only valid for version 4 and above
  //    BIT16~31: Reserved
  UINT32                      Flags;
  UINT64                      AddressOfControlArea;
  UINT32                      StartMethod;
  UINT8                       PlatformSpecificParameters[12];  // size up to 12
  UINT32                      Laml;                          // Optional
  UINT64                      Lasa;                          // Optional
} EFI_TPM2_ACPI_TABLE_V4;

#pragma pack()

EFI_TPM2_ACPI_TABLE_V4  mTpm2AcpiTemplate = {
  {
    EFI_ACPI_5_0_TRUSTED_COMPUTING_PLATFORM_2_TABLE_SIGNATURE,
    sizeof (mTpm2AcpiTemplate),
    EFI_TPM2_ACPI_TABLE_REVISION,
    //
    // Compiler initializes the remaining bytes to 0
    // These fields should be filled in in production
    //
  },
  0, // BIT0~15:  PlatformClass
     // BIT16~31: Reserved
  0, // Control Area
  EFI_TPM2_ACPI_TABLE_START_METHOD_TIS, // StartMethod
};

/**
  Publish TPM2 ACPI table

  @retval   EFI_SUCCESS     The TPM2 ACPI table is published successfully.
  @retval   Others          The TPM2 ACPI table is not published.

**/
EFI_STATUS
PublishTpm2 (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINTN                          TableKey;
  UINT64                         OemTableId;
  EFI_TPM2_ACPI_CONTROL_AREA     *ControlArea;
  TPM2_PTP_INTERFACE_TYPE        InterfaceType;

  //
  // Measure to PCR[0] with event EV_POST_CODE ACPI DATA
  //
  TpmMeasureAndLogData(
    0,
    EV_POST_CODE,
    EV_POSTCODE_INFO_ACPI_DATA,
    ACPI_DATA_LEN,
    &mTpm2AcpiTemplate,
    sizeof(mTpm2AcpiTemplate)
    );

  mTpm2AcpiTemplate.Header.Revision = PcdGet8(PcdTpm2AcpiTableRev);
  DEBUG((DEBUG_INFO, "Tpm2 ACPI table revision is %d\n", mTpm2AcpiTemplate.Header.Revision));

  //
  // PlatformClass is only valid for version 4 and above
  //    BIT0~15:  PlatformClass
  //    BIT16~31: Reserved
  //
  if (mTpm2AcpiTemplate.Header.Revision >= EFI_TPM2_ACPI_TABLE_REVISION_4) {
    mTpm2AcpiTemplate.Flags = (mTpm2AcpiTemplate.Flags & 0xFFFF0000) | PcdGet8(PcdTpmPlatformClass);
    DEBUG((DEBUG_INFO, "Tpm2 ACPI table PlatformClass is %d\n", (mTpm2AcpiTemplate.Flags & 0x0000FFFF)));
  }

  if ((mTpm2AcpiTemplate.Header.Revision < EFI_TPM2_ACPI_TABLE_REVISION_4) ||
      (mTpm2AcpiTemplate.Laml == 0) || (mTpm2AcpiTemplate.Lasa == 0)) {
    //
    // If version is smaller than 4 or Laml/Lasa is not valid, rollback to original Length.
    //
    mTpm2AcpiTemplate.Header.Length = sizeof(EFI_TPM2_ACPI_TABLE);
  }

  InterfaceType = PcdGet8(PcdActiveTpmInterfaceType);
  switch (InterfaceType) {
  case Tpm2PtpInterfaceCrb:
    mTpm2AcpiTemplate.StartMethod = EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE;
    mTpm2AcpiTemplate.AddressOfControlArea = PcdGet64 (PcdTpmBaseAddress) + 0x40;
    ControlArea = (EFI_TPM2_ACPI_CONTROL_AREA *)(UINTN)mTpm2AcpiTemplate.AddressOfControlArea;
    ControlArea->CommandSize  = 0xF80;
    ControlArea->ResponseSize = 0xF80;
    ControlArea->Command      = PcdGet64 (PcdTpmBaseAddress) + 0x80;
    ControlArea->Response     = PcdGet64 (PcdTpmBaseAddress) + 0x80;
    break;
  case Tpm2PtpInterfaceFifo:
  case Tpm2PtpInterfaceTis:
    break;
  default:
    DEBUG((EFI_D_ERROR, "TPM2 InterfaceType get error! %d\n", InterfaceType));
    break;
  }

  CopyMem (mTpm2AcpiTemplate.Header.OemId, PcdGetPtr (PcdAcpiDefaultOemId), sizeof (mTpm2AcpiTemplate.Header.OemId));
  OemTableId = PcdGet64 (PcdAcpiDefaultOemTableId);
  CopyMem (&mTpm2AcpiTemplate.Header.OemTableId, &OemTableId, sizeof (UINT64));
  mTpm2AcpiTemplate.Header.OemRevision      = PcdGet32 (PcdAcpiDefaultOemRevision);
  mTpm2AcpiTemplate.Header.CreatorId        = PcdGet32 (PcdAcpiDefaultCreatorId);
  mTpm2AcpiTemplate.Header.CreatorRevision  = PcdGet32 (PcdAcpiDefaultCreatorRevision);

  //
  // Construct ACPI table
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR (Status);

  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        &mTpm2AcpiTemplate,
                        mTpm2AcpiTemplate.Header.Length,
                        &TableKey
                        );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  The driver's entry point.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval Others          Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeTcgAcpi (
  IN EFI_HANDLE                  ImageHandle,
  IN EFI_SYSTEM_TABLE            *SystemTable
  )
{
  EFI_STATUS                     Status;

  //
  // Set TPM2 ACPI table
  //
  Status = PublishTpm2 ();
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

