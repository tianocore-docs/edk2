/** @file
  EDKII Timestamp PPI is used to provide a platform independent interface
  for retrieving a high resolution timestamp counter.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_TIME_STAMP_PPI_H__
#define __EDKII_TIME_STAMP_PPI_H__

#include <Protocol/Timestamp.h>

#define EDKII_PEI_TIMESTAMP_PPI_GUID \
  { 0x60e3a81a, 0xf1e8, 0x48aa, { 0x93, 0xce, 0xff, 0xc4, 0xac, 0x7a, 0xa6, 0x3 } }

///
/// Declare forward reference for the EDKII Time Stamp PPI
///
typedef struct _EDKII_PEI_TIMESTAMP_PPI EDKII_PEI_TIMESTAMP_PPI;

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
typedef
UINT64
(EFIAPI *EDKII_PEI_TIMESTAMP_GET)(
  IN CONST EDKII_PEI_TIMESTAMP_PPI      *This
  );

/**
  Obtains timestamp counter properties including frequency and value limits.

  @param[in]  This          Pointer to the local data for the interface.
  @param[out] Properties    The properties of the timestamp counter.

  @retval EFI_SUCCESS               The properties were successfully retrieved.
  @retval EFI_DEVICE_ERROR          An error occurred trying to retrieve the properties of the timestamp
                                    counter subsystem. Properties is not pedated.
  @retval EFI_INVALID_PARAMETER     Properties is NULL.

**/
typedef
EFI_STATUS
(EFIAPI *EDKII_PEI_TIMESTAMP_GET_PROPERTIES)(
  IN CONST EDKII_PEI_TIMESTAMP_PPI      *This,
  OUT EFI_TIMESTAMP_PROPERTIES          *Properties
  );

///
/// EDKII_PEI_TIMESTAMP_PPI
/// The PPI provides a platform independent interface for
/// retrieving a high resolution timestamp counter.
///
struct _EDKII_PEI_TIMESTAMP_PPI {
  EDKII_PEI_TIMESTAMP_GET               GetTimestamp;
  EDKII_PEI_TIMESTAMP_GET_PROPERTIES    GetProperties;
};

extern EFI_GUID gEdkiiPeiTimestampPpiGuid;

#endif

