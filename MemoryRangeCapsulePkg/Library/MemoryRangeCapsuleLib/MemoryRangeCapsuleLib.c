/** @file

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryRangeCapsuleLib.h>

BOOLEAN
InternalIsMemoryRangeOverlap (
  IN OUT VOID                          **MemoryBase,
  IN OUT UINTN                         *MemorySize,
  IN EFI_PHYSICAL_ADDRESS              Address,
  IN UINT64                            Length
  )
{
  UINTN       Temp;
  VOID        *CandidateBase1;
  UINTN       CandidateSize1;
  VOID        *CandidateBase2;
  UINTN       CandidateSize2;

  if (((UINTN)*MemoryBase >= Address) &&
      ((UINTN)*MemoryBase < Address + Length)) {
    if (Address + Length >= (UINTN)*MemoryBase + *MemorySize) {
      //
      // Addr: |-------------|
      // Mem:     |-------|
      //
      *MemoryBase = NULL;
      *MemorySize = 0;
      return FALSE;
    } else {
      //
      // Addr: |-------------|
      // Mem:     |----------------|
      //
      Temp = (UINTN)*MemoryBase;
      *MemoryBase = (VOID *)(UINTN)(Address + Length);
      *MemorySize = Temp + *MemorySize - (UINTN)*MemoryBase;
      return TRUE;
    }
  }
  if ((Address >= (UINTN)*MemoryBase) &&
      (Address < (UINTN)*MemoryBase + *MemorySize)) {
    if (Address + Length >= (UINTN)*MemoryBase + *MemorySize) {
      //
      // Addr:       |-------------|
      // Mem:     |-------|
      //
      *MemorySize = (UINTN)(Address - (UINTN)*MemoryBase);
      if (*MemorySize == 0) {
        return FALSE;
      } else {
        return TRUE;
      }
    } else {
      //
      // Addr:       |-------------|
      // Mem:     |----------------------|
      //
      CandidateBase1 = *MemoryBase;
      CandidateSize1 = (UINTN)(Address - (UINTN)*MemoryBase);
      CandidateBase2 = (VOID *)(UINTN)(Address + Length);
      CandidateSize2 = (UINTN)((UINTN)*MemoryBase + *MemorySize - (Address + Length));

      if (CandidateSize2 > CandidateSize1) {
        *MemoryBase = (VOID *)CandidateBase2;
        *MemorySize = CandidateSize2;
      } else {
        *MemoryBase = (VOID *)CandidateBase1;
        *MemorySize = CandidateSize1;
      }
      if (*MemorySize == 0) {
        return FALSE;
      } else {
        return TRUE;
      }
    }
  }
  return FALSE;
}

BOOLEAN
InternalIsMemoryRangeCapsuleOverlap (
  IN OUT VOID                          **MemoryBase,
  IN OUT UINTN                         *MemorySize,
  IN EFI_MEMORY_RANGE_CAPSULE          *MemoryRangeCapsule
  )
{
  UINTN       Index;
  UINTN       Pages;

  if (*MemorySize == 0) {
    return FALSE;
  }

  for (Index = 0; Index < MemoryRangeCapsule->NumberOfMemoryRanges; Index++) {
    if (InternalIsMemoryRangeOverlap (MemoryBase, MemorySize, MemoryRangeCapsule->MemoryRanges[Index].Address, MemoryRangeCapsule->MemoryRanges[Index].Length)) {
      return TRUE;
    }
    if (*MemorySize == 0) {
      return FALSE;
    }
  }
  //
  // Last check 
  //
  if (MemoryRangeCapsule->MemoryForFirmwareUse.Address != 0 && MemoryRangeCapsule->MemoryForFirmwareUse.Length != 0) {
    Pages = EFI_SIZE_TO_PAGES(MemoryRangeCapsule->Header.CapsuleImageSize);
    return InternalIsMemoryRangeOverlap (
             MemoryBase,
             MemorySize,
             MemoryRangeCapsule->MemoryForFirmwareUse.Address,
             EFI_PAGES_TO_SIZE(Pages)
             );
  }
  return FALSE;
}

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
  )
{
  BOOLEAN                            IsOverlap;

  ASSERT (MemoryRangeCapsule != NULL);
  ASSERT(CompareGuid (&MemoryRangeCapsule->Header.CapsuleGuid, &gEfiMemoryRangeCapsuleGuid));

  //
  // Make sure MemoryRangeCapsule is valid data.
  //
  if (MemoryRangeCapsule->MemoryForFirmwareUse.Address != 0 && MemoryRangeCapsule->MemoryForFirmwareUse.Length != 0) {
    MemoryRangeCapsule = (VOID *)(UINTN)MemoryRangeCapsule->MemoryForFirmwareUse.Address;
  }

  //
  // Get candidate.
  //
  while (TRUE) {
    DEBUG ((DEBUG_INFO, "Candidate 0x%x-0x%x\n", *MemoryBase, *MemorySize));
    IsOverlap = InternalIsMemoryRangeCapsuleOverlap (MemoryBase, MemorySize, MemoryRangeCapsule);
    if (*MemorySize == 0) {
      return EFI_NOT_FOUND;
    }
    if (!IsOverlap) {
      //
      // Good get address.
      //
      DEBUG ((DEBUG_INFO, "FinalCandidate 0x%x-0x%x\n", *MemoryBase, *MemorySize));
      return EFI_SUCCESS;
    }
  }
}
