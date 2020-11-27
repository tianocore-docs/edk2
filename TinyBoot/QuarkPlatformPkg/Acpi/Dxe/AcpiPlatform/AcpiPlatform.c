/*++
  ACPI Platform Driver

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

--*/

#include <Protocol/AcpiTable.h>
#include <PiDxe.h>
#include <Library/AcpiTableLib.h>
#include <IndustryStandard/Pci22.h>
#include "AcpiPlatform.h"

//
// Global Variable
//
STATIC EFI_GLOBAL_NVS_AREA_PROTOCOL  mGlobalNvsArea;

EFI_STATUS
LocateSupportProtocol (
  IN  EFI_GUID                       *Protocol,
  OUT VOID                           **Instance,
  IN  UINT32                         Type
  )
/*++

Routine Description:

  Locate the first instance of a protocol.  If the protocol requested is an
  FV protocol, then it will return the first FV that contains the ACPI table
  storage file.

Arguments:

  Protocol      The protocol to find.
  Instance      Return pointer to the first instance of the protocol

Returns:

  EFI_SUCCESS           The function completed successfully.
  EFI_NOT_FOUND         The protocol could not be located.
  EFI_OUT_OF_RESOURCES  There are not enough resources to find the protocol.

--*/
{
  EFI_STATUS              Status;
  EFI_HANDLE              *HandleBuffer;
  UINTN                   NumberOfHandles;
  EFI_FV_FILETYPE         FileType;
  UINT32                  FvStatus;
  EFI_FV_FILE_ATTRIBUTES  Attributes;
  UINTN                   Size;
  UINTN                   i;

  FvStatus = 0;

  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   Protocol,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );
  if (EFI_ERROR (Status)) {

    //
    // Defined errors at this time are not found and out of resources.
    //
    return Status;
  }



  //
  // Looking for FV with ACPI storage file
  //

  for (i = 0; i < NumberOfHandles; i++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol (
                     HandleBuffer[i],
                     Protocol,
                     Instance
                     );
    ASSERT_EFI_ERROR (Status);

    if (!Type) {
      //
      // Not looking for the FV protocol, so find the first instance of the
      // protocol.  There should not be any errors because our handle buffer
      // should always contain at least one or LocateHandleBuffer would have
      // returned not found.
      //
      break;
    }

    //
    // See if it has the ACPI storage file
    //

    Status = ((EFI_FIRMWARE_VOLUME2_PROTOCOL*) (*Instance))->ReadFile (*Instance,
                                                              (EFI_GUID*)PcdGetPtr (PcdAcpiTableStorageFile),
                                                              NULL,
                                                              &Size,
                                                              &FileType,
                                                              &Attributes,
                                                              &FvStatus
                                                              );

    //
    // If we found it, then we are done
    //
    if (Status == EFI_SUCCESS) {
      break;
    }
  }

  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //

  //
  // Free any allocated buffers
  //
  gBS->FreePool (HandleBuffer);

  return Status;
}


VOID
DsdtTableUpdate (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader,
  IN OUT   EFI_ACPI_TABLE_VERSION       *Version
  )
/*++

  Routine Description:

    Update the DSDT table

  Arguments:

    Table   - The table to be set
    Version - Version to publish

  Returns:

    None

--*/
{

  UINT8      *CurrPtr;
  UINT8      *DsdtPointer;
  UINT32     *Signature;
  UINT8      *Operation;
  UINT32     *Address;
  UINT16     *Size;
  //
  // Loop through the ASL looking for values that we must fix up.
  //
  CurrPtr = (UINT8 *) TableHeader;
  for (DsdtPointer = CurrPtr;
       DsdtPointer <= (CurrPtr + ((EFI_ACPI_COMMON_HEADER *) CurrPtr)->Length);
       DsdtPointer++
      )
  {
    Signature = (UINT32 *) DsdtPointer;
    switch (*Signature) {
    //
    // MNVS operation region
    //
    case (SIGNATURE_32 ('M', 'N', 'V', 'S')):
      //
      // Conditional match.  For Region Objects, the Operator will always be the
      // byte immediately before the specific name.  Therefore, subtract 1 to check
      // the Operator.
      //
      Operation = DsdtPointer - 1;
      if (*Operation == AML_OPREGION_OP) {
        Address   = (UINT32 *) (DsdtPointer + 6);
        *Address  = (UINT32) (UINTN) mGlobalNvsArea.Area;
        Size      = (UINT16 *) (DsdtPointer + 11);
        *Size     = sizeof (EFI_GLOBAL_NVS_AREA);
      }
      break;
      
    //
    // Update processor PBLK register I/O base address
    //
    case (SIGNATURE_32 ('P', 'R', 'I', 'O')):
      //
      // Conditional match. Update the following ASL code:
      // Processor (CPU0, 0x01, 0x4F495250, 0x06) {}
      // The 3rd parameter will be updated to the actual PBLK I/O base address.
      // the Operator.
      //
      Operation = DsdtPointer - 8;
      if ((*Operation == AML_EXT_OP) && (*(Operation + 1) == AML_EXT_PROCESSOR_OP)) {
        *(UINT32 *)DsdtPointer = PcdGet16(PcdPmbaIoBaseAddress);
      }
      break;
    default:
      break;
    }
  }
}

VOID
AcpiUpdateTable (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader,
  IN OUT   EFI_ACPI_TABLE_VERSION       *Version
  )
/*++

  Routine Description:

    Set the correct table revision upon the setup value

  Arguments:

    Table   - The table to be set
    Version - Version to publish

  Returns:

    None

--*/

{
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *FadtHeader2;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *AllocationStructurePtr;
  
  if (TableHeader != NULL && Version != NULL) {

    *Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;
    //
    // Here we use all 3.0 signature because all version use same signature if they supported
    //
    switch (TableHeader->Signature) {
    //
    // "APIC" Multiple APIC Description Table
    //
    case EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE:
      break;
    //
    // "DSDT" Differentiated System Description Table
    //
    case EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      DsdtTableUpdate (TableHeader, Version);
      break;

    //
    // "FACP" Fixed ACPI Description Table (FADT)
    //
    case EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
      *Version = EFI_ACPI_TABLE_VERSION_NONE;
      if (TableHeader->Revision == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
      	*Version = EFI_ACPI_TABLE_VERSION_2_0;
      	FadtHeader2 = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *) TableHeader;      	
      	FadtHeader2->SmiCmd     = PcdGet16(PcdSmmActivationPort);
      	FadtHeader2->Pm1aEvtBlk = PcdGet16(PcdPm1blkIoBaseAddress);
      	FadtHeader2->Pm1aCntBlk = PcdGet16(PcdPm1blkIoBaseAddress) + R_QNC_PM1BLK_PM1C;
      	FadtHeader2->PmTmrBlk   = PcdGet16(PcdPm1blkIoBaseAddress) + R_QNC_PM1BLK_PM1T;        
      	FadtHeader2->Gpe0Blk    = PcdGet16(PcdGpe0blkIoBaseAddress);
        FadtHeader2->XPm1aEvtBlk.Address = FadtHeader2->Pm1aEvtBlk;
        FadtHeader2->XPm1aCntBlk.Address = FadtHeader2->Pm1aCntBlk;
        FadtHeader2->XPmTmrBlk.Address   = FadtHeader2->PmTmrBlk;        
        FadtHeader2->XGpe0Blk.Address    = FadtHeader2->Gpe0Blk;
      }
      break;
    //
    // "FACS" Firmware ACPI Control Structure
    //
    case EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
      break;
    //
    // "SSDT" Secondary System Description Table
    //
    case EFI_ACPI_3_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    break;
    //
    // "HPET" IA-PC High Precision Event Timer Table
    //
    case EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE:
      //
      // If HPET is disabled in setup, don't publish the table.
      //
      if (mGlobalNvsArea.Area->HpetEnable == 0) {
        *Version = EFI_ACPI_TABLE_VERSION_NONE;
      }
      ((EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER *) TableHeader)->BaseAddressLower32Bit.Address
        = PcdGet64 (PcdHpetBaseAddress);
      break;
    //
    // "SPCR" Serial Port Concole Redirection Table
    //
    case EFI_ACPI_3_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE:
      break;
    //
    // "MCFG" PCI Express Memory Mapped Configuration Space Base Address Description Table
    //
    case EFI_ACPI_3_0_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE:
      AllocationStructurePtr = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *)
        ((UINT8 *)TableHeader + sizeof(EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER));
      AllocationStructurePtr->BaseAddress = PcdGet64(PcdPciExpressBaseAddress);
      break;
    // Lakeport platform doesn't support the following table
    /*
      //
    // "ECDT" Embedded Controller Boot Resources Table
        //
    case EFI_ACPI_3_0_EMBEDDED_CONTROLLER_BOOT_RESOURCES_TABLE_SIGNATURE:
      break;
        //
    // "PSDT" Persistent System Description Table
          //
    case EFI_ACPI_3_0_PERSISTENT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      break;
          //
    // "SBST" Smart Battery Specification Table
    //
    case EFI_ACPI_3_0_SMART_BATTERY_SPECIFICATION_TABLE_SIGNATURE:
          break;
    //
    // "SLIT" System Locality Information Table
    //
    case EFI_ACPI_3_0_SYSTEM_LOCALITY_INFORMATION_TABLE_SIGNATURE:
          break;
    //
    // "SRAT" Static Resource Affinity Table
    //
    case EFI_ACPI_3_0_STATIC_RESOURCE_AFFINITY_TABLE_SIGNATURE:
    break;
  //
    // "XSDT" Extended System Description Table
  //
    case EFI_ACPI_3_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      break;
  //
    // "BOOT" MS Simple Boot Spec
  //
    case EFI_ACPI_3_0_SIMPLE_BOOT_FLAG_TABLE_SIGNATURE:
      break;
  //
    // "CPEP" Corrected Platform Error Polling Table
  //
    case EFI_ACPI_3_0_CORRECTED_PLATFORM_ERROR_POLLING_TABLE_SIGNATURE:
      break;
  //
    // "DBGP" MS Debug Port Spec
  //
    case EFI_ACPI_3_0_DEBUG_PORT_TABLE_SIGNATURE:
      break;
    //
    // "ETDT" Event Timer Description Table
    //
    case EFI_ACPI_3_0_EVENT_TIMER_DESCRIPTION_TABLE_SIGNATURE:
      break;
    //
    // "SPMI" Server Platform Management Interface Table
    //
    case EFI_ACPI_3_0_SERVER_PLATFORM_MANAGEMENT_INTERFACE_TABLE_SIGNATURE:
      break;
    //
    // "TCPA" Trusted Computing Platform Alliance Capabilities Table
    //
    case EFI_ACPI_3_0_TRUSTED_COMPUTING_PLATFORM_ALLIANCE_CAPABILITIES_TABLE_SIGNATURE:
      break;
    */
    default:
      break;
    }
  }
}

//
// Description:
//    Entrypoint of Acpi Platform driver
// In:
//    ImageHandle
//    SystemTable
// Out:
//    EFI_SUCCESS
//    EFI_LOAD_ERROR
//    EFI_OUT_OF_RESOURCES
//

EFI_STATUS
AcpiPlatformEntryPoint (
  IN EFI_HANDLE     ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol;
  INTN                         Instance;
  EFI_ACPI_COMMON_HEADER       *CurrentTable;
  UINTN                        TableHandle;
  UINT32                       FvStatus;
  UINTN                        Size;
  EFI_ACPI_TABLE_VERSION       Version;
  QNC_DEVICE_ENABLES           QNCDeviceEnables;

  Instance = 0;
  TableHandle = 0;
  CurrentTable = NULL;
  QNCDeviceEnables.Uint32 = PcdGet32 (PcdDeviceEnables);

  //
  // Initialize the EFI Driver Library
  //

  ASSERT (sizeof (EFI_GLOBAL_NVS_AREA) == 512);

  Status = gBS->AllocatePool (
                   EfiACPIMemoryNVS,
                   sizeof (EFI_GLOBAL_NVS_AREA),
                   (VOID**)&mGlobalNvsArea.Area
                   );
  SetMem (
      mGlobalNvsArea.Area,
      sizeof (EFI_GLOBAL_NVS_AREA),
      0
      );

  //
  // Initialize the data.  Eventually, this will be controlled by setup options.
  //
  mGlobalNvsArea.Area->HpetEnable           =  PcdGetBool (PcdHpetEnable);  
  mGlobalNvsArea.Area->Pm1blkIoBaseAddress  =  PcdGet16(PcdPm1blkIoBaseAddress);
  mGlobalNvsArea.Area->PmbaIoBaseAddress    =  PcdGet16(PcdPmbaIoBaseAddress);   
  mGlobalNvsArea.Area->Gpe0blkIoBaseAddress =  PcdGet16(PcdGpe0blkIoBaseAddress);
  mGlobalNvsArea.Area->GbaIoBaseAddress     =  PcdGet16(PcdGbaIoBaseAddress);                        
  mGlobalNvsArea.Area->SmbaIoBaseAddress    =  PcdGet16(PcdSmbaIoBaseAddress);
  mGlobalNvsArea.Area->SpiDmaIoBaseAddress  =  PcdGet16(PcdSpiDmaIoBaseAddress);
  mGlobalNvsArea.Area->WdtbaIoBaseAddress   =  PcdGet16(PcdWdtbaIoBaseAddress);
  mGlobalNvsArea.Area->HpetBaseAddress      =  (UINT32)PcdGet64(PcdHpetBaseAddress);       
  mGlobalNvsArea.Area->HpetSize             =  (UINT32)PcdGet64(PcdHpetSize);              
  mGlobalNvsArea.Area->PciExpressBaseAddress=  (UINT32)PcdGet64(PcdPciExpressBaseAddress); 
  mGlobalNvsArea.Area->PciExpressSize       =  (UINT32)PcdGet64(PcdPciExpressSize);        
  mGlobalNvsArea.Area->RcbaMmioBaseAddress  =  (UINT32)PcdGet64(PcdRcbaMmioBaseAddress); 
  mGlobalNvsArea.Area->RcbaMmioSize         =  (UINT32)PcdGet64(PcdRcbaMmioSize);        
  mGlobalNvsArea.Area->IoApicBaseAddress    =  (UINT32)PcdGet64(PcdIoApicBaseAddress);     
  mGlobalNvsArea.Area->IoApicSize           =  (UINT32)PcdGet64(PcdIoApicSize);

  //
  // Locate the firmware volume protocol
  //
  Status = LocateSupportProtocol (&gEfiFirmwareVolume2ProtocolGuid, (VOID**)&FwVol, 1);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }
  //
  // Read tables from the storage file.
  //

  while (Status == EFI_SUCCESS) {

    Status = FwVol->ReadSection (
                      FwVol,
                      (EFI_GUID*)PcdGetPtr (PcdAcpiTableStorageFile),
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID**)&CurrentTable,
                      &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR(Status)) {
      //
      // Perform any table specific updates.
      //
      AcpiUpdateTable ((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable, &Version);
        
      //
      // Add the table
      //
      if (Version != EFI_ACPI_TABLE_VERSION_NONE) {
        Status = InstallAcpiTable (
                   CurrentTable,
                   ((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->Length
                   );
        if (EFI_ERROR(Status)) {
          return EFI_ABORTED;
        }
      }

      //
      // Increment the instance
      //
      Instance++;
      CurrentTable = NULL;
    }
  }

  return EFI_SUCCESS;
}
