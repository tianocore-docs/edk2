/** @file

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#ifndef __EFI_MEMORY_RANGE_CAPSULE_GUID_H__
#define __EFI_MEMORY_RANGE_CAPSULE_GUID_H__

#include <Uefi.h>

#define EFI_MEMORY_RANGE_CAPSULE_GUID  { \
  0xe8143950, 0x8c5c, 0x410e, 0x86, 0xf, 0x12, 0x63, 0x64, 0xd3, 0xc6, 0xc9 \
}

#define EFI_MEMORY_RANGE_REQUIREMENT_DATA_GUID  { \
  0xf7d8603, 0xa812, 0x4210, 0x86, 0x1b, 0x1b, 0x32, 0x4, 0xde, 0x56, 0x5b \
}

#pragma pack(1)

#define EFI_MEMORY_RANGE_CAPSULE_FLAG (CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE)

typedef struct {
  EFI_PHYSICAL_ADDRESS   Address;
  UINT64                 Length;
} EFI_MEMORY_RANGE;

typedef struct {
  EFI_CAPSULE_HEADER        Header;
  EFI_MEMORY_TYPE           OsRequestedMemoryType;
  //
  // This optional field declares the free memory firmware location
  // so that firmware can use this memory to parse MemoryRanges.
  //
  // It could be zero, if the whole capsule image is small. (e.g. < 4K)
  // It must be non-zero, if the whole capsule image is big. (e.g. > 4K)
  //
  EFI_MEMORY_RANGE          MemoryForFirmwareUse;
  UINT64                    NumberOfMemoryRanges;
  EFI_MEMORY_RANGE          MemoryRanges[];
} EFI_MEMORY_RANGE_CAPSULE;

typedef struct {
  UINT64                    FirmwareMemoryRequirement;
} EFI_MEMORY_RANGE_REQUIREMENT_DATA;

#pragma pack()

extern EFI_GUID gEfiMemoryRangeCapsuleGuid;
extern EFI_GUID gEfiMemoryRangeRequirementDataGuid;

#endif
