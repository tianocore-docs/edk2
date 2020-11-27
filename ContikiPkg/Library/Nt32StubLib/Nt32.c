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
 *         Implementation of the clock functions for NT32
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

//
// UEFI world
//
#ifndef CONST
#define CONST const
#endif

#include <WinNtDxe.h>
#include <Library/WinNtLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>

// Duration in ms of each tick
#define TIMER_TICK_DURATION     1

// Timer protocol expects units of 100ns. For a 1 ms tick rate, that means 10000
#define TIMER_TICK_RATE         (TIMER_TICK_DURATION * 10000)

BOOLEAN                 mIsSystemSched = TRUE;

UINT                     mMMTimerThreadID = 0;
UINT                     mTimerPeriod = 10;
HANDLE                   mNtMainThreadHandle;

typedef BOOL (*NT_GET_THREAD_CONTEXT) (
  HANDLE hThread,
  LPCONTEXT lpContext
);

typedef BOOL (*NT_SET_THREAD_CONTEXT) (
  HANDLE hThread,
  const CONTEXT* lpContext
);

NT_GET_THREAD_CONTEXT NtGetThreadContext;
NT_SET_THREAD_CONTEXT NtSetThreadContext;

#if 0
#define K_DLL L"ThreadDll.dll"
#define GET_THREAD_CONTEXT "NtGetThreadContext"
#define SET_THREAD_CONTEXT "NtSetThreadContext"
#else
#define K_DLL L"Kernel32.dll"
#define GET_THREAD_CONTEXT "GetThreadContext"
#define SET_THREAD_CONTEXT "SetThreadContext"
#endif

void
clock_isr(void);
void
rtimer_isr(void);

VOID
CALLBACK
MMTimerThread (
  UINT  wTimerID,
  UINT  msg,
  DWORD dwUser,
  DWORD dw1,
  DWORD dw2
  )
{
  if (gWinNt->SuspendThread (mNtMainThreadHandle) == -1) {
    return;
  }

  if (mIsSystemSched) {
    rtimer_isr ();
    clock_isr ();
  }

  gWinNt->ResumeThread (mNtMainThreadHandle);
}

EFI_STATUS
UefiIvSetup (
  VOID
  )
{
  HMODULE Kernel;
  UINTN   Result;

  Result = gWinNt->DuplicateHandle (
                    gWinNt->GetCurrentProcess (),
                    gWinNt->GetCurrentThread (),
                    gWinNt->GetCurrentProcess (),
                    &mNtMainThreadHandle,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                    );
  if (Result == 0) {
    return EFI_DEVICE_ERROR;
  }

  Kernel = gWinNt->LoadLibraryEx (K_DLL, NULL ,0);
  if (NULL == Kernel) {
    return EFI_NOT_FOUND;
  }

  NtGetThreadContext = (NT_GET_THREAD_CONTEXT)gWinNt->GetProcAddress (Kernel, GET_THREAD_CONTEXT);
  if (NULL == NtGetThreadContext) {
    return EFI_NOT_FOUND;
  }

  NtSetThreadContext = (NT_SET_THREAD_CONTEXT)gWinNt->GetProcAddress (Kernel, SET_THREAD_CONTEXT);
  if (NULL == NtSetThreadContext) {
    return EFI_NOT_FOUND;
  }

  mMMTimerThreadID = gWinNt->timeSetEvent (
                               mTimerPeriod,
                               0,
                               MMTimerThread,
                               (DWORD_PTR) NULL,
                               TIME_PERIODIC | TIME_KILL_SYNCHRONOUS | TIME_CALLBACK_FUNCTION
                               );
  if (mMMTimerThreadID == 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

VOID
UefiIvTeardown (
  VOID
  )
{
  if (mMMTimerThreadID) {
    gWinNt->timeKillEvent (mMMTimerThreadID);
  }

  mMMTimerThreadID = 0;

  return ;
}

VOID
UefiIvInit ()
{
  return ;
}
