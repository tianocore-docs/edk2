/** @file
  ACPI Table Library

  Copyright(c) 2014 Intel Corporation. All rights reserved.

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

#ifndef _ACPI_TABLE_H_
#define _ACPI_TABLE_H_


#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
#include <Guid/Acpi.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/DxeSmmReadyToLock.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PcdLib.h>

//
// Statements that include other files
//
#include <IndustryStandard/Acpi.h>

//
// Private Driver Data
//
//
// ACPI Table Linked List Signature.
//
#define EFI_ACPI_TABLE_LIST_SIGNATURE SIGNATURE_32 ('E', 'A', 'T', 'L')

//
// ACPI Table Linked List Entry definition.
//
//  Signature must be set to EFI_ACPI_TABLE_LIST_SIGNATURE
//  Link is the linked list data.
//  Version is the versions of the ACPI tables that this table belongs in.
//  Table is a pointer to the table.
//  PageAddress is the address of the pages allocated for the table.
//  NumberOfPages is the number of pages allocated at PageAddress.
//  Handle is used to identify a particular table.
//
typedef struct {
  UINT32                  Signature;
  LIST_ENTRY              Link;
  EFI_ACPI_COMMON_HEADER  *Table;
  EFI_PHYSICAL_ADDRESS    PageAddress;
  UINTN                   NumberOfPages;
  UINTN                   Handle;
} EFI_ACPI_TABLE_LIST;

//
// Containment record for ACPI Table linked list.
//
#define EFI_ACPI_TABLE_LIST_FROM_LINK(_link)  CR (_link, EFI_ACPI_TABLE_LIST, Link, EFI_ACPI_TABLE_LIST_SIGNATURE)

//
// The maximum number of tables this driver supports
//
#define EFI_ACPI_MAX_NUM_TABLES 20

//
// Protocol private structure definition
//

//
// ACPI support protocol instance data structure
//
typedef struct {
  EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp1;                 // Pointer to RSD_PTR structure
  EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp3;                 // Pointer to RSD_PTR structure
  EFI_ACPI_DESCRIPTION_HEADER                   *Rsdt3;                 // Pointer to RSDT table header
  EFI_ACPI_DESCRIPTION_HEADER                   *Xsdt;                  // Pointer to XSDT table header
  EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE     *Fadt3;                 // Pointer to FADT table header
  EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *Facs3;                 // Pointer to FACS table header
  EFI_ACPI_DESCRIPTION_HEADER                   *Dsdt3;                 // Pointer to DSDT table header
  LIST_ENTRY                                    TableList;
  UINTN                                         NumberOfTableEntries3;  // Number of ACPI 3.0 tables
  BOOLEAN                                       TablesInstalled3;       // ACPI 3.0 tables published
} EFI_ACPI_TABLE_INSTANCE;

//
// Protocol Constructor functions
//

/**
  Constructor for the ACPI support protocol.  Initializes instance
  data.

  @param  AcpiTableInstance       Instance to construct

  @return EFI_SUCCESS             Instance initialized.
  @return EFI_OUT_OF_RESOURCES    Unable to allocate required resources.

**/
EFI_STATUS
AcpiTableAcpiTableConstructor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

/**

  This function calculates and updates an UINT8 checksum.

  @param[in]  Buffer          Pointer to buffer to checksum
  @param[in]  Size            Number of bytes to checksum
  @param[in]  ChecksumOffset  Offset to place the checksum result in

  @retval EFI_SUCCESS             The function completed successfully.

**/
EFI_STATUS
AcpiPlatformChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  );

#endif
