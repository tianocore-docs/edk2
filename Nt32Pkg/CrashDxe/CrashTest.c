/** @file

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/Cpu.h>

VOID
EFIAPI
MyNt32ExceptionHandler (
  IN CONST  EFI_EXCEPTION_TYPE  InterruptType,
  IN CONST  EFI_SYSTEM_CONTEXT  SystemContext
  )
{
  DEBUG ((DEBUG_INFO, "NT32 CPU Debug - %x\n", InterruptType));
}



EFI_STATUS
EFIAPI
InitializeCrashTest (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  UINT8                  *Ptr;
  EFI_STATUS             Status;
  EFI_CPU_ARCH_PROTOCOL  *Cpu;
  EFI_EXCEPTION_TYPE     Index;

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, &Cpu);
  ASSERT_EFI_ERROR(Status);

  for (Index = 0; Index < 0x20; Index++) {
    Cpu->RegisterInterruptHandler (Cpu, Index, MyNt32ExceptionHandler);
  }

  Ptr = NULL;
  Print (L"Hello world! - %x\n", *(Ptr + sizeof(UINTN)));

  return EFI_SUCCESS;
}
