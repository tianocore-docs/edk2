/** @file
  This file declares EDKII Stall2 protocol.

  This protocol induces a fine-grained stall for DXE environment.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_STALL2_PROTOCOL_H__
#define __EDKII_STALL2_PROTOCOL_H__

#define EDKII_STALL2_PROTOCOL_GUID \
  { 0x2c40927, 0x6414, 0x442b, { 0xa1, 0x1, 0xf1, 0x40, 0xfc, 0x25, 0xf5, 0x51 } }

typedef struct _EDKII_STALL2_PROTOCOL EDKII_STALL2_PROTOCOL;

/**
  Induces a fine-grained stall.

  @param[in] Nanoseconds        The number of nanoseconds to stall execution.

  @retval EFI_SUCCESS           Execution was stalled at least the requested number of
                                Nanoseconds.

**/
typedef
EFI_STATUS
(EFIAPI *EDKII_STALL2)(
  IN UINTN                      Nanoseconds
  );

///
/// This service induces a fine-grained stall.
///
struct _EDKII_STALL2_PROTOCOL {
  EDKII_STALL2  Stall2;
};

extern EFI_GUID gEdkiiStall2ProtocolGuid;

#endif
