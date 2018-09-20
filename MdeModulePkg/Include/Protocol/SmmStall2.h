/** @file
  This file declares EDKII SMM Stall2 protocol.

  This protocol induces a fine-grained stall for SMM environment.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_SMM_STALL2_PROTOCOL_H__
#define __EDKII_SMM_STALL2_PROTOCOL_H__

#include <Protocol/Stall2.h>

#define EDKII_SMM_STALL2_PROTOCOL_GUID \
  { 0x93661a82, 0xa557, 0x4c24, { 0x80, 0x17, 0x9, 0x92, 0x20, 0xa5, 0xa9, 0x69 } }

typedef struct _EDKII_SMM_STALL2_PROTOCOL EDKII_SMM_STALL2_PROTOCOL;

///
/// This service induces a fine-grained stall.
///
struct _EDKII_SMM_STALL2_PROTOCOL {
  EDKII_STALL2  SmmStall2;
};

extern EFI_GUID gEdkiiSmmStall2ProtocolGuid;

#endif
