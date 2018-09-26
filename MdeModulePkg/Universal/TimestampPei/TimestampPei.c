/** @file
  Implementation of Timestamp and Stall2 PPIs.

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Ppi/Timestamp.h>
#include <Ppi/Stall2.h>

#define PEI_TIMESTAMP_INSTANCE_SIGNATURE SIGNATURE_32 ('P', 'T', 'I', 'S')

typedef struct {
  UINTN                         Signature;
  EDKII_PEI_TIMESTAMP_PPI       TimestampPpi;
  EDKII_PEI_STALL2_PPI          Stall2Ppi;
  EFI_PEI_PPI_DESCRIPTOR        PpiList[2];
  UINT64                        TimerLibStartValue;
  UINT64                        TimerLibEndValue;
  EFI_TIMESTAMP_PROPERTIES      TimestampProperties;
} PEI_TIMESTAMP_INSTANCE;

#define PEI_TIMESTAMP_INSTANCE_FROM_TIMESTAMP_PPI(Ppi) \
  CR (Ppi, PEI_TIMESTAMP_INSTANCE, TimestampPpi, PEI_TIMESTAMP_INSTANCE_SIGNATURE)
#define PEI_TIMESTAMP_INSTANCE_FROM_STALL2_PPI(Ppi) \
  CR (Ppi, PEI_TIMESTAMP_INSTANCE, Stall2Ppi, PEI_TIMESTAMP_INSTANCE_SIGNATURE)

/**
  Retrieves the current value of a 64-bit free running timestamp counter.

  The counter shall count up in proportion to the amount of time that has passed. The counter value
  will always roll over to zero. The properties of the counter can be retrieved from GetProperties().
  The caller should be prepared for the function to return the same value twice across successive calls.
  The counter value will not go backwards other than when wrapping, as defined by EndValue in GetProperties().
  The frequency of the returned timestamp counter value must remain constant. Power management operations that
  affect clocking must not change the returned counter frequency. The quantization of counter value updates may
  vary as long as the value reflecting time passed remains consistent.

  @param[in] This           Pointer to the local data for the interface.

  @retval The current value of the free running timestamp counter.

**/
UINT64
EFIAPI
TimestampPeiGetTimestamp (
  IN CONST EDKII_PEI_TIMESTAMP_PPI      *This
  )
{
  //
  // The timestamp of Timestamp Protocol
  //
  UINT64                    TimestampValue;
  PEI_TIMESTAMP_INSTANCE    *Instance;

  TimestampValue = 0;

  Instance = PEI_TIMESTAMP_INSTANCE_FROM_TIMESTAMP_PPI (This);
  ASSERT (Instance != NULL);

  //
  // Get the timestamp
  //
  if (Instance->TimerLibStartValue > Instance->TimerLibEndValue) {
    TimestampValue = Instance->TimerLibStartValue - GetPerformanceCounter();
  } else {
    TimestampValue = GetPerformanceCounter() - Instance->TimerLibStartValue;
  }

  return TimestampValue;
}

/**
  Obtains timestamp counter properties including frequency and value limits.

  @param[in]  This          Pointer to the local data for the interface.
  @param[out] Properties    The properties of the timestamp counter.

  @retval EFI_SUCCESS               The properties were successfully retrieved.
  @retval EFI_DEVICE_ERROR          An error occurred trying to retrieve the properties of the timestamp
                                    counter subsystem. Properties is not pedated.
  @retval EFI_INVALID_PARAMETER     Properties is NULL.

**/
EFI_STATUS
EFIAPI
TimestampPeiGetProperties(
  IN CONST EDKII_PEI_TIMESTAMP_PPI      *This,
  OUT EFI_TIMESTAMP_PROPERTIES          *Properties
  )
{
  PEI_TIMESTAMP_INSTANCE    *Instance;

  if (Properties == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = PEI_TIMESTAMP_INSTANCE_FROM_TIMESTAMP_PPI (This);
  ASSERT (Instance != NULL);

  //
  // Get timestamp properties
  //
  CopyMem (
    (VOID *) Properties,
    (VOID *) &Instance->TimestampProperties,
    sizeof (Instance->TimestampProperties)
    );

  return EFI_SUCCESS;
}

/**
  The Stall2() function provides a blocking stall for at least the number
  of nanoseconds stipulated in the final argument of the API.

  @param[in] This           Pointer to the local data for the interface.
  @param[in] Nanoseconds    Number of nanoseconds for which to stall.

  @retval EFI_SUCCESS   The service provided at least the required delay.

**/
EFI_STATUS
EFIAPI
PeiStall2 (
  IN CONST EDKII_PEI_STALL2_PPI     *This,
  IN UINTN                          Nanoseconds
  )
{
  NanoSecondDelay (Nanoseconds);

  return EFI_SUCCESS;
}

/**
  Entry point of the Timestamp PEIM.

  @param FileHandle     Handle of the file being invoked.
  @param PeiServices    Describes the list of possible PEI Services.

  @retval EFI_SUCCESS   Timestamp and Stall2 PPIs successfully installed.

**/
EFI_STATUS
EFIAPI
TimestampPeiInitialize (
  IN EFI_PEI_FILE_HANDLE    FileHandle,
  IN CONST EFI_PEI_SERVICES **PeiServices
  )
{
  EFI_STATUS                Status;
  PEI_TIMESTAMP_INSTANCE    *Instance;

  Instance = AllocatePool (sizeof (*Instance));
  ASSERT (Instance != NULL);

  Instance->Signature = PEI_TIMESTAMP_INSTANCE_SIGNATURE;

  //
  // Get the start value, end value and frequency in Timerlib
  //
  Instance->TimestampProperties.Frequency = GetPerformanceCounterProperties (
                                              &Instance->TimerLibStartValue,
                                              &Instance->TimerLibEndValue
                                              );

  //
  // Set the EndValue
  //
  if (Instance->TimerLibEndValue > Instance->TimerLibStartValue) {
    Instance->TimestampProperties.EndValue = Instance->TimerLibEndValue - Instance->TimerLibStartValue;
  } else {
    Instance->TimestampProperties.EndValue = Instance->TimerLibStartValue - Instance->TimerLibEndValue;
  }

  DEBUG ((
    DEBUG_INFO,
    "%a() Frequency:0x%lx, TimerLibStartValue:0x%lx, TimerLibEndValue:0x%lx\n",
    __FUNCTION__,
    Instance->TimestampProperties.Frequency,
    Instance->TimerLibStartValue,
    Instance->TimerLibEndValue
    ));


  Instance->TimestampPpi.GetTimestamp = TimestampPeiGetTimestamp;
  Instance->TimestampPpi.GetProperties = TimestampPeiGetProperties;

  Instance->PpiList[0].Flags = EFI_PEI_PPI_DESCRIPTOR_PPI;
  Instance->PpiList[0].Guid = &gEdkiiPeiTimestampPpiGuid;
  Instance->PpiList[0].Ppi = &Instance->TimestampPpi;

  Instance->Stall2Ppi.Stall2 = PeiStall2;

  Instance->PpiList[1].Flags = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  Instance->PpiList[1].Guid = &gEdkiiPeiStall2PpiGuid;
  Instance->PpiList[1].Ppi = &Instance->Stall2Ppi;

  Status = PeiServicesInstallPpi (Instance->PpiList);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
