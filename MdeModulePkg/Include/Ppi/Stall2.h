/** @file
  This file declares EDKII Stall2 PPI.

  This ppi abstracts the blocking stall service to other agents.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_STALL2_PPI_H__
#define __EDKII_STALL2_PPI_H__

#define EDKII_PEI_STALL2_PPI_GUID \
  { 0x6c82f6d7, 0x100e, 0x4436, { 0xba, 0x5e, 0xec, 0x3, 0xe6, 0xe6, 0x17, 0x80 } }

typedef struct _EDKII_PEI_STALL2_PPI EDKII_PEI_STALL2_PPI;

/**
  The Stall2() function provides a blocking stall for at least the number
  of nanoseconds stipulated in the final argument of the API.

  @param[in] This           Pointer to the local data for the interface.
  @param[in] Nanoseconds    Number of nanoseconds for which to stall.

  @retval EFI_SUCCESS   The service provided at least the required delay.

**/
typedef
EFI_STATUS
(EFIAPI *EDKII_PEI_STALL2)(
  IN CONST EDKII_PEI_STALL2_PPI     *This,
  IN UINTN                          Nanoseconds
  );

///
/// This service provides a simple, blocking stall with platform-specific resolution.
///
struct _EDKII_PEI_STALL2_PPI {
  EDKII_PEI_STALL2  Stall2;
};

extern EFI_GUID gEdkiiPeiStall2PpiGuid;

#endif
