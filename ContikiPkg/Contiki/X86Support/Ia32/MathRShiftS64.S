#------------------------------------------------------------------------------
#
# Copyright (c) 2014, Intel Corporation. All rights reserved.
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#------------------------------------------------------------------------------


    .686:
    .code:

ASM_GLOBAL ASM_PFX(__ashrdi3)

#------------------------------------------------------------------------------
#
# void __cdecl __ashrdi3 (void)
#
#------------------------------------------------------------------------------
ASM_PFX(__ashrdi3):
    #
    # Checking: Only handle 64bit shifting or more
    #
    cmpb    $64, %cl
    jae     _Exit

    #
    # Handle shifting between 0 and 31 bits
    #
    cmpb    $32, %cl
    jae     More32
    shrd    %cl, %edx, %eax
    sar     %cl, %edx
    ret

    #
    # Handle shifting of 32-63 bits
    #
More32:
    movl    %edx, %eax
    sar     $31, %edx
    and     $31, %cl
    sar     %cl, %eax
    ret

    #
    # Return 0 or -1, depending on the sign of edx
    #
_Exit:
    sar     $31, %edx
    movl    %edx, %eax
    ret
