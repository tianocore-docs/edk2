/** @file
  This PEIM initialize platform for MRC, following action is performed,
    1. Initizluize GMCH
    2. Detect boot mode
    3. Detect video adapter to determine whether we need pre allocated memory 
    4. Calls MRC to initialize memory and install a PPI notify to do post memory initialization.
  This file contains the main entrypoint of the PEIM.
  
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

**/


#include "CommonHeader.h"
#include "PlatformEarlyInit.h"

#include "Platform.h"
#include <Ppi/CltMemoryInit.h>

//
// Function prototypes to routines implemented in other source modules
// within this component.
//

EFI_STATUS
EFIAPI
SocUnitEarlyInitialisation (
  VOID
  );

EFI_STATUS
EFIAPI
SocUnitReleasePcieControllerPreWaitPllLock (
  VOID
  );

EFI_STATUS
EFIAPI
SocUnitReleasePcieControllerPostPllLock (
  VOID
  );

EFI_STATUS
EFIAPI
PlatformErratasPostMrc (
  VOID
  );

VOID
PostInstallMemory (
  IN MRC_PARAMS                           *MrcData
  );

//
// The global indicator, the FvFileLoader callback will modify it to TRUE after loading PEIM into memory
//
BOOLEAN ImageInMemory = FALSE;

BOARD_LEGACY_GPIO_CONFIG      mBoardLegacyGpioConfigTable[]  = { PLATFORM_LEGACY_GPIO_TABLE_DEFINITION };
UINTN                         mBoardLegacyGpioConfigTableLen = (sizeof(mBoardLegacyGpioConfigTable) / sizeof(BOARD_LEGACY_GPIO_CONFIG));

/** Early initialisation of the PCIe controller

  @retval   EFI_SUCCESS               Operation success.

**/
EFI_STATUS
EFIAPI
PcieControllerEarlyInit (
  VOID
  )
{
  //
  // Release and wait for PCI controller to come out of reset.
  //
  SocUnitReleasePcieControllerPreWaitPllLock ();
  MicroSecondDelay (PCIEXP_DELAY_US_WAIT_PLL_LOCK);
  SocUnitReleasePcieControllerPostPllLock ();

  //
  // Early PCIe initialisation
  //
  SocUnitEarlyInitialisation ();

  return EFI_SUCCESS;
}

/**
  This is the entrypoint of PEIM
  
  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS if it completed successfully.  
**/
EFI_STATUS
EFIAPI
DxeInitPlatform (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  MRC_PARAMS MrcData;

  //
  // Do any early platform specific initialisation
  //
  EarlyPlatformInit ();

  //
  // Initialise System Phys
  //

  // Program USB Phy
  InitializeUSBPhy();
  
  QNCClearSmiAndWake ();    

  DEBUG ((EFI_D_INFO, "PostInstallMemory\n"));
  ZeroMem (&MrcData, sizeof(MrcData));
  MrcData.mem_size = FixedPcdGet32(PcdMemorySize);
  MrcData.ecc_enables = 0;
  PostInstallMemory (&MrcData);

  //
  // Release PCIe Controller from Reset.
  //
  DEBUG ((EFI_D_INFO, "Early PCIe controller initialisation\n"));
  PcieControllerEarlyInit ();

  DEBUG ((EFI_D_INFO, "Platform Erratas After MRC\n"));
  PlatformErratasPostMrc ();

  return EFI_SUCCESS;
}

/**
  This function will initialize USB Phy registers associated with QuarkSouthCluster.

  @param  VOID                  No Argument

  @retval EFI_SUCCESS           All registers have been initialized
**/
VOID
EFIAPI
InitializeUSBPhy (
    VOID
   )
{
    UINT32 RegData32;

    /** In order to configure the PHY to use clk120 (ickusbcoreclk) as PLL reference clock
     *  and Port2 as a USB device port, the following sequence must be followed
     *
     **/

    // Sideband register write to USB AFE (Phy)
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_GLOBAL_PORT);
    RegData32 &= ~(BIT1);
    //
    // Sighting #4930631 PDNRESCFG [8:7] of USB2_GLOBAL_PORT = 11b.
    // For port 0 & 1 as host and port 2 as device.
    //
    RegData32 |= (BIT8 | BIT7);
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_GLOBAL_PORT, RegData32);

    //
    // Sighting #4930653 Required BIOS change on Disconnect vref to change to 600mV.
    //
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_COMPBG);
    RegData32 &= ~(BIT10 | BIT9 | BIT8 | BIT7);
    RegData32 |= (BIT10 | BIT7);
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_COMPBG, RegData32);

    // Sideband register write to USB AFE (Phy)
    // (pllbypass) to bypass/Disable PLL before switch
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL2);
    RegData32 |= BIT29;
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL2, RegData32);

    // Sideband register write to USB AFE (Phy)
    // (coreclksel) to select 120MHz (ickusbcoreclk) clk source.
    // (Default 0 to select 96MHz (ickusbclk96_npad/ppad))
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL1);
    RegData32 |= BIT1;
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL1, RegData32);

    // Sideband register write to USB AFE (Phy)
    // (divide by 8) to achieve internal 480MHz clock
    // for 120MHz input refclk.  (Default: 4'b1000 (divide by 10) for 96MHz)
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL1);
    RegData32 &= ~(BIT5 | BIT4 | BIT3);
    RegData32 |= BIT6;
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL1, RegData32);

    // Sideband register write to USB AFE (Phy)
    // Clear (pllbypass)
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL2);
    RegData32 &= ~BIT29;
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL2, RegData32);

    // Sideband register write to USB AFE (Phy)
    // Set (startlock) to force the PLL FSM to restart the lock
    // sequence due to input clock/freq switch.
    RegData32 = QNCAltPortRead (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL2);
    RegData32 |= BIT24;
    QNCAltPortWrite (QUARK_SC_USB_AFE_SB_PORT_ID, USB2_PLL2, RegData32);

    // At this point the PLL FSM and COMP FSM will complete

}

/**
  This function provides early platform Thermal sensor initialisation.
**/
VOID
EFIAPI
EarlyPlatformThermalSensorInit (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "Early Platform Thermal Sensor Init\n"));

  //
  // Set Thermal sensor mode.
  //
  QNCThermalSensorSetRatiometricMode ();

  //
  // Enable RMU Thermal sensor with a Catastrophic Trip point.
  //
  QNCThermalSensorEnableWithCatastrophicTrip (PLATFORM_CATASTROPHIC_TRIP_CELSIUS);

  //
  // Lock all RMU Thermal sensor control & trip point registers.
  //
  QNCThermalSensorLockAllRegisters ();
}

/**
  Check if system reset due to error condition.

  @param  ClearErrorBits  If TRUE clear error flags and value bits.

  @retval TRUE  if system reset due to error condition.
  @retval FALSE if NO reset error conditions.
**/
BOOLEAN
CheckForResetDueToErrors (
  IN BOOLEAN                              ClearErrorBits
  )
{
  UINT32                            RegValue;
  BOOLEAN                           ResetDueToError;

  ResetDueToError = FALSE;

  //
  // Check if RMU reset system due to access violations.
  // RMU updates a SOC Unit register before reseting the system.
  //
  RegValue = QNCAltPortRead (QUARK_SCSS_SOC_UNIT_SB_PORT_ID, QUARK_SCSS_SOC_UNIT_CFG_STICKY_RW);
  if ((RegValue & B_CFG_STICKY_RW_VIOLATION) != 0) {
    ResetDueToError = TRUE;

    DEBUG (
      (EFI_D_ERROR,
      "\nReset due to access violation: %s %s %s %s\n",
      ((RegValue & B_CFG_STICKY_RW_IMR_VIOLATION) != 0) ? L"'IMR'" : L".",
      ((RegValue & B_CFG_STICKY_RW_DECC_VIOLATION) != 0) ? L"'DECC'" : L".",
      ((RegValue & B_CFG_STICKY_RW_SMM_VIOLATION) != 0) ? L"'SMM'" : L".",
      ((RegValue & B_CFG_STICKY_RW_HMB_VIOLATION) != 0) ? L"'HMB'" : L"."
      ));

    //
    // Clear error bits.
    //
    if (ClearErrorBits) {
      RegValue &= ~(B_CFG_STICKY_RW_VIOLATION);
      QNCAltPortWrite (QUARK_SCSS_SOC_UNIT_SB_PORT_ID, QUARK_SCSS_SOC_UNIT_CFG_STICKY_RW, RegValue);
    }
  }

  return ResetDueToError;
}

/**
  This function provides early platform initialisation.
**/
VOID
EFIAPI
EarlyPlatformInit (
  VOID
  )
{

  //
  // Check if system reset due to error condition.
  //
  if (CheckForResetDueToErrors (TRUE)) {
  }

  //
  // Early Gpio Init.
  //
  EarlyPlatformGpioInit ();

  //
  // Early Thermal Sensor Init.
  //
  EarlyPlatformThermalSensorInit ();

}

/**
  This function provides early platform GPIO initialisation.
**/
VOID
EFIAPI
EarlyPlatformGpioInit (
  VOID
  )
{
  BOARD_LEGACY_GPIO_CONFIG          *LegacyGpioConfig;
  UINT32                            NewValue;
  UINT32                            GpioBaseAddress;

  //
  // Get platform type from platform info hob.
  //
  ASSERT (ClantonHill < mBoardLegacyGpioConfigTableLen);

  LegacyGpioConfig = &mBoardLegacyGpioConfigTable[ClantonHill];
  DEBUG ((EFI_D_INFO, "EarlyPlatformGpioInit for PlatType=0x%02x\n", ClantonHill));

  GpioBaseAddress = (UINT32)PcdGet16 (PcdGbaIoBaseAddress);

  NewValue     = 0x0;
  //
  // Program QNC GPIO Registers.
  //
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGEN_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGEN_CORE_WELL, NewValue );
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGIO_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellIoSelect;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGIO_CORE_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGLVL_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellLvlForInputOrOutput;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGLVL_CORE_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGTPE_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellTriggerPositiveEdge;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGTPE_CORE_WELL, NewValue );
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGTNE_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellTriggerNegativeEdge;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGTNE_CORE_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGGPE_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellGPEEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGGPE_CORE_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGSMI_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellSMIEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGSMI_CORE_WELL, NewValue );
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CGTS_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellTriggerStatus;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CGTS_CORE_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_CNMIEN_CORE_WELL) & 0xFFFFFFFC) | LegacyGpioConfig->CoreWellNMIEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_CNMIEN_CORE_WELL, NewValue);

  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGEN_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGEN_RESUME_WELL, NewValue );
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGIO_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellIoSelect;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGIO_RESUME_WELL, NewValue) ;
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGLVL_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellLvlForInputOrOutput;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGLVL_RESUME_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGTPE_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellTriggerPositiveEdge;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGTPE_RESUME_WELL, NewValue );
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGTNE_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellTriggerNegativeEdge;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGTNE_RESUME_WELL, NewValue) ;
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGGPE_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellGPEEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGGPE_RESUME_WELL, NewValue);
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGSMI_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellSMIEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGSMI_RESUME_WELL, NewValue );
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RGTS_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellTriggerStatus;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RGTS_RESUME_WELL, NewValue) ;
  NewValue = (IoRead32 (GpioBaseAddress + R_QNC_GPIO_RNMIEN_RESUME_WELL) & 0xFFFFFFC0) | LegacyGpioConfig->ResumeWellNMIEnable;
  IoWrite32 (GpioBaseAddress + R_QNC_GPIO_RNMIEN_RESUME_WELL, NewValue);
}
