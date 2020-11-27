;-------------------------------------------------------------------------------
;
; Copyright (c) 2014, Intel Corporation. All rights reserved.
; This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php
; 
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
;-------------------------------------------------------------------------------

.CODE

EXTRN PrevIH:QWORD
EXTRN mIsSystemSched:BYTE
EXTRN clock_isr:PROC

@_REGFILE          STRUCT 2t
RegEax               QWORD         ?
RegEbx               QWORD         ?
RegEcx               QWORD         ?
RegEdx               QWORD         ?
RegEsi               QWORD         ?
RegEdi               QWORD         ?
RegEbp               QWORD         ?
RegEsp               QWORD         ?
RegCs                WORD          ?
RegDs                WORD          ?
RegSs                WORD          ?
RegEs                WORD          ?
RegFs                WORD          ?
RegGs                WORD          ?
RegEflags            QWORD         ?
RegEip               QWORD         ?
Rsvd                 DWORD         ?
RegR8                QWORD         ?
RegR9                QWORD         ?
RegR10               QWORD         ?
RegR11               QWORD         ?
RegR12               QWORD         ?
RegR13               QWORD         ?
RegR14               QWORD         ?
RegR15               QWORD         ?
RegXmm0              OWORD         ?
RegXmm1              OWORD         ?
RegXmm2              OWORD         ?
RegXmm3              OWORD         ?
RegXmm4              OWORD         ?
RegXmm5              OWORD         ?
RegXmm6              OWORD         ?
RegXmm7              OWORD         ?
RegXmm8              OWORD         ?
RegXmm9              OWORD         ?
RegXmm10             OWORD         ?
RegXmm11             OWORD         ?
RegXmm12             OWORD         ?
RegXmm13             OWORD         ?
RegXmm14             OWORD         ?
RegXmm15             OWORD         ?
@_REGFILE          ENDS
_REGFILE          TYPEDEF         @_REGFILE

PUSHAQ MACRO
    push        rax
    push        rcx
    push        rdx
    push        rbx
    push        rsi
    push        rdi
    push        rbp
    push        rbp ; should be rsp
    push        r8
    push        r9
    push        r10
    push        r11
    push        r12
    push        r13
    push        r14
    push        r15
    sub         rsp, 10h
    movdqu      [rsp], xmm0
    sub         rsp, 10h
    movdqu      [rsp], xmm1
    sub         rsp, 10h
    movdqu      [rsp], xmm2
    sub         rsp, 10h
    movdqu      [rsp], xmm3
    sub         rsp, 10h
    movdqu      [rsp], xmm4
    sub         rsp, 10h
    movdqu      [rsp], xmm5
    sub         rsp, 10h
    movdqu      [rsp], xmm6
    sub         rsp, 10h
    movdqu      [rsp], xmm7
    sub         rsp, 10h
    movdqu      [rsp], xmm8
    sub         rsp, 10h
    movdqu      [rsp], xmm9
    sub         rsp, 10h
    movdqu      [rsp], xmm10
    sub         rsp, 10h
    movdqu      [rsp], xmm11
    sub         rsp, 10h
    movdqu      [rsp], xmm12
    sub         rsp, 10h
    movdqu      [rsp], xmm13
    sub         rsp, 10h
    movdqu      [rsp], xmm14
    sub         rsp, 10h
    movdqu      [rsp], xmm15
ENDM

POPAQ MACRO
    movdqu      xmm15, [rsp]
    add         rsp, 10h
    movdqu      xmm14, [rsp]
    add         rsp, 10h
    movdqu      xmm13, [rsp]
    add         rsp, 10h
    movdqu      xmm12, [rsp]
    add         rsp, 10h
    movdqu      xmm11, [rsp]
    add         rsp, 10h
    movdqu      xmm10, [rsp]
    add         rsp, 10h
    movdqu      xmm9, [rsp]
    add         rsp, 10h
    movdqu      xmm8, [rsp]
    add         rsp, 10h
    movdqu      xmm7, [rsp]
    add         rsp, 10h
    movdqu      xmm6, [rsp]
    add         rsp, 10h
    movdqu      xmm5, [rsp]
    add         rsp, 10h
    movdqu      xmm4, [rsp]
    add         rsp, 10h
    movdqu      xmm3, [rsp]
    add         rsp, 10h
    movdqu      xmm2, [rsp]
    add         rsp, 10h
    movdqu      xmm1, [rsp]
    add         rsp, 10h
    movdqu      xmm0, [rsp]
    add         rsp, 10h
    pop         r15
    pop         r14
    pop         r13
    pop         r12
    pop         r11
    pop         r10
    pop         r9
    pop         r8
    pop         rbp ; should be rsp
    pop         rbp
    pop         rdi
    pop         rsi
    pop         rbx
    pop         rdx
    pop         rcx
    pop         rax
ENDM

Irq0Handler PROC PUBLIC
; Stack layout:
;
; +------+
; |  RAX |
; +------+
; |  RIP |
; |  CS  |
; |RFLAGS|
; |  RSP |
; |  SS  |
; +------+
; | Dummy|<-for alignment
; +------+
; |  RIP |<-RSP (unaligned)
; |  CS  |
; |RFLAGS|
; |  RSP |
; |  SS  |
; +------+
    mov         [rsp - 38h], rax    ; save rax to stack

    pushfq
    pop         rax
    mov         [rsp - 20h], rax

    mov         rax, ss
    mov         [rsp - 10h], rax
    mov         rax, rsp
    mov         [rsp - 18h], rax
    sub         rsp, 20h

    mov         rax, cs
    push        rax

    mov         rax, [rsp - 10h]    ; restore rax
    call        qword ptr PrevIH

    cmp         mIsSystemSched, 1
    jne         Irq0HandlerExit

    PUSHAQ
    sub         rsp, 28h
    call        clock_isr
    add         rsp, 28h
    POPAQ

Irq0HandlerExit:
    iretq
Irq0Handler ENDP

END

