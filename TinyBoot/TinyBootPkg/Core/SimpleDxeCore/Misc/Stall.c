/** @file
  UEFI Miscellaneous boot Services Stall service implementation

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

//
// Include statements
//

#include "DxeMain.h"

/**
  Internal worker function to call the Metronome Architectural Protocol for 
  the number of ticks specified by the UINT64 Counter value.  WaitForTick() 
  service of the Metronome Architectural Protocol uses a UINT32 for the number
  of ticks to wait, so this function loops when Counter is larger than 0xffffffff.

  @param  Counter           Number of ticks to wait.

**/
VOID
CoreInternalWaitForTick (
  IN UINT64  Counter
  )
{
  while ((Counter & 0xffffffff00000000ULL) != 0) {
    gMetronome->WaitForTick (gMetronome, 0xffffffff);
    Counter -= 0xffffffff;
  }
  gMetronome->WaitForTick (gMetronome, (UINT32)Counter);
}

/**
  Introduces a fine-grained stall.

  @param  Microseconds           The number of microseconds to stall execution.

  @retval EFI_SUCCESS            Execution was stalled for at least the requested
                                 amount of microseconds.
  @retval EFI_NOT_AVAILABLE_YET  gMetronome is not available yet

**/
EFI_STATUS
EFIAPI
CoreStall (
  IN UINTN            Microseconds
  )
{
  UINT64  Counter;
  UINT32  Remainder;
  UINTN   Index;

  if (gMetronome == NULL) {
    return EFI_NOT_AVAILABLE_YET;
  }

  //
  // Counter = Microseconds * 10 / gMetronome->TickPeriod
  // 0x1999999999999999 = (2^64 - 1) / 10
  //
  if (Microseconds > 0x1999999999999999ULL) {
    //
    // Microseconds is too large to multiple by 10 first.  Perform the divide 
    // operation first and loop 10 times to avoid 64-bit math overflow.
    //
    Counter = DivU64x32Remainder (
                Microseconds,
                gMetronome->TickPeriod,
                &Remainder
                );
    for (Index = 0; Index < 10; Index++) {
      CoreInternalWaitForTick (Counter);
    }      

    if (Remainder != 0) {
      //
      // If Remainder was not zero, then normally, Counter would be rounded 
      // up by 1 tick.  In this case, since a loop for 10 counts was used
      // to emulate the multiply by 10 operation, Counter needs to be rounded
      // up by 10 counts.
      //
      CoreInternalWaitForTick (10);
    }
  } else {
    //
    // Calculate the number of ticks by dividing the number of microseconds by
    // the TickPeriod.  Calculation is based on 100ns unit.
    //
    Counter = DivU64x32Remainder (
                MultU64x32 (Microseconds, 10),
                gMetronome->TickPeriod,
                &Remainder
                );
    if (Remainder != 0) {
      //
      // If Remainder is not zero, then round Counter up by one tick.
      //
      Counter++;
    }
    CoreInternalWaitForTick (Counter);
  }

  return EFI_SUCCESS;
}
