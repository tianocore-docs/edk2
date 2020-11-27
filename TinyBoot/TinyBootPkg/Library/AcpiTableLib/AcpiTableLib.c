/** @file
  ACPI Table Lib Implementation

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

//
// Includes
//
#include "AcpiTable.h"
//
// The maximum number of tables that pre-allocated. 
//
UINTN         mEfiAcpiMaxNumTables = EFI_ACPI_MAX_NUM_TABLES; 

EFI_ACPI_TABLE_INSTANCE   mAcpiTableInstance;

/**
  This function adds an ACPI table to the table list.  It will detect FACS and
  allocate the correct type of memory and properly align the table.

  @param  AcpiTableInstance         Instance of the protocol.
  @param  Table                     Table to add.
  @param  Checksum                  Does the table require checksumming.
  @param  Version                   The version of the list to add the table to.
  @param  Handle                    Pointer for returning the handle.

  @return EFI_SUCCESS               The function completed successfully.
  @return EFI_OUT_OF_RESOURCES      Could not allocate a required resource.
  @return EFI_ABORTED               The table is a duplicate of a table that is required
                                    to be unique.

**/
EFI_STATUS
AddTableToList (
  IN EFI_ACPI_TABLE_INSTANCE              *AcpiTableInstance,
  IN VOID                                 *Table
  );

/**
  This function calculates and updates an UINT8 checksum.

  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum
  @param  ChecksumOffset  Offset to place the checksum result in

  @return EFI_SUCCESS             The function completed successfully.
**/
EFI_STATUS
AcpiPlatformChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  );

/**
  Checksum all versions of the common tables, RSDP, RSDT, XSDT.

  @param  AcpiTableInstance  Protocol instance private data.

  @return EFI_SUCCESS        The function completed successfully.

**/
EFI_STATUS
ChecksumCommonTables (
  IN OUT EFI_ACPI_TABLE_INSTANCE          *AcpiTableInstance
  );

//
// Protocol function implementations.
//

/**
  This function publishes the specified versions of the ACPI tables by
  installing EFI configuration table entries for them.  Any combination of
  table versions can be published.

  @param  AcpiTableInstance  Instance of the protocol.
  @param  Version            Version(s) to publish.

  @return EFI_SUCCESS  The function completed successfully.
  @return EFI_ABORTED  The function could not complete successfully.

**/
EFI_STATUS
EFIAPI
PublishTables (
  IN EFI_ACPI_TABLE_INSTANCE              *AcpiTableInstance
  )
{
  EFI_STATUS                Status;
  UINT32                    *CurrentRsdtEntry;
  VOID                      *CurrentXsdtEntry;
  UINT64                    Buffer64;

  //
  // Reorder tables as some operating systems don't seem to find the
  // FADT correctly if it is not in the first few entries
  //

  //
  // Add FADT as the first entry
  //
  {
    CurrentRsdtEntry  = (UINT32 *) ((UINT8 *) AcpiTableInstance->Rsdt3 + sizeof (EFI_ACPI_DESCRIPTION_HEADER));
    *CurrentRsdtEntry = (UINT32) (UINTN) AcpiTableInstance->Fadt3;
    CurrentXsdtEntry  = (VOID *) ((UINT8 *) AcpiTableInstance->Xsdt + sizeof (EFI_ACPI_DESCRIPTION_HEADER));
    //
    // Add entry to XSDT, XSDT expects 64 bit pointers, but
    // the table pointers in XSDT are not aligned on 8 byte boundary.
    //
    Buffer64 = (UINT64) (UINTN) AcpiTableInstance->Fadt3;
    CopyMem (
      CurrentXsdtEntry,
      &Buffer64,
      sizeof (UINT64)
      );
  }

  //
  // Do checksum again because Dsdt/Xsdt is updated.
  //
  ChecksumCommonTables (AcpiTableInstance);

  //
  // Add the RSD_PTR to the system table and store that we have installed the
  // tables.
  //
  if (!AcpiTableInstance->TablesInstalled3) {
    Status = gBS->InstallConfigurationTable (&gEfiAcpi10TableGuid, AcpiTableInstance->Rsdp1);
    if (EFI_ERROR (Status)) {
      return EFI_ABORTED;
    }
    Status = gBS->InstallConfigurationTable (&gEfiAcpiTableGuid, AcpiTableInstance->Rsdp3);
    if (EFI_ERROR (Status)) {
      return EFI_ABORTED;
    }
    AcpiTableInstance->TablesInstalled3= TRUE;
  }

  return EFI_SUCCESS;
}


/**
  Installs an ACPI table into the RSDT/XSDT.
  Note that the ACPI table should be checksumed before installing it.
  Otherwise it will assert.

  @param  AcpiTableBuffer      A pointer to a buffer containing the ACPI table to be installed.
  @param  AcpiTableBufferSize  Specifies the size, in bytes, of the AcpiTableBuffer buffer.

  @return EFI_SUCCESS            The table was successfully inserted.
  @return EFI_INVALID_PARAMETER  Either AcpiTableBuffer is NULL, TableKey is NULL, or AcpiTableBufferSize 
                                 and the size field embedded in the ACPI table pointed to by AcpiTableBuffer
                                 are not in sync.
  @return EFI_OUT_OF_RESOURCES   Insufficient resources exist to complete the request.

**/
EFI_STATUS
EFIAPI
InstallAcpiTable (
  IN   VOID                                       *AcpiTableBuffer,
  IN   UINTN                                      AcpiTableBufferSize
  )
{
  EFI_ACPI_TABLE_INSTANCE   *AcpiTableInstance;
  EFI_STATUS                Status;
  VOID                      *AcpiTableBufferConst;

  //
  // Check for invalid input parameters
  //
  if ((AcpiTableBuffer == NULL) ||
      (((EFI_ACPI_DESCRIPTION_HEADER *) AcpiTableBuffer)->Length != AcpiTableBufferSize)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the instance of the ACPI table protocol
  //
  AcpiTableInstance = &mAcpiTableInstance;

  //
  // Install the ACPI table
  //
  AcpiTableBufferConst = AllocateCopyPool (AcpiTableBufferSize,AcpiTableBuffer);
  Status = AddTableToList (
             AcpiTableInstance,
             AcpiTableBufferConst
             );
  if (!EFI_ERROR (Status)) {
    Status = PublishTables (
               AcpiTableInstance
               );
  }
  FreePool (AcpiTableBufferConst);

  return Status;
}

/**
  This function adds an ACPI table to the table list.  It will detect FACS and
  allocate the correct type of memory and properly align the table.

  @param  AcpiTableInstance         Instance of the protocol.
  @param  Table                     Table to add.
  @param  Checksum                  Does the table require checksumming.
  @param  Version                   The version of the list to add the table to.
  @param  Handle                    Pointer for returning the handle.

  @return EFI_SUCCESS               The function completed successfully.
  @return EFI_OUT_OF_RESOURCES      Could not allocate a required resource.
  @return EFI_ABORTED               The table is a duplicate of a table that is required
                                    to be unique.

**/
EFI_STATUS
AddTableToList (
  IN EFI_ACPI_TABLE_INSTANCE              *AcpiTableInstance,
  IN VOID                                 *Table
  )
{
  EFI_STATUS          Status;
  EFI_ACPI_TABLE_LIST *CurrentTableList;
  UINT32              CurrentTableSignature;
  UINT32              CurrentTableSize;
  UINT32              *CurrentRsdtEntry;
  VOID                *CurrentXsdtEntry;
  UINT64              Buffer64;
  BOOLEAN             AddToRsdt;

  //
  // Init locals
  //
  AddToRsdt = TRUE;

  //
  // Create a new list entry
  //
  CurrentTableList = AllocatePool (sizeof (EFI_ACPI_TABLE_LIST));
  ASSERT (CurrentTableList);

  //
  // Determine table type and size
  //
  CurrentTableSignature = ((EFI_ACPI_COMMON_HEADER *) Table)->Signature;
  CurrentTableSize      = ((EFI_ACPI_COMMON_HEADER *) Table)->Length;

  //
  // Allocate a buffer for the table.  All tables are allocated in the lower 32 bits of address space
  // for backwards compatibility with ACPI 1.0 OS.
  //
  // This is done because ACPI 1.0 pointers are 32 bit values.
  // ACPI 2.0 OS and all 64 bit OS must use the 64 bit ACPI table addresses.
  // There is no architectural reason these should be below 4GB, it is purely
  // for convenience of implementation that we force memory below 4GB.
  //
  CurrentTableList->PageAddress   = 0xFFFFFFFF;
  CurrentTableList->NumberOfPages = EFI_SIZE_TO_PAGES (CurrentTableSize);

  //
  // Allocation memory type depends on the type of the table
  //
  if ((CurrentTableSignature == EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) || 
      (CurrentTableSignature == EFI_ACPI_4_0_UEFI_ACPI_DATA_TABLE_SIGNATURE)) {
    //
    // Allocate memory for the FACS.  This structure must be aligned
    // on a 64 byte boundary and must be ACPI NVS memory.
    // Using AllocatePages should ensure that it is always aligned.
    // Do not change signature for new ACPI version because they are same.
    //
    // UEFI table also need to be in ACPI NVS memory, because some data field
    // could be updated by OS present agent. For example, BufferPtrAddress in
    // SMM communication ACPI table.
    //
    ASSERT ((EFI_PAGE_SIZE % 64) == 0);
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIMemoryNVS,
                    CurrentTableList->NumberOfPages,
                    &CurrentTableList->PageAddress
                    );
  } else {
    //
    // All other tables are ACPI reclaim memory, no alignment requirements.
    //
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIReclaimMemory,
                    CurrentTableList->NumberOfPages,
                    &CurrentTableList->PageAddress
                    );
  }
  //
  // Check return value from memory alloc.
  //
  if (EFI_ERROR (Status)) {
    gBS->FreePool (CurrentTableList);
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Update the table pointer with the allocated memory start
  //
  CurrentTableList->Table = (EFI_ACPI_COMMON_HEADER *) (UINTN) CurrentTableList->PageAddress;

  //
  // Initialize the table contents
  //
  CurrentTableList->Signature = EFI_ACPI_TABLE_LIST_SIGNATURE;
  CopyMem (CurrentTableList->Table, Table, CurrentTableSize);

  //
  // Update internal pointers if this is a required table.  If it is a required
  // table and a table of that type already exists, return an error.
  //
  // Calculate the checksum if the table is not FACS.
  //
  switch (CurrentTableSignature) {

  case EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
    //
    // We don't add the FADT in the standard way because some
    // OS expect the FADT to be early in the table list.
    // So we always add it as the first element in the list.
    //
    AddToRsdt = FALSE;

    //
    // Check that the table has not been previously added.
    //
    if (AcpiTableInstance->Fadt3 != NULL) {
      gBS->FreePages (CurrentTableList->PageAddress, CurrentTableList->NumberOfPages);
      gBS->FreePool (CurrentTableList);
      return EFI_ABORTED;
    }
    //
    // Add the table to the appropriate table version
    //

      //
      // Save a pointer to the table
      //
      AcpiTableInstance->Fadt3 = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *) CurrentTableList->Table;

      //
      // Update pointers in FADT.  If tables don't exist this will put NULL pointers there.
      // Note: If the FIRMWARE_CTRL is non-zero, then X_FIRMWARE_CTRL must be zero, and 
      // vice-versa.
      //
      if ((UINT64)(UINTN)AcpiTableInstance->Facs3 < BASE_4GB) {
        AcpiTableInstance->Fadt3->FirmwareCtrl  = (UINT32) (UINTN) AcpiTableInstance->Facs3;
      } else {
        Buffer64 = (UINT64) (UINTN) AcpiTableInstance->Facs3;
        CopyMem (
          &AcpiTableInstance->Fadt3->XFirmwareCtrl,
          &Buffer64,
          sizeof (UINT64)
          );
      }
      AcpiTableInstance->Fadt3->Dsdt  = (UINT32) (UINTN) AcpiTableInstance->Dsdt3;
      Buffer64                          = (UINT64) (UINTN) AcpiTableInstance->Dsdt3;
      CopyMem (
        &AcpiTableInstance->Fadt3->XDsdt,
        &Buffer64,
        sizeof (UINT64)
        );

      //
      // RSDP OEM information is updated to match the FADT OEM information
      //
      CopyMem (
        &AcpiTableInstance->Rsdp3->OemId,
        &AcpiTableInstance->Fadt3->Header.OemId,
        6
        );
      
      //
      // RSDT OEM information is updated to match FADT OEM information.
      //
      CopyMem (
        &AcpiTableInstance->Rsdt3->OemId,
        &AcpiTableInstance->Fadt3->Header.OemId,
        6
        );
      CopyMem (
        &AcpiTableInstance->Rsdt3->OemTableId,
        &AcpiTableInstance->Fadt3->Header.OemTableId,
        sizeof (UINT64)
        );
      AcpiTableInstance->Rsdt3->OemRevision = AcpiTableInstance->Fadt3->Header.OemRevision;
      
      //
      // XSDT OEM information is updated to match FADT OEM information.
      //
      CopyMem (
        &AcpiTableInstance->Xsdt->OemId,
        &AcpiTableInstance->Fadt3->Header.OemId,
        6
        );
      CopyMem (
        &AcpiTableInstance->Xsdt->OemTableId,
        &AcpiTableInstance->Fadt3->Header.OemTableId,
        sizeof (UINT64)
        );
      AcpiTableInstance->Xsdt->OemRevision = AcpiTableInstance->Fadt3->Header.OemRevision;

    //
    // Checksum the table
    //
    AcpiPlatformChecksum (
      CurrentTableList->Table,
      CurrentTableList->Table->Length,
      OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
      Checksum)
      );
    break;

  case EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
    //
    // Check that the table has not been previously added.
    //
    if (AcpiTableInstance->Facs3 != NULL) {
      gBS->FreePages (CurrentTableList->PageAddress, CurrentTableList->NumberOfPages);
      gBS->FreePool (CurrentTableList);
      return EFI_ABORTED;
    }
    //
    // FACS is referenced by FADT and is not part of RSDT
    //
    AddToRsdt = FALSE;

      //
      // Save a pointer to the table
      //
      AcpiTableInstance->Facs3 = (EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *) CurrentTableList->Table;

      //
      // If FADT already exists, update table pointers.
      //
      if (AcpiTableInstance->Fadt3 != NULL) {
        //
        // Note: If the FIRMWARE_CTRL is non-zero, then X_FIRMWARE_CTRL must be zero, and 
        // vice-versa.
        //
        if ((UINT64)(UINTN)AcpiTableInstance->Facs3 < BASE_4GB) {
          AcpiTableInstance->Fadt3->FirmwareCtrl  = (UINT32) (UINTN) AcpiTableInstance->Facs3;
        } else {
          Buffer64 = (UINT64) (UINTN) AcpiTableInstance->Facs3;
          CopyMem (
            &AcpiTableInstance->Fadt3->XFirmwareCtrl,
            &Buffer64,
            sizeof (UINT64)
            );
        }

        //
        // Checksum FADT table
        //
        AcpiPlatformChecksum (
          AcpiTableInstance->Fadt3,
          AcpiTableInstance->Fadt3->Header.Length,
          OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
          Checksum)
          );
      }

    break;

  case EFI_ACPI_1_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    //
    // Check that the table has not been previously added.
    //
    if (AcpiTableInstance->Dsdt3 != NULL) {
      gBS->FreePages (CurrentTableList->PageAddress, CurrentTableList->NumberOfPages);
      gBS->FreePool (CurrentTableList);
      return EFI_ABORTED;
    }
    //
    // DSDT is referenced by FADT and is not part of RSDT
    //
    AddToRsdt = FALSE;
    
      //
      // Save a pointer to the table
      //
      AcpiTableInstance->Dsdt3 = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTableList->Table;

      //
      // If FADT already exists, update table pointers.
      //
      if (AcpiTableInstance->Fadt3 != NULL) {
        AcpiTableInstance->Fadt3->Dsdt  = (UINT32) (UINTN) AcpiTableInstance->Dsdt3;
        Buffer64                          = (UINT64) (UINTN) AcpiTableInstance->Dsdt3;
        CopyMem (
          &AcpiTableInstance->Fadt3->XDsdt,
          &Buffer64,
          sizeof (UINT64)
          );

        //
        // Checksum FADT table
        //
        AcpiPlatformChecksum (
          AcpiTableInstance->Fadt3,
          AcpiTableInstance->Fadt3->Header.Length,
          OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
          Checksum)
          );
      }

    //
    // Checksum the table
    //
    AcpiPlatformChecksum (
      CurrentTableList->Table,
      CurrentTableList->Table->Length,
      OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
      Checksum)
      );
    break;

  default:
    //
    // Checksum the table
    //
    AcpiPlatformChecksum (
      CurrentTableList->Table,
      CurrentTableList->Table->Length,
      OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
      Checksum)
      );
    break;
  }
  //
  // Add the table to the current list of tables
  //
  InsertTailList (&AcpiTableInstance->TableList, &CurrentTableList->Link);

  //
  // Add the table to RSDT and/or XSDT table entry lists.
  //
  //
  // Add to ACPI 1.0b table tree
  //
  //
  // Add to ACPI 2.0/3.0  table tree
  //
     if (AddToRsdt) {
       //
       // If the table number exceed the gEfiAcpiMaxNumTables, enlarge the table buffer
       //
       ASSERT (AcpiTableInstance->NumberOfTableEntries3 < mEfiAcpiMaxNumTables);
       //
       // At this time, it is assumed that RSDT and XSDT maintain parallel lists of tables.
       // If it becomes necessary to maintain separate table lists, changes will be required.
       //
       CurrentRsdtEntry = (UINT32 *)
         (
           (UINT8 *) AcpiTableInstance->Rsdt3 +
           sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
           AcpiTableInstance->NumberOfTableEntries3 *
           sizeof (UINT32)
         );

       //
       // This pointer must not be directly dereferenced as the XSDT entries may not
       // be 64 bit aligned resulting in a possible fault.  Use CopyMem to update.
       //
       CurrentXsdtEntry = (VOID *)
         (
           (UINT8 *) AcpiTableInstance->Xsdt +
           sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
           AcpiTableInstance->NumberOfTableEntries3 *
           sizeof (UINT64)
         );

       //
       // Add entry to the RSDT
       //
       *CurrentRsdtEntry = (UINT32) (UINTN) CurrentTableList->Table;

       //
       // Update RSDT length
       //
       AcpiTableInstance->Rsdt3->Length = AcpiTableInstance->Rsdt3->Length + sizeof (UINT32);

       //
       // Add entry to XSDT, XSDT expects 64 bit pointers, but
       // the table pointers in XSDT are not aligned on 8 byte boundary.
       //
       Buffer64 = (UINT64) (UINTN) CurrentTableList->Table;
       CopyMem (
         CurrentXsdtEntry,
         &Buffer64,
         sizeof (UINT64)
         );

       //
       // Update length
       //
       AcpiTableInstance->Xsdt->Length = AcpiTableInstance->Xsdt->Length + sizeof (UINT64);

       AcpiTableInstance->NumberOfTableEntries3++;
    }

  ChecksumCommonTables (AcpiTableInstance);
  return EFI_SUCCESS;
}

/**
  This function calculates and updates an UINT8 checksum.

  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum
  @param  ChecksumOffset  Offset to place the checksum result in

  @return EFI_SUCCESS             The function completed successfully.

**/
EFI_STATUS
AcpiPlatformChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  )
{
  UINT8 Sum;
  UINT8 *Ptr;

  Sum = 0;
  //
  // Initialize pointer
  //
  Ptr = Buffer;

  //
  // set checksum to 0 first
  //
  Ptr[ChecksumOffset] = 0;

  //
  // add all content of buffer
  //
  while ((Size--) != 0) {
    Sum = (UINT8) (Sum + (*Ptr++));
  }
  //
  // set checksum
  //
  Ptr                 = Buffer;
  Ptr[ChecksumOffset] = (UINT8) (0xff - Sum + 1);

  return EFI_SUCCESS;
}


/**
  Checksum all versions of the common tables, RSDP, RSDT, XSDT.

  @param  AcpiTableInstance  Protocol instance private data.

  @return EFI_SUCCESS        The function completed successfully.

**/
EFI_STATUS
ChecksumCommonTables (
  IN OUT EFI_ACPI_TABLE_INSTANCE                   *AcpiTableInstance
  )
{
  //
  // RSDP ACPI 1.0 checksum for 1.0 table.  This is only the first 20 bytes of the structure
  //
  AcpiPlatformChecksum (
    AcpiTableInstance->Rsdp1,
    sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER),
    OFFSET_OF (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER,
    Checksum)
    );

  //
  // RSDP ACPI 1.0 checksum for 2.0/3.0 table.  This is only the first 20 bytes of the structure
  //
  AcpiPlatformChecksum (
    AcpiTableInstance->Rsdp3,
    sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER),
    OFFSET_OF (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER,
    Checksum)
    );

  //
  // RSDP ACPI 2.0/3.0 checksum, this is the entire table
  //
  AcpiPlatformChecksum (
    AcpiTableInstance->Rsdp3,
    sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER),
    OFFSET_OF (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER,
    ExtendedChecksum)
    );

  //
  // RSDT checksums
  //

  AcpiPlatformChecksum (
    AcpiTableInstance->Rsdt3,
    AcpiTableInstance->Rsdt3->Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
    Checksum)
    );

  //
  // XSDT checksum
  //
  AcpiPlatformChecksum (
    AcpiTableInstance->Xsdt,
    AcpiTableInstance->Xsdt->Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
    Checksum)
    );
  
  return EFI_SUCCESS;
}


/**
  Constructor for the ACPI table protocol.  Initializes instance
  data.

  @param  AcpiTableInstance   Instance to construct

  @return EFI_SUCCESS             Instance initialized.
  @return EFI_OUT_OF_RESOURCES    Unable to allocate required resources.

**/
EFI_STATUS
AcpiTableAcpiTableConstructor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS            Status;
  UINT64                CurrentData;
  UINTN                 TotalSize;
  UINTN                 RsdpTableSize;
  UINT8                 *Pointer;
  EFI_PHYSICAL_ADDRESS  PageAddress;

  EFI_ACPI_TABLE_INSTANCE   *AcpiTableInstance;

  //
  // Initialize our protocol
  //
  AcpiTableInstance = &mAcpiTableInstance;

  InitializeListHead (&AcpiTableInstance->TableList);

  //
  // Create RSDP table
  //
  RsdpTableSize = sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER) +
                  sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER);

  PageAddress = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIReclaimMemory,
                  EFI_SIZE_TO_PAGES (RsdpTableSize),
                  &PageAddress
                  );

  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Pointer = (UINT8 *) (UINTN) PageAddress;
  ZeroMem (Pointer, RsdpTableSize);

  AcpiTableInstance->Rsdp1 = (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER *) Pointer;
  Pointer += sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
  AcpiTableInstance->Rsdp3 = (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *) Pointer;

  //
  // Create RSDT, XSDT structures
  //
  TotalSize = sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 2.0/3.0 RSDT
              mEfiAcpiMaxNumTables * sizeof (UINT32) +
              sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 2.0/3.0 XSDT
              mEfiAcpiMaxNumTables * sizeof (UINT64);

  //
  // Allocate memory in the lower 32 bit of address range for
  // compatibility with ACPI 1.0 OS.
  //
  // This is done because ACPI 1.0 pointers are 32 bit values.
  // ACPI 2.0 OS and all 64 bit OS must use the 64 bit ACPI table addresses.
  // There is no architectural reason these should be below 4GB, it is purely
  // for convenience of implementation that we force memory below 4GB.
  //
  PageAddress = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIReclaimMemory,
                  EFI_SIZE_TO_PAGES (TotalSize),
                  &PageAddress
                  );

  if (EFI_ERROR (Status)) {
    gBS->FreePages ((EFI_PHYSICAL_ADDRESS)(UINTN)AcpiTableInstance->Rsdp1, EFI_SIZE_TO_PAGES (RsdpTableSize));
    return EFI_OUT_OF_RESOURCES;
  }

  Pointer = (UINT8 *) (UINTN) PageAddress;
  ZeroMem (Pointer, TotalSize);

  AcpiTableInstance->Rsdt3 = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;
  Pointer += (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + EFI_ACPI_MAX_NUM_TABLES * sizeof (UINT32));
  AcpiTableInstance->Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;

  //
  // Initialize RSDP
  //
  CurrentData = EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE;
  CopyMem (&AcpiTableInstance->Rsdp1->Signature, &CurrentData, sizeof (UINT64));
  CopyMem (AcpiTableInstance->Rsdp1->OemId, PcdGetPtr (PcdAcpiDefaultOemId), sizeof (AcpiTableInstance->Rsdp1->OemId));
  AcpiTableInstance->Rsdp1->Reserved    = EFI_ACPI_RESERVED_BYTE;
  AcpiTableInstance->Rsdp1->RsdtAddress = (UINT32) (UINTN) AcpiTableInstance->Rsdt3;

  CurrentData = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE;
  CopyMem (&AcpiTableInstance->Rsdp3->Signature, &CurrentData, sizeof (UINT64));
  CopyMem (AcpiTableInstance->Rsdp3->OemId, PcdGetPtr (PcdAcpiDefaultOemId), sizeof (AcpiTableInstance->Rsdp3->OemId));
  AcpiTableInstance->Rsdp3->Revision    = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION;
  AcpiTableInstance->Rsdp3->RsdtAddress = (UINT32) (UINTN) AcpiTableInstance->Rsdt3;
  AcpiTableInstance->Rsdp3->Length      = sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
  CurrentData = (UINT64) (UINTN) AcpiTableInstance->Xsdt;
  CopyMem (&AcpiTableInstance->Rsdp3->XsdtAddress, &CurrentData, sizeof (UINT64));
  SetMem (AcpiTableInstance->Rsdp3->Reserved, 3, EFI_ACPI_RESERVED_BYTE);

  //
  // Initialize Rsdt
  //
  // Note that we "reserve" one entry for the FADT so it can always be
  // at the beginning of the list of tables.  Some OS don't seem
  // to find it correctly if it is too far down the list.
  //
  AcpiTableInstance->Rsdt3->Signature       = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE;
  AcpiTableInstance->Rsdt3->Length          = sizeof (EFI_ACPI_DESCRIPTION_HEADER);
  AcpiTableInstance->Rsdt3->Revision        = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_TABLE_REVISION;
  CopyMem (AcpiTableInstance->Rsdt3->OemId, PcdGetPtr (PcdAcpiDefaultOemId), sizeof (AcpiTableInstance->Rsdt3->OemId));
  CurrentData = PcdGet64 (PcdAcpiDefaultOemTableId);
  CopyMem (&AcpiTableInstance->Rsdt3->OemTableId, &CurrentData, sizeof (UINT64));
  AcpiTableInstance->Rsdt3->OemRevision     = PcdGet32 (PcdAcpiDefaultOemRevision);
  AcpiTableInstance->Rsdt3->CreatorId       = PcdGet32 (PcdAcpiDefaultCreatorId);
  AcpiTableInstance->Rsdt3->CreatorRevision = PcdGet32 (PcdAcpiDefaultCreatorRevision);
  //
  // We always reserve first one for FADT
  //
  AcpiTableInstance->NumberOfTableEntries3  = 1;
  AcpiTableInstance->Rsdt3->Length          = AcpiTableInstance->Rsdt3->Length + sizeof(UINT32);

  //
  // Initialize Xsdt
  //
  AcpiTableInstance->Xsdt->Signature  = EFI_ACPI_3_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE;
  AcpiTableInstance->Xsdt->Length     = sizeof (EFI_ACPI_DESCRIPTION_HEADER);
  AcpiTableInstance->Xsdt->Revision   = EFI_ACPI_3_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_REVISION;
  CopyMem (AcpiTableInstance->Xsdt->OemId, PcdGetPtr (PcdAcpiDefaultOemId), sizeof (AcpiTableInstance->Xsdt->OemId));
  CurrentData = PcdGet64 (PcdAcpiDefaultOemTableId);
  CopyMem (&AcpiTableInstance->Xsdt->OemTableId, &CurrentData, sizeof (UINT64));
  AcpiTableInstance->Xsdt->OemRevision      = PcdGet32 (PcdAcpiDefaultOemRevision);
  AcpiTableInstance->Xsdt->CreatorId        = PcdGet32 (PcdAcpiDefaultCreatorId);
  AcpiTableInstance->Xsdt->CreatorRevision  = PcdGet32 (PcdAcpiDefaultCreatorRevision);
  //
  // We always reserve first one for FADT
  //
  AcpiTableInstance->Xsdt->Length           = AcpiTableInstance->Xsdt->Length + sizeof(UINT64);

  ChecksumCommonTables (AcpiTableInstance);

  //
  // Completed successfully
  //
  return EFI_SUCCESS;
}

