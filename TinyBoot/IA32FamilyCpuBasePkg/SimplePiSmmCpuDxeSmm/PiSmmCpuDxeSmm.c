/** @file
  Agent Module to load other modules to deploy SMM Entry Vector for X86 CPU.

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

typedef struct {
  UINT32   SmrrBase;
  UINT32   SmrrSize;
  UINT32   UsableSmramBase;
  UINT32   UsableSmramSize;
} TINY_SMM_CORE_CONTEXT;

TINY_SMM_CORE_CONTEXT      mTinySmmCoreContext;

VOID *
EFIAPI
MyAllocatePages (
  IN UINTN  Pages
  )
{
  if (EFI_PAGES_TO_SIZE(Pages) >= mTinySmmCoreContext.UsableSmramSize) {
    ASSERT (FALSE);
  }
  mTinySmmCoreContext.UsableSmramSize -= EFI_PAGES_TO_SIZE(Pages);

  return (VOID *)(UINTN)(mTinySmmCoreContext.UsableSmramBase + mTinySmmCoreContext.UsableSmramSize);
}

//
// SMM CPU Private Data structure that contains SMM Configuration Protocol 
// along its supporting fields.
//
SMM_CPU_PRIVATE_DATA  mSmmCpuPrivateData = {
  SMM_CPU_PRIVATE_DATA_SIGNATURE,               // Signature
  NULL,                                         // SmmCpuHandle

  { 0 },                                        // SmmReservedSmramRegion
  {
    mSmmCpuPrivateData.SmmReservedSmramRegion,  // SmmConfiguration.SmramReservedRegions
    RegisterSmmEntry                            // SmmConfiguration.RegisterSmmEntry
  },
};

CPU_HOT_PLUG_DATA mCpuHotPlugData = {
  { 0 },                                        // SmBases array
  { 0 },                                        // APIC ID array
  0,                                            // SmrrBase
  0                                             // SmrrSize
};

//
// Global pointer used to access mSmmCpuPrivateData from outside and inside SMM
//
SMM_CPU_PRIVATE_DATA  *gSmmCpuPrivate = &mSmmCpuPrivateData;

EFI_CPU_INTERRUPT_HANDLER   mExternalVectorTable[EXCEPTION_VECTOR_NUMBER];

//
// SMM stack information
//
UINTN mSmmStackArrayBase;
UINTN mSmmStackArrayEnd;
UINTN mSmmStackSize;

///
/// Macro used to simplfy the lookup table entries of type CPU_SMM_SAVE_STATE_LOOKUP_ENTRY
///
#define SMM_CPU_OFFSET(Field) OFFSET_OF (EFI_SMM_CPU_STATE, Field)

///
/// Macro used to simplfy the lookup table entries of type CPU_SMM_SAVE_STATE_REGISTER_RANGE
///
#define SMM_REGISTER_RANGE(Start, End) { Start, End, End - Start + 1 }

/**
  C function for SMI handler. To change all processor's SMMBase Register.

**/
VOID
EFIAPI
SmmInitHandler (
  VOID
  )
{
  UINT32                            ApicId;
  IA32_DESCRIPTOR                   X64Idtr;
  IA32_IDT_GATE_DESCRIPTOR          IdtEntryTable[EXCEPTION_VECTOR_NUMBER];

  //
  // Set up X64 IDT table
  //
  ZeroMem (IdtEntryTable, sizeof (IA32_IDT_GATE_DESCRIPTOR) * EXCEPTION_VECTOR_NUMBER);
  X64Idtr.Base = (UINTN) IdtEntryTable;
  X64Idtr.Limit = (UINT16) (sizeof (IA32_IDT_GATE_DESCRIPTOR) * EXCEPTION_VECTOR_NUMBER - 1);
  AsmWriteIdtr ((IA32_DESCRIPTOR *) &X64Idtr);

  ApicId = GetApicId ();

  SmmInitiFeatures (0, mCpuHotPlugData.SmrrBase, mCpuHotPlugData.SmrrSize, (UINT32)mCpuHotPlugData.SmBase[0], TRUE);

  PlatformSmmExitProcessing ();
  return;
}

/**
  Relocate SmmBases for each processor.

  Execute on first boot and all S3 resumes

**/
VOID
EFIAPI
SmmRelocateBases (
  VOID
  )
{
  UINT8                             BakBuf[BACK_BUF_SIZE];
  EFI_SMM_CPU_STATE                 BakBuf2;
  EFI_SMM_CPU_STATE                 *CpuStatePtr;
  UINT8                             *U8Ptr;
  UINT32                            ApicId;

  //
  // Make sure the reserved size is large enough for procedure SmmInitTemplate.
  //
  ASSERT (sizeof (BakBuf) >= gcSmmInitSize);

  //
  // Patch ASM code template with current CR0, CR3, and CR4 values
  //
  gSmmCr0 = (UINT32)AsmReadCr0 ();
  gSmmCr3 = (UINT32)AsmReadCr3 ();
  gSmmCr4 = (UINT32)AsmReadCr4 ();

  U8Ptr = (UINT8*)(UINTN)(SMM_DEFAULT_SMBASE + SMM_HANDLER_OFFSET);
  CpuStatePtr = (EFI_SMM_CPU_STATE *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_CPU_STATE_OFFSET);

  //
  // Backup original contents @ 0x38000
  //
  CopyMem (BakBuf, U8Ptr, sizeof (BakBuf));
  CopyMem (&BakBuf2, CpuStatePtr, sizeof (BakBuf2));

  //
  // Load image for relocation
  //
  CopyMem (U8Ptr, gcSmmInitTemplate, gcSmmInitSize);

  //
  // Retrieve the local APIC ID of current processor
  //
  ApicId = GetApicId ();

  //
  // Relocate BSP's SM base
  //
  SendSmiIpi (ApicId);

  //
  // Restore contents @ 0x38000
  //
  CopyMem (CpuStatePtr, &BakBuf2, sizeof (BakBuf2));
  CopyMem (U8Ptr, BakBuf, sizeof (BakBuf));
}

/**
  Smm Ready To Lock event notification handler.

  The CPU S3 data is copied to SMRAM for security. SMM Code Access Check feature is enabled if available.
  
  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
 **/
EFI_STATUS
EFIAPI
SmmReadyToLockEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  //
  // Configure SMM Code Access Check feature if available.
  //
  if (PcdGetBool (PcdCpuSmmCodeAccessCheckEnable) ||
      FeaturePcdGet (PcdCpuSmmFeatureControlMsrLock)) {
    //
    // Save the PcdCpuSmmCodeAccessCheckEnable value into a local variable for s3 boot path.
    //
    mSmmCodeAccessCheckEnable = PcdGetBool (PcdCpuSmmCodeAccessCheckEnable);
    ConfigSmmCodeAccessCheck ();
  }

  return EFI_SUCCESS;
}

/**
  The module Entry Point of the CPU SMM driver.

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval Other          Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PiCpuSmmEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  VOID                       *Buffer;
  UINTN                      TileSize;
  UINT8                      *Stacks;

  CopyMem (&mTinySmmCoreContext, ImageHandle, sizeof(mTinySmmCoreContext));

  //
  // Fix segment address of the long-mode-switch jmp
  //
  if (sizeof (UINTN) == sizeof (UINT64)) {
    gSmmJmpAddr.Segment = 8;
  }

  //
  // Find out SMRR Base and Size
  //
  FindSmramInfo (&mCpuHotPlugData.SmrrBase, &mCpuHotPlugData.SmrrSize);

  DEBUG ((EFI_D_INFO, "Calculate SMBASE\n"));

  //
  // The CPU save state and code for the SMI entry point are tiled within an SMRAM 
  // allocated buffer.  The miniumu size of this buffer for a uniprocessor system 
  // is 32 KB, because the entry point is SMBASE + 32KB, and CPU save state area 
  // just below SMBASE + 64KB.  If more than one CPU is present in the platform, 
  // then the SMI entry point and the CPU save state areas can be tiles to minimize 
  // the total amount SMRAM required for all the CPUs.  The tile size can be computed 
  // by adding the   // CPU save state size, any extra CPU specific context, and 
  // the size of code that must be placed at the SMI entry point to transfer 
  // control to a C function in the native SMM execution mode.  This size is 
  // rounded up to the nearest power of 2 to give the tile size for a each CPU.
  // The total amount of memory required is the maximum number of CPUs that 
  // platform supports times the tile size.  The picture below shows the tiling, 
  // where m is the number of tiles that fit in 32KB.
  //
  //  +-----------------------------+  <-- 2^n offset from Base of allocated buffer
  //  |   CPU m+1 Save State        |
  //  +-----------------------------+
  //  |   CPU m+1 Extra Data        |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 2m  SMI Entry         |
  //  +#############################+  <-- Base of allocated buffer + 64 KB 
  //  |   CPU m-1 Save State        |
  //  +-----------------------------+
  //  |   CPU m-1 Extra Data        |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 2m-1 SMI Entry        |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   . . . . . . . . . . . .   |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   CPU 2 Save State          |
  //  +-----------------------------+
  //  |   CPU 2 Extra Data          |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU m+1 SMI Entry         |
  //  +=============================+  <-- Base of allocated buffer + 32 KB
  //  |   CPU 1 Save State          |
  //  +-----------------------------+
  //  |   CPU 1 Extra Data          |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU m SMI Entry           |
  //  +#############################+  <-- Base of allocated buffer + 32 KB == CPU 0 SMBASE + 64 KB
  //  |   CPU 0 Save State          |
  //  +-----------------------------+
  //  |   CPU 0 Extra Data          |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU m-1 SMI Entry         |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   . . . . . . . . . . . .   |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 1 SMI Entry           |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 0 SMI Entry           |
  //  +#############################+  <-- Base of allocated buffer == CPU 0 SMBASE + 32 KB
  //

  //
  // Compute tile size of buffer required to hold CPU save state, any extra CPU 
  // specific context, and the SMI entry point.  This size of rounded up to 
  // nearest power of 2.
  //
  TileSize = 2 * GetPowerOfTwo32 (sizeof (EFI_SMM_CPU_STATE) + sizeof (PROCESSOR_SMM_DESCRIPTOR) + gcSmiHandlerSize - 1);

  //
  // Allocate buffer for all of the tiles. 
  // For 486/Pentium, the SMBASE (and hence the buffer) must be aligned on a 32KB boundary
  //
  Buffer = MyAllocatePages (EFI_SIZE_TO_PAGES (SIZE_64KB));
  Buffer = (VOID *)(((UINTN)Buffer & 0xFFFF8000) + SIZE_32KB);
  ASSERT (Buffer != NULL);

  //
  // Retrieve APIC ID of each enabled processor from the MP Services protocol.
  // Also compute the SMBASE address, CPU Save State address, and CPU Save state 
  // size for each CPU in the platform
  //
  mCpuHotPlugData.SmBase[0]          = (UINTN)Buffer - SIZE_32KB;

  mCpuHotPlugData.ApicId[0] = 0;
      
  DEBUG ((EFI_D_INFO, "CPU[%03x]  APIC ID=%04x  SMBASE=%08x\n",
        0, 
        (UINT32)mCpuHotPlugData.ApicId[0], 
        mCpuHotPlugData.SmBase[0]
        ));

  //
  // Allocate SMI stacks for all processors.
  //
  mSmmStackSize = PcdGet32 (PcdCpuSmmStackSize);
  Stacks = (UINT8 *) MyAllocatePages (EFI_SIZE_TO_PAGES (1 * mSmmStackSize));
  ASSERT (Stacks != NULL);

  //
  // Set SMI stack for SMM base relocation
  //
  gSmmInitStack = (UINTN) (Stacks + mSmmStackSize - sizeof (UINTN));

  DEBUG ((EFI_D_INFO, "SmmRelocateBases...\n"));

  //
  // Relocate SMM Base addresses to the ones allocated from SMRAM
  // BUGBUG: Work on later
  //
  SmmRelocateBases ();

  DEBUG ((EFI_D_INFO, "SmmRelocateBases Done\n"));

  //
  // Initialize IDT
  // BUGBUG: Work on later
  //
  InitializeIDT ();

  SetMem (mExternalVectorTable, sizeof(mExternalVectorTable), 0);
  //
  // Set the pointer to the array of C based exception handling routines.
  //
  InitializeSmmExternalVectorTablePtr (mExternalVectorTable);

  DEBUG ((EFI_D_INFO, "InitializeMpServiceData ...\n"));

  //
  // Initialize MP globals
  //
  InitializeMpServiceData (ImageHandle, Stacks, mSmmStackSize);

  DEBUG ((EFI_D_INFO, "InitializeMpServiceData Done\n"));

  //
  // Install the SMM Configuration Protocol onto a new handle on the handle database.
  // The entire SMM Configuration Protocol is allocated from SMRAM, so only a pointer
  // to an SMRAM address will be present in the handle database 
  //
  Status = SystemTable->BootServices->InstallMultipleProtocolInterfaces (
                                        &gSmmCpuPrivate->SmmCpuHandle,
                                        &gEfiSmmConfigurationProtocolGuid, &gSmmCpuPrivate->SmmConfiguration,
                                        NULL
                                        );
  ASSERT_EFI_ERROR (Status);

  //
  // At the end of enable smi sources
  //
  SmmPlatformEnableSmiSources ();

  DEBUG ((EFI_D_INFO, "SMM CPU Module exit from SMRAM with EFI_SUCCESS\n"));

  return EFI_SUCCESS;
}

/**

  Find out SMRAM information including SMRR base and SMRR size.

  @param          SmrrBase          SMRR base
  @param          SmrrSize          SMRR size
  
**/
VOID
FindSmramInfo (
  OUT UINT32   *SmrrBase,
  OUT UINT32   *SmrrSize
  )
{
  *SmrrBase = mTinySmmCoreContext.SmrrBase;
  *SmrrSize = mTinySmmCoreContext.SmrrSize;

  DEBUG ((EFI_D_INFO, "SMRR Base: 0x%x, SMRR Size: 0x%x\n", *SmrrBase, *SmrrSize));
}

