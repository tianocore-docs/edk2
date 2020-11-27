/** @file

  SMM Platform Hook Library.

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


#include <PiDxe.h>
#include <Library/QNCAccessLib.h>

#include <Library/IntelQNCLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>

#define ACPI_ENABLE                 0xA0
#define ACPI_DISABLE                0xA1

#define R_IOPORT_CMOS_STANDARD_INDEX            0x70
#define R_IOPORT_CMOS_STANDARD_DATA             0x71
#define RTC_ADDRESS_REGISTER_C    12
#define RTC_ADDRESS_REGISTER_D    13

VOID
EnableAcpiCallback (
  VOID
  )
/*++

Routine Description:
  SMI handler to enable ACPI mode
  
  Dispatched on reads from APM port with value 0xA0

  Disables the SW SMI Timer.  
  ACPI events are disabled and ACPI event status is cleared.  
  SCI mode is then enabled.

   Disable SW SMI Timer

   Clear all ACPI event status and disable all ACPI events
   Disable PM sources except power button
   Clear status bits

   Disable GPE0 sources
   Clear status bits

   Disable GPE1 sources
   Clear status bits

   Guarantee day-of-month alarm is invalid (ACPI 1.0 section 4.7.2.4)

   Enable SCI

Arguments:
  DispatchHandle  - EFI Handle
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:
  Nothing

--*/
{
  UINT32      SmiEn;
  UINT16      Pm1Cnt;
  UINT8       Data8;
  UINT32      QncPmBase;
  UINT32      QncGpe0Base;

  DEBUG ((EFI_D_INFO, "EnableAcpiCallback\n"));

  QncPmBase    = PciRead16 (PCI_LIB_ADDRESS(PCI_BUS_NUMBER_QNC, PCI_DEVICE_NUMBER_QNC_LPC, 0, R_QNC_LPC_PM1BLK)) & B_QNC_LPC_PM1BLK_MASK;
  QncGpe0Base  = PciRead16 (PCI_LIB_ADDRESS(PCI_BUS_NUMBER_QNC, PCI_DEVICE_NUMBER_QNC_LPC, 0, R_QNC_LPC_GPE0BLK)) & B_QNC_LPC_GPE0BLK_MASK;

  SmiEn = IoRead32 (QncGpe0Base + R_QNC_GPE0BLK_SMIE);

  //
  // Disable SW SMI Timer
  //
  SmiEn &= ~(B_QNC_GPE0BLK_SMIE_SWT);
  IoWrite32 (QncGpe0Base + R_QNC_GPE0BLK_SMIE, SmiEn);

  //
  // Guarantee day-of-month alarm is invalid (ACPI 1.0 section 4.7.2.4)
  //
  Data8 = RTC_ADDRESS_REGISTER_D;
  IoWrite8 (R_IOPORT_CMOS_STANDARD_INDEX, Data8);
  Data8 = 0x0;
  IoWrite8 (R_IOPORT_CMOS_STANDARD_DATA, Data8);

  //
  // Enable SCI
  //
  Pm1Cnt = IoRead16 (QncPmBase + R_QNC_PM1BLK_PM1C);
  Pm1Cnt |= B_QNC_PM1BLK_PM1C_SCIEN;
  IoWrite16 (QncPmBase + R_QNC_PM1BLK_PM1C, Pm1Cnt);

  //
  // Do platform specific stuff for ACPI enable SMI
  //
  return ;
}


VOID
DisableAcpiCallback (
  VOID
  )
/*++

Routine Description:
  SMI handler to disable ACPI mode

  Dispatched on reads from APM port with value 0xA1

  ACPI events are disabled and ACPI event status is cleared.  
  SCI mode is then disabled.
   Clear all ACPI event status and disable all ACPI events
   Disable PM sources except power button
   Clear status bits
   Disable GPE0 sources
   Clear status bits
   Disable GPE1 sources
   Clear status bits
   Disable SCI

Arguments:
  DispatchHandle  - EFI Handle
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:
  Nothing

--*/
{
  UINT16      Pm1Cnt;
  UINT32      QncPmBase;

  DEBUG ((EFI_D_INFO, "DisableAcpiCallback\n"));

  QncPmBase    = PciRead16 (PCI_LIB_ADDRESS(PCI_BUS_NUMBER_QNC, PCI_DEVICE_NUMBER_QNC_LPC, 0, R_QNC_LPC_PM1BLK)) & B_QNC_LPC_PM1BLK_MASK;

  Pm1Cnt = IoRead16 (QncPmBase + R_QNC_PM1BLK_PM1C);

  //
  // Disable SCI
  //
  Pm1Cnt &= ~B_QNC_PM1BLK_PM1C_SCIEN;

  IoWrite16 (QncPmBase + R_QNC_PM1BLK_PM1C, Pm1Cnt); 

  return ;
}

/**
  This function is to enable smi sources
  
**/
VOID
SmmPlatformEnableSmiSources (
	VOID
	)
{
  UINT16        PM1BLK_Base;
  UINT16        GPE0BLK_Base;
  UINT32        NewValue;

  //
  // Get PM1BLK_Base & GPE0BLK_Base
  //
  PM1BLK_Base = PcdGet16 (PcdPm1blkIoBaseAddress);
  GPE0BLK_Base = (UINT16)(LpcPciCfg32 (R_QNC_LPC_GPE0BLK) & 0xFFFF);

  //
  // Enable APM SMI
  //
  IoOr32 ((GPE0BLK_Base + R_QNC_GPE0BLK_SMIE), B_QNC_GPE0BLK_SMIE_APM);

  //
  // Enable SMI globally
  //
  NewValue = QNCPortRead (QUARK_NC_HOST_BRIDGE_SB_PORT_ID, QNC_MSG_FSBIC_REG_HMISC);
  NewValue |= SMI_EN;
  QNCPortWrite (QUARK_NC_HOST_BRIDGE_SB_PORT_ID, QNC_MSG_FSBIC_REG_HMISC, NewValue);
}

/**
  This function is to enable smi sources
  
**/
VOID
SmmPlatformSmiCallBack (
	VOID
	)
{
  UINT8                        SmiStatus;
  UINT8                        Data; 

  DEBUG ((EFI_D_INFO, "SmmPlatformSmiCallBack\n"));

  SmiStatus = IoRead8 ((UINT16)(LpcPciCfg32 (R_QNC_LPC_GPE0BLK) & 0xFFFF) + R_QNC_GPE0BLK_SMIS);
  if (((SmiStatus & B_QNC_GPE0BLK_SMIS_APM) != 0) &&
       (IoRead8 (PcdGet16 (PcdSmmActivationPort)) == PcdGet8 (PcdSmmActivationData))) {
    Data = IoRead8 (PcdGet16 (PcdSmmDataPort));
    if (Data == ACPI_ENABLE) {
      EnableAcpiCallback ();
    }
  }
}

EFI_STATUS
GetSmramRegion (
  IN OUT UINT64  *SmramBase,
  IN OUT UINT64  *SmramSize
  )
{
  *SmramBase = FixedPcdGet32(PcdMemorySize) - FixedPcdGet32(PcdTSegSize);
  *SmramSize = FixedPcdGet32(PcdTSegSize);
  return EFI_SUCCESS;
}

BOOLEAN
OpenSmramRegion (
  VOID
  )
{
  return QNCOpenSmramRegion ();
}

BOOLEAN
CloseSmramRegion (
  VOID
  )
{
  return QNCCloseSmramRegion ();
}

BOOLEAN
LockSmramRegion (
  VOID
  )
{
  QNCLockSmramRegion ();
  return TRUE;
}
