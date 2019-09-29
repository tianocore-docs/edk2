/** @file
  The header file for Tcg2 ACPI driver.

Copyright (c) 2015 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __TCG2_ACPI_H__
#define __TCG2_ACPI_H__

#include <PiDxe.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Tpm2Acpi.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/Tcg2Protocol.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/TpmMeasurementLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tcg2PhysicalPresenceLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/Tpm2DeviceLib.h>

#include <IndustryStandard/TpmPtp.h>

#endif  // __TCG_SMM_H__
