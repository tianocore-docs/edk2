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


#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/RealTimeClock.h>
#include <Protocol/Reset.h>
#include <Protocol/Cpu.h>
#include <Protocol/Metronome.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/Capsule.h>
#include <Protocol/WatchdogTimer.h>
#include <Protocol/Security.h>
#include <Protocol/Variable.h>
#include <Protocol/MonotonicCounter.h>
#include <Protocol/Timer.h>

/**
  @param  Arg1                   Undefined
  @param  Arg2                   Undefined

  @return EFI_NOT_FOUND

**/
EFI_STATUS
EFIAPI
EfiNotFoundArg2 (
  UINTN Arg1,
  UINTN Arg2
  )
{
  return EFI_NOT_FOUND;
}


/**
  @return EFI_UNSUPPORTED

**/
EFI_STATUS
EFIAPI
EfiUnsupportedArg0 (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}


/**
  @param  Arg1                   Undefined

  @return EFI_UNSUPPORTED

**/
EFI_STATUS
EFIAPI
EfiUnsupportedArg1 (
  UINTN Arg1
  )
{
  return EFI_UNSUPPORTED;
}


/**
  @param  Arg1                   Undefined
  @param  Arg2                   Undefined

  @return EFI_UNSUPPORTED

**/
EFI_STATUS
EFIAPI
EfiUnsupportedArg2 (
  UINTN Arg1,
  UINTN Arg2
  )
{
  return EFI_UNSUPPORTED;
}


/**
  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined

  @return EFI_UNSUPPORTED

**/
EFI_STATUS
EFIAPI
EfiUnsupportedArg3 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3
  )
{
  return EFI_UNSUPPORTED;
}


/**
  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined
  @param  Arg4                   Undefined

  @return EFI_UNSUPPORTED

**/
EFI_STATUS
EFIAPI
EfiUnsupportedArg4 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4
  )
{
  return EFI_UNSUPPORTED;
}


/**
  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined
  @param  Arg4                   Undefined
  @param  Arg5                   Undefined

  @return EFI_UNSUPPORTED

**/
EFI_STATUS
EFIAPI
EfiUnsupportedArg5 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4,
  UINTN Arg5
  )
{
  return EFI_UNSUPPORTED;
}

EFI_RUNTIME_SERVICES mEfiRuntimeServicesTableTemplate = {
  {
    0,                                                        // Signature
    0,                                                        // Revision
    0,                                                        // HeaderSize
    0,                                                        // CRC32
    0                                                         // Reserved
  },
  (EFI_GET_TIME)                    NULL, //EfiUnsupportedArg2,       // GetTime
  (EFI_SET_TIME)                    NULL, //EfiUnsupportedArg1,       // SetTime
  (EFI_GET_WAKEUP_TIME)             NULL, //EfiUnsupportedArg3,       // GetWakeupTime
  (EFI_SET_WAKEUP_TIME)             NULL, //EfiUnsupportedArg2,       // SetWakeupTime
  (EFI_SET_VIRTUAL_ADDRESS_MAP)     NULL, //EfiUnsupportedArg4,       // SetVirtualAddressMap
  (EFI_CONVERT_POINTER)             NULL, //EfiUnsupportedArg2,       // ConvertPointer
  (EFI_GET_VARIABLE)                NULL, //EfiUnsupportedArg5,       // GetVariable
  (EFI_GET_NEXT_VARIABLE_NAME)      NULL, //EfiUnsupportedArg3,       // GetNextVariableName
  (EFI_SET_VARIABLE)                NULL, //EfiUnsupportedArg5,       // SetVariable
  (EFI_GET_NEXT_HIGH_MONO_COUNT)    EfiUnsupportedArg1,       // GetNextHighMonotonicCount
  (EFI_RESET_SYSTEM)                NULL, //EfiUnsupportedArg4,       // ResetSystem
  (EFI_UPDATE_CAPSULE)              EfiUnsupportedArg3,       // UpdateCapsule
  (EFI_QUERY_CAPSULE_CAPABILITIES)  NULL, //EfiUnsupportedArg4,       // QueryCapsuleCapabilities
  (EFI_QUERY_VARIABLE_INFO)         NULL, //EfiUnsupportedArg4        // QueryVariableInfo
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_CPU_ARCH_PROTOCOL mCpuArchProtocol = {
  (EFI_CPU_FLUSH_DATA_CACHE)            EfiUnsupportedArg4,
  (EFI_CPU_ENABLE_INTERRUPT)            EfiUnsupportedArg1,
  (EFI_CPU_DISABLE_INTERRUPT)           EfiUnsupportedArg1,
  (EFI_CPU_GET_INTERRUPT_STATE)         EfiUnsupportedArg2,
  (EFI_CPU_INIT)                        EfiUnsupportedArg2,
  (EFI_CPU_REGISTER_INTERRUPT_HANDLER)  EfiUnsupportedArg3,
  (EFI_CPU_GET_TIMER_VALUE)             EfiUnsupportedArg4,
  (EFI_CPU_SET_MEMORY_ATTRIBUTES)       EfiUnsupportedArg4,
  0, //                              NumberOfTimers;
  0  //                              DmaBufferAlignment;
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_TIMER_ARCH_PROTOCOL mTimerArchProtocol = {
  (EFI_TIMER_REGISTER_HANDLER)          EfiUnsupportedArg2,
  (EFI_TIMER_SET_TIMER_PERIOD)          EfiUnsupportedArg2,
  (EFI_TIMER_GET_TIMER_PERIOD)          EfiUnsupportedArg2,
  (EFI_TIMER_GENERATE_SOFT_INTERRUPT)   EfiUnsupportedArg1,
};

typedef struct {
  EFI_GUID  *Protocol;
  VOID      *Value;
} ARCH_PROTOCOL_MAP;

ARCH_PROTOCOL_MAP ProtocolList[] = {
  { &gEfiSecurityArchProtocolGuid,              NULL },
  //{ &gEfiCpuArchProtocolGuid,                   (VOID*) &mCpuArchProtocol },
  //{ &gEfiMetronomeArchProtocolGuid,             NULL },
  // { &gEfiTimerArchProtocolGuid,                 (VOID*) &mTimerArchProtocol },
  //{ &gEfiBdsArchProtocolGuid,                 NULL },
  { &gEfiWatchdogTimerArchProtocolGuid,         NULL },
  //{ &gEfiRuntimeArchProtocolGuid,             NULL },
  // { &gEfiVariableArchProtocolGuid,              NULL },
  // { &gEfiVariableWriteArchProtocolGuid,         NULL },
  { &gEfiCapsuleArchProtocolGuid,               NULL },
  { &gEfiMonotonicCounterArchProtocolGuid,      NULL },
//  { &gEfiResetArchProtocolGuid,                 NULL },
//  { &gEfiRealTimeClockArchProtocolGuid,         NULL },
};

RETURN_STATUS
EFIAPI
NullArchProtocolsLibConstructor (
  VOID
  )
{
  EFI_HANDLE   Handle;
  UINTN        Loop;
  VOID**       Src;
  VOID**       Dst;

  for (Src = (VOID**) &mEfiRuntimeServicesTableTemplate.GetTime,
       Dst = (VOID**) &(gRT->GetTime);
       Src < (VOID**) ((&mEfiRuntimeServicesTableTemplate) + 1);
       Src++, Dst++
       ) {
    if (*Src != NULL) {
      *Dst = *Src;
    }
  }

  for (Loop = 0; Loop < (sizeof (ProtocolList) / sizeof (ProtocolList[0])); Loop++) {
    Handle = NULL;
    gBS->InstallProtocolInterface (
            &Handle,
            ProtocolList[Loop].Protocol,
            EFI_NATIVE_INTERFACE,
            ProtocolList[Loop].Value
            );
  }

  return RETURN_SUCCESS;
}
