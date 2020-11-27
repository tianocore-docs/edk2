#------------------------------------------------------------------------------
#
# Copyright(c) 2013 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# * Neither the name of Intel Corporation nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Module Name:
#
#   SmiException.S
#
# Abstract:
#
#   Exception handlers used in SM mode
#
#------------------------------------------------------------------------------

ASM_GLOBAL  ASM_PFX(gSmiMtrrs)
ASM_GLOBAL  ASM_PFX(gcSmiIdtr)
ASM_GLOBAL  ASM_PFX(gcSmiGdtr)
ASM_GLOBAL  ASM_PFX(gcPsd)
ASM_GLOBAL  ASM_PFX(FeaturePcdGet (PcdCpuSmmStackGuard))
ASM_GLOBAL  ASM_PFX(gSavedPageFaultIdtEntry)
ASM_GLOBAL  ASM_PFX(gSavedDebugExceptionIdtEntry)
ASM_GLOBAL  ASM_PFX(FeaturePcdGet (PcdCpuSmmProfileEnable))
ASM_GLOBAL  ASM_PFX(InitializeSmmExternalVectorTablePtr)

    .data 

NullSeg:    .quad 0
            .quad 0                     # reserved for future use
CodeSeg32:
            .word -1                    # LimitLow
            .word 0                     # BaseLow
            .byte 0                     # BaseMid
            .byte 0x9b
            .byte 0xcf                  # LimitHigh
            .byte 0                     # BaseHigh
DataSeg32:
            .word -1                    # LimitLow
            .word 0                     # BaseLow
            .byte 0                     # BaseMid
            .byte 0x93
            .byte 0xcf                  # LimitHigh
            .byte 0                     # BaseHigh
            .quad 0                     # reserved for future use
CodeSeg16:
            .word -1
            .word 0
            .byte 0
            .byte 0x9b
            .byte 0x8f
            .byte 0
DataSeg16:
            .word -1
            .word 0
            .byte 0
            .byte 0x93
            .byte 0x8f
            .byte 0
CodeSeg64:
            .word -1                    # LimitLow
            .word 0                     # BaseLow
            .byte 0                     # BaseMid
            .byte 0x9b
            .byte 0xaf                  # LimitHigh
            .byte 0                     # BaseHigh
.equ  GDT_SIZE, .- NullSeg

.equ  CODE_SEL,          CodeSeg32 - NullSeg
.equ  DATA_SEL,          DataSeg32 - NullSeg

ASM_PFX(gcPsd):
            .ascii  "PSDSIG  "
            .word      PSD_SIZE
            .word 2
            .word      1 << 2
            .word      CODE_SEL
            .word      DATA_SEL
            .word      DATA_SEL
            .word      DATA_SEL
            .word 0
            .long 0
            .long 0
            .long 0
            .long 0
            .quad 0
            .long      NullSeg
            .long 0
            .long      GDT_SIZE
            .long 0
            .space 24, 0
            .long      ASM_PFX(gSmiMtrrs)
            .long 0
.equ  PSD_SIZE,  . - ASM_PFX(gcPsd)

ASM_PFX(gcSmiGdtr):  .word      GDT_SIZE - 1
                     .long      NullSeg

ASM_PFX(gcSmiIdtr):  .word      IDT_SIZE - 1
                     .long      _SmiIDT

_SmiIDT:
# The following segment repeats 32 times:
# No. 1
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 2
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 3
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 4
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 5
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 6
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 7
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 8
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 9
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 10
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 11
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 12
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 13
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 14
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 15
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 16
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 17
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 18
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 19
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 20
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 21
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 22
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 23
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 24
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 25
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 26
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 27
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 28
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 29
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 30
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 31
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31
# No. 32
    .word 0                             # Offset 0:15
    .word      CODE_SEL
    .byte 0                             # Unused
    .byte 0x8e                          # Interrupt Gate, Present
    .word 0                             # Offset 16:31

.equ  IDT_SIZE, . - _SmiIDT

TaskGateDescriptor:
    .word      0                        # Reserved
    .word      EXCEPTION_TSS_SEL        # TSS Segment selector
    .byte      0                        # Reserved
    .byte      0x85                     # Task Gate, present, DPL = 0
    .word      0                        # Reserved

# point to the external interrupt vector table

ExternalVectorTablePtr:  .long 0

#
# Saved IDT Entry for Page Fault
#
ASM_PFX(gSavedPageFaultIdtEntry):
    .long      0                        
    .long      0        

#
# Saved IDT Entry for INT 1
#
ASM_PFX(gSavedDebugExceptionIdtEntry):
    .long      0                        
    .long      0        
    
    .text

ASM_PFX(InitializeSmmExternalVectorTablePtr):
    movl     4(%esp), %eax
    movl     %eax, ExternalVectorTablePtr
    ret

#------------------------------------------------------------------------------
# Exception handlers
#------------------------------------------------------------------------------
_SmiExceptionHandlers:
.equ  IHDLRIDX, 0
# The following segment repeats 8 times:
# No. 1
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 2
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 3
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 4
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 5
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 6
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 7
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 8
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1

    pushl    $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1

    pushl   %eax                        # dummy error code
    pushl    $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1

# The following segment repeats 5 times:
# No. 1
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 2
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 3
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 4
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 5
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1

# The following segment repeats 2 times:
# No. 1
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 2
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1

    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
    int     $3
.equ  IHDLRIDX, IHDLRIDX + 1

# The following segment repeats 14 times:
# No. 1
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 2
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 3
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 4
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 5
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 6
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 7
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 8
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 9
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 10
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 11
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 12
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 13
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1
# No. 14
    pushl   %eax                        # dummy error code
    pushl   $IHDLRIDX
    .byte 0xe9                          # jmp disp32
    .long      _SmiExceptionEntryPoint - (. + 4)
.equ  IHDLRIDX, IHDLRIDX + 1


#------------------------------------------------------------------------------
# _SmiExceptionEntryPoint is the entry point for all exceptions
#
# Stack:
#+---------------------+
#+    EFlags           +
#+---------------------+
#+    CS               +
#+---------------------+
#+    EIP              +
#+---------------------+
#+    Error Code       +
#+---------------------+
#+    Vector Number    +
#+---------------------+
#+    EBP              +
#+---------------------+ <-- EBP
#
# RSP set to odd multiple of 8 means ErrCode PRESENT
#------------------------------------------------------------------------------
_SmiExceptionEntryPoint:
    pushl   %ebp
    movl    %esp, %ebp


    #
    # Align stack to make sure that EFI_FX_SAVE_STATE_IA32 of EFI_SYSTEM_CONTEXT_IA32
    # is 16-byte aligned
    #
    andl    $0xfffffff0, %esp
    subl    $12, %esp

## UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
    pushl   %eax
    pushl   %ecx
    pushl   %edx
    pushl   %ebx
    leal    (6*4)(%ebp), %ecx
    pushl   %ecx                          # ESP
    pushl   (%ebp)                        # EBP
    pushl   %esi
    pushl   %edi

## UINT32  Gs, Fs, Es, Ds, Cs, Ss;
    movl    %ss, %eax
    pushl   %eax
    movzwl  (4*4)(%ebp), %eax
    pushl   %eax
    movl    %ds, %eax
    pushl   %eax
    movl    %es, %eax
    pushl   %eax
    movl    %fs, %eax
    pushl   %eax
    movl    %gs, %eax
    pushl   %eax

## UINT32  Eip;
    movl    (3*4)(%ebp), %eax
    pushl   %eax

## UINT32  Gdtr[2], Idtr[2];
    subl    $8, %esp
    sidt    (%esp)
    movl    2(%esp), %eax
    xchgl   (%esp), %eax
    andl    $0xffff, %eax
    movl    %eax, 4(%esp)

    subl    $8, %esp
    sgdt    (%esp)
    movl    2(%esp), %eax
    xchgl   (%esp), %eax
    andl    $0xffff, %eax
    movl    %eax, 4(%esp)

## UINT32  Ldtr, Tr;
    xorl    %eax, %eax
    strw    %ax
    pushl   %eax
    sldtw   %ax
    pushl   %eax

## UINT32  EFlags;
    movl    (5*4)(%ebp), %eax
    pushl   %eax

## UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
    movl    %cr4, %eax
    orl     $0x208, %eax
    movl    %eax, %cr4
    pushl   %eax
    movl    %cr3, %eax
    pushl   %eax
    movl    %cr2, %eax
    pushl   %eax
    xorl    %eax, %eax
    pushl   %eax
    movl    %cr0, %eax
    pushl   %eax

## UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
    movl    %dr7, %eax
    pushl   %eax
    movl    %dr6, %eax
    pushl   %eax
    movl    %dr3, %eax
    pushl   %eax
    movl    %dr2, %eax
    pushl   %eax
    movl    %dr1, %eax
    pushl   %eax
    movl    %dr0, %eax
    pushl   %eax

## FX_SAVE_STATE_IA32 FxSaveState;
    subl    $512, %esp
    movl    %esp, %edi
    .byte   0x0f, 0xae, 0x07                  #fxsave [edi]

# UEFI calling convention for IA32 requires that Direction flag in EFLAGs is clear
    cld

## UINT32  ExceptionData;
    pushl   (2*4)(%ebp)

## call into exception handler
    movl    ExternalVectorTablePtr, %eax      # get the interrupt vectors base
    orl     %eax, %eax                        # NULL?
    jz      nullExternalExceptionHandler

    movl    4(%ebp), %ecx
    movl    (%eax, %ecx, 4), %eax
    orl     %eax, %eax                        # NULL?
    jz      nullExternalExceptionHandler

## Prepare parameter and call
    movl    %esp, %edx
    pushl   %edx
    movl    (1*4)(%ebp), %edx
    pushl   %edx

    #
    # Call External Exception Handler
    #
    call    *%eax
    addl    $8, %esp
    jmp     L4

nullExternalExceptionHandler:
# CpuDeadLoop() is the default exception handler since it preserves the processor
# branch log.
    call    ASM_PFX(CpuDeadLoop)

L4:
## UINT32  ExceptionData;
    addl    $4, %esp

## FX_SAVE_STATE_IA32 FxSaveState;
    movl    %esp, %esi
    .byte   0xf, 0xae, 0xe                 # fxrstor [esi]
    addl    $512, %esp

## UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
## Skip restoration of DRx registers to support in-circuit emualators
## or debuggers set breakpoint in interrupt/exception context
    addl    $4*6, %esp

## UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
    popl    %eax
    movl    %eax, %cr0
    addl    $4, %esp                       # not for Cr1
    popl    %eax
    movl    %eax, %cr2
    popl    %eax
    movl    %eax, %cr3
    popl    %eax
    movl    %eax, %cr4

## UINT32  EFlags;
    popl    (5*4)(%ebp)

## UINT32  Ldtr, Tr;
## UINT32  Gdtr[2], Idtr[2];
## Best not let anyone mess with these particular registers...
    addl    $24, %esp

## UINT32  Eip;
    popl    (3*4)(%ebp)

## UINT32  Gs, Fs, Es, Ds, Cs, Ss;
## NOTE - modified segment registers could hang the debugger...  We
##        could attempt to insulate ourselves against this possibility,
##        but that poses risks as well.
##
    popl    %gs
    popl    %fs
    popl    %es
    popl    %ds
    popl    (4*4)(%ebp)
    popl    %ss

## UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
    popl    %edi
    popl    %esi
    addl    $4, %esp                      # not for ebp
    addl    $4, %esp                      # not for esp
    popl    %ebx
    popl    %edx
    popl    %ecx
    popl    %eax

    movl    %ebp, %esp
    popl    %ebp

    addl    $8, %esp                      # skip INT# & ErrCode
    iret

ASM_GLOBAL ASM_PFX(InitializeIDT)
ASM_PFX(InitializeIDT):
    pushl   %ebx
    lea     _SmiIDT - 8, %edx
#    push    IDT_SIZE / 8
    .byte 0x68                          # push /id
    .long      IDT_SIZE / 8
    lea      _SmiExceptionHandlers - 8, %ebx
    popl    %ecx
L1: 
    leal    (%ebx,%ecx,8),%eax
    movw    %ax,(%edx,%ecx,8)
    shrl    $16,%eax
    movw    %ax, 6(%edx, %ecx, 8)
    loop    L1

    ret

