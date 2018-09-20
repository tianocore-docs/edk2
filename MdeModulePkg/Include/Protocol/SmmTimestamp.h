/** @file
  EDKII SMM Timestamp Protocol is used to provide a platform independent interface
  for retrieving a high resolution timestamp counter.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_SMM_TIME_STAMP_PROTOCOL_H__
#define __EDKII_SMM_TIME_STAMP_PROTOCOL_H__

#include <Protocol/Timestamp.h>

#define EDKII_SMM_TIMESTAMP_PROTOCOL_GUID \
  { 0xe203a636, 0x2d1e, 0x4456, { 0x90, 0xf1, 0xb8, 0x27, 0xb4, 0x42, 0xb8, 0xb3 } }

///
/// Declare forward reference for the Time Stamp Protocol
///
typedef struct _EDKII_SMM_TIMESTAMP_PROTOCOL  EDKII_SMM_TIMESTAMP_PROTOCOL;

///
/// EDKII_SMM_TIMESTAMP_PROTOCOL
/// The protocol provides a platform independent interface
/// for retrieving a high resolution timestamp counter.
///
struct _EDKII_SMM_TIMESTAMP_PROTOCOL {
  TIMESTAMP_GET                 SmmGetTimestamp;
  TIMESTAMP_GET_PROPERTIES      SmmGetProperties;
};

extern EFI_GUID gEdkiiSmmTimestampProtocolGuid;

#endif

