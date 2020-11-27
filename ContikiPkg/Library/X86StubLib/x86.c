/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Implementation of the clock functions for x86
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

//
// UEFI world
//

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/Timer.h>
#include <Protocol/Cpu.h>
#include <Protocol/Legacy8259.h>
#include <Library/PcdLib.h>

// Duration in ms of each tick
#define TIMER_TICK_DURATION     1

// Timer protocol expects units of 100ns. For a 1 ms tick rate, that means 10000
#define TIMER_TICK_RATE         (TIMER_TICK_DURATION * 10000)

BOOLEAN                 mIsSystemSched = TRUE;

UINTN     PrevIH;
static UINT64    IntDescEntry;
#ifdef MDE_CPU_X64
static UINT64     IntDescEntryHi;
#endif

UINTN                       gIrq0Offset;
UINT64                      gDefaultTimerInterval;

VOID
Irq0Handler (
  VOID
  );

EFI_STATUS
UefiIvSetup (
  VOID
  )
{
  IA32_DESCRIPTOR             Idtr;
  BOOLEAN                     OldFlag;
  EFI_STATUS                  Status;
  EFI_TIMER_ARCH_PROTOCOL     *Timer;
  IA32_IDT_GATE_DESCRIPTOR    *Irq0Ptr;

  //
  // Locate the Timer Architecture Protocol and modify the IRQ0 clock rate
  // to a better value than the default 54.9ms.
  //
  Status = gBS->LocateProtocol (
                  &gEfiTimerArchProtocolGuid,
                  NULL,
                  (VOID**)&Timer
                  );
  if (!EFI_ERROR (Status)) {
    Timer->GetTimerPeriod (Timer, &gDefaultTimerInterval);
    Timer->SetTimerPeriod (Timer, TIMER_TICK_RATE);
  }

  OldFlag = SaveAndDisableInterrupts ();

  //
  // Get the address of the IDT.
  //
  AsmReadIdtr (&Idtr);

  //
  // Calculate the address of IRQ0.  This will always be 0x68 (104) in
  // protected mode.
  //
  Irq0Ptr = (IA32_IDT_GATE_DESCRIPTOR *)(Idtr.Base + gIrq0Offset);
  //
  // Save the IRQ0 hander so it can be called later.
  //
  PrevIH = (Irq0Ptr->Bits.OffsetHigh << 16) | Irq0Ptr->Bits.OffsetLow;
#ifdef MDE_CPU_X64
  PrevIH |= ((UINT64)Irq0Ptr->Bits.OffsetUpper << 32);
#endif
  //
  // Save the entire existing IDT entry for restoration on shutdown.
  //
#ifdef MDE_CPU_IA32
  IntDescEntry = Irq0Ptr->Uint64;
#endif
#ifdef MDE_CPU_X64
  IntDescEntry = Irq0Ptr->Uint128.Uint64;
  IntDescEntryHi = Irq0Ptr->Uint128.Uint64_1;
#endif
  //
  // Load the new IRQ0 handler contained in this file.
  //
  Irq0Ptr->Bits.OffsetLow = (UINT16)(UINTN)Irq0Handler;
  Irq0Ptr->Bits.OffsetHigh = (UINT16)((UINTN)Irq0Handler >> 16);
#ifdef MDE_CPU_X64
  Irq0Ptr->Bits.OffsetUpper = (UINT32)((UINT64)(UINTN)Irq0Handler >> 32);
#endif

  AsmWriteIdtr (&Idtr);

  SetInterruptState (OldFlag);

  return EFI_SUCCESS;
}

VOID
UefiIvTeardown (
  VOID
  )
{
  IA32_DESCRIPTOR             Idtr;
  BOOLEAN                     OldFlag;
  EFI_STATUS                  Status;
  EFI_TIMER_ARCH_PROTOCOL     *Timer;
  IA32_IDT_GATE_DESCRIPTOR    *Irq0Ptr;

  //
  // Locate the Timer Architecture Protocol and restore the IRQ0 timer
  // tick rate back to the default.
  //

  Status = gBS->LocateProtocol(
                  &gEfiTimerArchProtocolGuid,
                  NULL,
                  (VOID**) &Timer
                  );
  if (!EFI_ERROR (Status)) {
    Timer->SetTimerPeriod (Timer, gDefaultTimerInterval);
  }

  OldFlag = SaveAndDisableInterrupts ();

  //
  // Get the address of the IDT.
  //
  AsmReadIdtr (&Idtr);
  //
  // Calculate the address of IRQ0.  This will always be 0x68 (104) in
  // protected mode.
  //
  Irq0Ptr = (IA32_IDT_GATE_DESCRIPTOR *)(Idtr.Base + gIrq0Offset);

  //
  // Restore the previous IDT entry.
  //
#ifdef MDE_CPU_IA32
  Irq0Ptr->Uint64 = IntDescEntry;
#endif
#ifdef MDE_CPU_X64
  Irq0Ptr->Uint128.Uint64 = IntDescEntry;
  Irq0Ptr->Uint128.Uint64_1 = IntDescEntryHi;
#endif

  AsmWriteIdtr (&Idtr);

  SetInterruptState (OldFlag);

  return ;
}

VOID
EFIAPI
DummyHandler (
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  ASSERT (FALSE);
}

UINT8
UefiIvGetHpetTimerVector (
  VOID
  )
{
  //
  // Dynamic detect HPET .. TBD
  //
  return PcdGet8 (PcdHpetLocalApicVector);
}

VOID
UefiIvInit ()
{
  EFI_STATUS                  Status;
  EFI_LEGACY_8259_PROTOCOL    *Legacy8259;
  UINT32                      TimerVector;
  EFI_CPU_ARCH_PROTOCOL       *Cpu;
  BOOLEAN                     TimerUses8254;

  TimerVector = 0x68;

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &Cpu);
  ASSERT_EFI_ERROR (Status);

  //
  // Try to see if system registered this Irq0
  //
  TimerUses8254 = FALSE;
  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &Legacy8259);
  if (!EFI_ERROR (Status)) {
    TimerVector = 0;
    Status = Legacy8259->GetVector (Legacy8259, Efi8259Irq0, (UINT8 *) &TimerVector);
    ASSERT_EFI_ERROR (Status);

    Status = Cpu->RegisterInterruptHandler (Cpu, TimerVector, DummyHandler);
    switch (Status) {
    case EFI_SUCCESS:
      //
      // Irq0 is NOT registered, suspect HPET enabled.
      //

      //
      // Unregister dummy handler
      //
      Status = Cpu->RegisterInterruptHandler (Cpu, TimerVector, NULL);
      TimerUses8254 = FALSE;
      break;

    case EFI_ALREADY_STARTED:
      //
      // Irq0 is already registered, just use this.
      //
      TimerUses8254 = TRUE;
      break;

    default:
      // Why ???
      ASSERT (FALSE);
      break;
    }

  }

  //
  // Legacy8259 not found, or Irq0 is not registered.
  // Check HPET here.
  //
  if (!TimerUses8254) {
    TimerVector = UefiIvGetHpetTimerVector ();
  }

  DEBUG ((EFI_D_INFO, "Contiki UefiIvInit - TimerVector - 0x%02x\n", TimerVector));
  gIrq0Offset = TimerVector * sizeof(UINTN) * 2;

  return ;
}
