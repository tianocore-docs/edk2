/** @file
  Implementation of SMM Timestamp and Stall2 protocols.

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiSmm.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SmmTimestamp.h>
#include <Protocol/SmmStall2.h>

//
// The StartValue in TimerLib
//
UINT64 mTimerLibStartValue = 0;

//
// The EndValue in TimerLib
//
UINT64 mTimerLibEndValue = 0;

//
// The properties of timestamp
//
EFI_TIMESTAMP_PROPERTIES mTimestampProperties = {
  0,
  0
};

/**
  Retrieves the current value of a 64-bit free running timestamp counter.

  The counter shall count up in proportion to the amount of time that has passed. The counter value
  will always roll over to zero. The properties of the counter can be retrieved from GetProperties().
  The caller should be prepared for the function to return the same value twice across successive calls.
  The counter value will not go backwards other than when wrapping, as defined by EndValue in GetProperties().
  The frequency of the returned timestamp counter value must remain constant. Power management operations that
  affect clocking must not change the returned counter frequency. The quantization of counter value updates may
  vary as long as the value reflecting time passed remains consistent.

  @retval The current value of the free running timestamp counter.

**/
UINT64
EFIAPI
TimestampSmmGetTimestamp (
  VOID
  )
{
  //
  // The timestamp of Timestamp Protocol
  //
  UINT64  TimestampValue;
  TimestampValue = 0;

  //
  // Get the timestamp
  //
  if (mTimerLibStartValue > mTimerLibEndValue) {
    TimestampValue = mTimerLibStartValue - GetPerformanceCounter();
  } else {
    TimestampValue = GetPerformanceCounter() - mTimerLibStartValue;
  }

  return TimestampValue;
}

/**
  Obtains timestamp counter properties including frequency and value limits.

  @param[out]  Properties              The properties of the timestamp counter.

  @retval      EFI_SUCCESS             The properties were successfully retrieved.
  @retval      EFI_DEVICE_ERROR        An error occurred trying to retrieve the properties of the timestamp
                                       counter subsystem. Properties is not pedated.
  @retval      EFI_INVALID_PARAMETER   Properties is NULL.

**/
EFI_STATUS
EFIAPI
TimestampSmmGetProperties(
  OUT   EFI_TIMESTAMP_PROPERTIES       *Properties
  )
{
  if (Properties == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get timestamp properties
  //
  CopyMem((VOID *) Properties, (VOID *) &mTimestampProperties, sizeof (mTimestampProperties));

  return EFI_SUCCESS;
}

//
// The SMM Timestamp Protocol instance produced by this driver
//
EDKII_SMM_TIMESTAMP_PROTOCOL  mSmmTimestamp = {
  TimestampSmmGetTimestamp,
  TimestampSmmGetProperties
};

/**
  Induces a fine-grained stall.

  @param[in] Nanoseconds        The number of nanoseconds to stall execution.

  @retval EFI_SUCCESS           Execution was stalled at least the requested number of
                                Nanoseconds.

**/
EFI_STATUS
EFIAPI
SmmStall2 (
  IN UINTN                      Nanoseconds
  )
{
  NanoSecondDelay (Nanoseconds);

  return EFI_SUCCESS;
}

//
// The SMM Stall2 Protocol instance produced by this driver
//
EDKII_SMM_STALL2_PROTOCOL  mSmmStall2 = {
  SmmStall2
};

/**
  Entry point of the Timestamp SMM driver.

  @param  ImageHandle   The image handle of this driver.
  @param  SystemTable   The pointer of EFI_SYSTEM_TABLE.

  @retval EFI_SUCCESS   SMM Timestamp and Stall2 protocols successfully installed.

**/
EFI_STATUS
EFIAPI
TimestampSmmInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  EFI_HANDLE  TimestampHandle;
  TimestampHandle = NULL;

  //
  // Get the start value, end value and frequency in Timerlib
  //
  mTimestampProperties.Frequency = GetPerformanceCounterProperties(&mTimerLibStartValue, &mTimerLibEndValue);

  //
  // Set the EndValue
  //
  if (mTimerLibEndValue > mTimerLibStartValue) {
    mTimestampProperties.EndValue = mTimerLibEndValue - mTimerLibStartValue;
  } else {
    mTimestampProperties.EndValue = mTimerLibStartValue - mTimerLibEndValue;
  }

  DEBUG ((
    DEBUG_INFO,
    "%a() Frequency:0x%lx, TimerLibStartValue:0x%lx, TimerLibEndValue:0x%lx\n",
    __FUNCTION__,
    mTimestampProperties.Frequency,
    mTimerLibStartValue,
    mTimerLibEndValue
    ));

  //
  // Install the SMM Timestamp Protocol onto a new handle
  //
  Status = gSmst->SmmInstallProtocolInterface (
                    &TimestampHandle,
                    &gEdkiiSmmTimestampProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mSmmTimestamp
                    );
  ASSERT_EFI_ERROR (Status);

  //
  // Install the SMM Stall2 Protocol
  //
  Status = gSmst->SmmInstallProtocolInterface (
                    &TimestampHandle,
                    &gEdkiiSmmStall2ProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mSmmStall2
                    );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
