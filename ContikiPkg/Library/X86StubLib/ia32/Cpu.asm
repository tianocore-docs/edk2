;------------------------------------------------------------------------------
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
;------------------------------------------------------------------------------

.686p
.model  c, flat        

EXTRN PrevIH:DWORD
EXTRN mIsSystemSched:BYTE
EXTRN clock_isr:PROC

@_REGFILE          STRUCT 2t
RegEax               DWORD         ?
RegEbx               DWORD         ?
RegEcx               DWORD         ?
RegEdx               DWORD         ?
RegEsi               DWORD         ?
RegEdi               DWORD         ?
RegEbp               DWORD         ?
RegEsp               DWORD         ?
RegCs                WORD          ?
RegDs                WORD          ?
RegSs                WORD          ?
RegEs                WORD          ?
RegFs                WORD          ?
RegGs                WORD          ?
RegEflags            DWORD         ?
RegEip               DWORD         ?
@_REGFILE          ENDS
_REGFILE          TYPEDEF         @_REGFILE

.data

.code

Irq0Handler PROC PUBLIC
    pushfd
    push        cs
    call        dword ptr PrevIH

    cmp         mIsSystemSched, 1
    jne         Irq0HandlerExit

    pushad
    call        clock_isr
    popad

Irq0HandlerExit:
    iretd
Irq0Handler ENDP

END

