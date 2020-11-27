/** @file
SMM MP service inplementation

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

#include "PiSmmCpuDxeSmm.h"

//
// Slots for all MTRR( FIXED MTRR + VARIABLE MTRR + MTRR_LIB_IA32_MTRR_DEF_TYPE)
//
UINT64                                      gSmiMtrrs[MTRR_NUMBER_OF_FIXED_MTRR + 2 * MTRR_NUMBER_OF_VARIABLE_MTRR + 1];
UINT64                                      gPhyMask;

/**
  Replace OS MTRR's with SMI MTRR's.

  @param    CpuIndex             Processor Index

**/
VOID
ReplaceOSMtrrs (
  IN      UINTN                     CpuIndex
  )
{
  PROCESSOR_SMM_DESCRIPTOR       *Psd;
  UINT64                         *SmiMtrrs;
  MTRR_SETTINGS                  *BiosMtrr;

  Psd = (PROCESSOR_SMM_DESCRIPTOR*)(mCpuHotPlugData.SmBase[CpuIndex] + SMM_PSD_OFFSET);
  SmiMtrrs = (UINT64*)(UINTN)Psd->MtrrBaseMaskPtr;

  DisableSmrr ();

  //
  // Replace all MTRRs registers
  //
  BiosMtrr  = (MTRR_SETTINGS*)SmiMtrrs;
  MtrrSetAllMtrrs(BiosMtrr);
}

/**
  SMI handler for BSP.

  @param     CpuIndex         BSP processor Index
  @param     SyncMode         SMM MP sync mode

**/
VOID
BSPHandler (
  IN      UINTN                     CpuIndex
  )
{
  MTRR_SETTINGS                     Mtrrs;

  DEBUG ((EFI_D_INFO, "BSPHandler\n"));

  //
  // If Traditional Sync Mode or need to configure MTRRs: gather all available APs.
  // To-Do: make NeedConfigureMtrrs a PCD?
  //
  if (NeedConfigureMtrrs()) {
    MtrrGetAllMtrrs(&Mtrrs);
    
    ReplaceOSMtrrs (CpuIndex);
  }

  //
  // Invoke SMM Foundation EntryPoint with the processor information context.
  //
  SmmPlatformSmiCallBack ();

  //
  // Set the EOS bit before SMI resume.
  //
  // BUGBUG: The following is a chipset specific action from a CPU module.
  //
  ClearSmi();

  if (NeedConfigureMtrrs()) {
    //
    // Restore OS MTRRs
    //
    ReenableSmrr ();
    MtrrSetAllMtrrs(&Mtrrs);
  }
}

/**
  Create 4G PageTable in SMRAM.

  @param          ExtraPages       Additional page numbers besides for 4G memory
  @return         PageTable Address

**/
UINT32
Gen4GPageTable (
  IN      UINTN                     ExtraPages
  )
{
  VOID    *PageTable;
  UINTN   Index;
  UINT64  *Pte;

  //
  // Allocate the page table
  //
  PageTable = MyAllocatePages (ExtraPages + 5);
  ASSERT (PageTable != NULL);

  PageTable = (VOID *)((UINTN)PageTable + EFI_PAGES_TO_SIZE (ExtraPages));
  Pte = (UINT64*)PageTable;

  //
  // Zero out all page table entries first
  //
  ZeroMem (Pte, EFI_PAGES_TO_SIZE (1));

  //
  // Set Page Directory Pointers
  //
  for (Index = 0; Index < 4; Index++) {
    Pte[Index] = (UINTN)PageTable + EFI_PAGE_SIZE * (Index + 1) + IA32_PG_P;
  }
  Pte += EFI_PAGE_SIZE / sizeof (*Pte);

  //
  // Fill in Page Directory Entries
  //
  for (Index = 0; Index < EFI_PAGE_SIZE * 4 / sizeof (*Pte); Index++) {
    Pte[Index] = (Index << 21) + IA32_PG_PS + IA32_PG_RW + IA32_PG_P;
  }

  return (UINT32)(UINTN)PageTable;
}

/**
  C function for SMI entry, each processor comes here upon SMI trigger.

  @param    CpuIndex              Cpu Index

**/
VOID
EFIAPI
SmiRendezvous (
  IN      UINTN                     CpuIndex
  )
{
  DEBUG ((EFI_D_INFO, "SmiRendezvous\n"));

  //
  // Enable exception table by load IDTR
  //
  AsmWriteIdtr (&gcSmiIdtr);

  //
  // Enable XMM instructions & exceptions
  //
  AsmWriteCr4 (AsmReadCr4 () | 0x600);
  
  //
  // BSP Handler is always called with a ValidSmi == TRUE
  //
  BSPHandler (CpuIndex);

  PlatformSmmExitProcessing ();
}

/**
  Initialize global data for MP synchronization.

  @param ImageHandle  File Image Handle.
  @param Stacks       Base address of SMI stack buffer for all processors.
  @param StackSize    Stack size for each processor in SMM.

**/
VOID
InitializeMpServiceData (
  IN EFI_HANDLE  ImageHandle,
  IN VOID        *Stacks,
  IN UINTN       StackSize
 )
{
  MTRR_SETTINGS             *Mtrr;
  PROCESSOR_SMM_DESCRIPTOR  *Psd;
  UINT32                    RegEax;

  //
  // Initialize physical address mask
  // NOTE: Physical memory above virtual address limit is not supported !!!
  //
  AsmCpuid (0x80000008, (UINT32*)&RegEax, NULL, NULL, NULL);
  gPhyMask = LShiftU64 (1, (UINT8)RegEax) - 1;
  gPhyMask &= (1ull << 48) - EFI_PAGE_SIZE;

  InitializeSmmMtrrManager ();
  //
  // Create page tables
  //
  gSmiCr3 = SmmInitPageTable ();

  //
  // Initialize SMM startup code & PROCESSOR_SMM_DESCRIPTOR structures
  //
  gSmiStack = (UINTN)Stacks + StackSize - sizeof (UINTN);

  //
  // The Smi Handler of CPU[i] and PSD of CPU[i+x] are in the same SMM_CPU_INTERVAL,
  // and they cannot overlap.
  //
  ASSERT (gcSmiHandlerSize + 0x10000 - SMM_PSD_OFFSET < SMM_CPU_INTERVAL);
  ASSERT (SMM_HANDLER_OFFSET % SMM_CPU_INTERVAL == 0);

    *(UINTN*)gSmiStack = 0;
    gSmbase = (UINT32)mCpuHotPlugData.SmBase[0];
    CopyMem (
      (VOID*)(UINTN)(mCpuHotPlugData.SmBase[0] + SMM_HANDLER_OFFSET),
      (VOID*)gcSmiHandlerTemplate,
      gcSmiHandlerSize
      );

    Psd = (PROCESSOR_SMM_DESCRIPTOR*)(VOID*)(UINTN)(mCpuHotPlugData.SmBase[0] + SMM_PSD_OFFSET);
    CopyMem (Psd, &gcPsd, sizeof (gcPsd));

    gSmiStack += StackSize;

  //
  // Record current MTRR settings
  //
  ZeroMem(gSmiMtrrs, sizeof (gSmiMtrrs));
  Mtrr =  (MTRR_SETTINGS*)gSmiMtrrs;
  MtrrGetAllMtrrs (Mtrr);

}

/**

  Register the SMM Foundation entry point.

  @param          This              Pointer to EFI_SMM_CONFIGURATION_PROTOCOL instance
  @param          SmmEntryPoint     SMM Foundation EntryPoint

  @retval         EFI_SUCCESS       Successfully to register SMM foundation entry point

**/
EFI_STATUS
EFIAPI
RegisterSmmEntry (
  IN CONST EFI_SMM_CONFIGURATION_PROTOCOL  *This,
  IN EFI_SMM_ENTRY_POINT                   SmmEntryPoint
  )
{
  ASSERT (FALSE);
  return EFI_SUCCESS;
}
