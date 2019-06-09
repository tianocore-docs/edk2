/** @file
  EDKII System Capsule library.

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#ifndef __MEMORY_RANGE_CAPSULE_LIB_H__
#define __MEMORY_RANGE_CAPSULE_LIB_H__

#include <Guid/MemoryRangeCapsule.h>

/**
  This API will find a subset of candidate memory block, excluded in MemoryRangeCapsule.
  
  @param[in, out] MemoryBase          Pointer to the base of a block of candidate memory.
                                      On output, it holds the base address of the BIOS usable memory inside of input candidata memory block.
  @param[in, out] MemorySize          Size of the candidate memory region.
                                      On output, it holds the size of the BIOS usable memory inside of input candidata memory block.
  @param[in]      MemoryRangeCapsule  A capsule image describe a list of scatter gather range which is reserved by OS.

  @return Status
**/
EFI_STATUS
EFIAPI
GetFreeMemoryFromMemoryRange (
  IN OUT VOID                          **MemoryBase,
  IN OUT UINTN                         *MemorySize,
  IN EFI_MEMORY_RANGE_CAPSULE          *MemoryRangeCapsule
  );

#endif

