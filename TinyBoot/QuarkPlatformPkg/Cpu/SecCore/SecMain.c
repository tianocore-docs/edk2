/** @file

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


#include "SecMain.h"

//
// These are IDT entries pointing to 10:FFFFFFE4h.
//
UINT64  mIdtEntryTemplate = 0xffff8e000010ffe4ULL;

/**
  Caller provided function to be invoked at the end of InitializeDebugAgent().

  Entry point to the C language phase of SEC. After the SEC assembly
  code has initialized some temporary memory and set up the stack,
  the control is transferred to this function.

  @param[in] Context    The first input parameter of InitializeDebugAgent().

**/
VOID
EFIAPI
SecStartupPhase2(
  IN VOID                     *Context
  );


/**

  Entry point to the C language phase of SEC. After the SEC assembly
  code has initialized some temporary memory and set up the stack,
  the control is transferred to this function.


  @param SizeOfRam           Size of the temporary memory available for use.
  @param TempRamBase         Base address of tempory ram
  @param BootFirmwareVolume  Base address of the Boot Firmware Volume.
**/
VOID
EFIAPI
SecStartup (
  IN UINT32                   SizeOfRam,
  IN UINT32                   TempRamBase,
  IN VOID                     *BootFirmwareVolume
  )
{
  HAND_OFF_INFO               Handoff;
  IA32_DESCRIPTOR             IdtDescriptor;
  SEC_IDT_TABLE               IdtTableInStack;
  UINT32                      Index;

  DEBUG_WRITE_STRING ("SecStartup - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)SecStartup);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("SizeOfRam - ");
  DEBUG_WRITE_UINT32 (SizeOfRam);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("TempRamBase - ");
  DEBUG_WRITE_UINT32 (TempRamBase);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("BootFirmwareVolume - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)BootFirmwareVolume);
  DEBUG_WRITE_STRING ("\n");

  //
  // Process all libraries constructor function linked to SecCore.
  //

  DEBUG_WRITE_STRING ("InitializeFloatingPointUnits\n");

  //
  // Initialize floating point operating environment
  // to be compliant with UEFI spec.
  //
  InitializeFloatingPointUnits ();


  // |-------------------|---->
  // |Idt Table          |
  // |-------------------|
  // |PeiService Pointer |    PeiStackSize
  // |-------------------|
  // |                   |
  // |      Stack        |
  // |-------------------|---->
  // |                   |
  // |                   |
  // |      Heap         |    PeiTemporayRamSize
  // |                   |
  // |                   |
  // |-------------------|---->  TempRamBase

  IdtTableInStack.PeiService = 0;
  for (Index = 0; Index < SEC_IDT_ENTRY_COUNT; Index ++) {
    CopyMem ((VOID*)&IdtTableInStack.IdtTable[Index], (VOID*)&mIdtEntryTemplate, sizeof (UINT64));
  }

  IdtDescriptor.Base  = (UINTN) &IdtTableInStack.IdtTable;
  IdtDescriptor.Limit = (UINT16)(sizeof (IdtTableInStack.IdtTable) - 1);

  AsmWriteIdtr (&IdtDescriptor);

  //
  // Update the base address and length of Pei temporary memory
  //
  Handoff.BootFirmwareVolumeBase = BootFirmwareVolume;

  DEBUG_WRITE_STRING ("InitializeDebugAgent\n");

  //
  // Initialize Debug Agent to support source level debug in SEC/PEI phases before memory ready.
  //
  InitializeDebugAgent (DEBUG_AGENT_INIT_PREMEM_SEC, &Handoff, SecStartupPhase2);
}

/**
  Caller provided function to be invoked at the end of InitializeDebugAgent().

  Entry point to the C language phase of SEC. After the SEC assembly
  code has initialized some temporary memory and set up the stack,
  the control is transferred to this function.

  @param[in] Context    The first input parameter of InitializeDebugAgent().

**/
VOID
EFIAPI
SecStartupPhase2(
  IN VOID                     *Context
  )
{
  HAND_OFF_INFO               *Handoff;

  DEBUG_WRITE_STRING ("SecStartupPhase2\n");

  Handoff = (HAND_OFF_INFO *) Context;

  //
  // Perform platform specific initialization before entering PeiCore.
  //
  DEBUG_WRITE_STRING ("Call SecPlatformMain\n");
  Handoff->MemoryTop = SecPlatformMain (Context);
  
  DEBUG_WRITE_STRING ("DecompressDxeFv\n");

  //
  // Find Pei Core entry point. It will report SEC and Pei Core debug information if remote debug
  // is enabled.
  //
  DEBUG_WRITE_STRING ("FindAndReportEntryPoints\n");
  DEBUG_WRITE_STRING ("BootFirmwareVolumeBase - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)Handoff->BootFirmwareVolumeBase);
  DEBUG_WRITE_STRING ("\n");

  FindAndReportEntryPoints (
    Handoff->BootFirmwareVolumeBase,
    &Handoff->DxeCoreEntryPoint,
    &Handoff->DxeCoreImageBase,
    &Handoff->DxeCoreLength
    );
  if (Handoff->DxeCoreEntryPoint == NULL) {
    DEBUG_WRITE_STRING ("DxeCoreEntryPoint == NULL\n");
    CpuDeadLoop ();
  }

  //
  // Transfer the control to the PEI core
  //
  DEBUG_WRITE_STRING ("Transfer to DxeCoreEntryPoint - ");
  DEBUG_WRITE_UINT32 ((UINT32)(UINTN)Handoff->DxeCoreEntryPoint);
  DEBUG_WRITE_STRING ("\n");

  SecIplEntry (Handoff);

  //
  // Should not come here.
  //
  return ;
}

VOID
EFIAPI
DummyEntrypoint (
  IN CONST EFI_SEC_PEI_HAND_OFF        *SecCoreDataPtr,
  IN CONST EFI_PEI_PPI_DESCRIPTOR      *PpiList,
  IN VOID                              *Data
  )
{
}