/** @file
  Data structure and functions to allocate and free memory space.

  Copyright(c) 2014 Intel Corporation. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.
  * Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#ifndef _IMEM_H_
#define _IMEM_H_

#if defined (MDE_CPU_IPF)
///
/// For Itanium machines make the default allocations 8K aligned
///
#define EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT  (EFI_PAGE_SIZE * 2)
#define DEFAULT_PAGE_ALLOCATION                     (EFI_PAGE_SIZE * 2)

#else
///
/// For genric EFI machines make the default allocations 4K aligned
///
#define EFI_ACPI_RUNTIME_PAGE_ALLOCATION_ALIGNMENT  (EFI_PAGE_SIZE)
#define DEFAULT_PAGE_ALLOCATION                     (EFI_PAGE_SIZE)

#endif


//
// MEMORY_MAP_ENTRY
//

#define MEMORY_MAP_SIGNATURE   SIGNATURE_32('m','m','a','p')
typedef struct {
  UINTN           Signature;
  LIST_ENTRY      Link;
  BOOLEAN         FromPages;

  EFI_MEMORY_TYPE Type;
  UINT64          Start;
  UINT64          End;

  UINT64          VirtualStart;
  UINT64          Attribute;
} MEMORY_MAP;

//
// Internal prototypes
//


/**
  Internal function.  Used by the pool functions to allocate pages
  to back pool allocation requests.

  @param  PoolType               The type of memory for the new pool pages
  @param  NumberOfPages          No of pages to allocate
  @param  Alignment              Bits to align.

  @return The allocated memory, or NULL

**/
VOID *
CoreAllocatePoolPages (
  IN EFI_MEMORY_TYPE    PoolType,
  IN UINTN              NumberOfPages,
  IN UINTN              Alignment
  );



/**
  Internal function.  Frees pool pages allocated via AllocatePoolPages ()

  @param  Memory                 The base address to free
  @param  NumberOfPages          The number of pages to free

**/
VOID
CoreFreePoolPages (
  IN EFI_PHYSICAL_ADDRESS   Memory,
  IN UINTN                  NumberOfPages
  );



/**
  Internal function to allocate pool of a particular type.
  Caller must have the memory lock held

  @param  PoolType               Type of pool to allocate
  @param  Size                   The amount of pool to allocate

  @return The allocate pool, or NULL

**/
VOID *
CoreAllocatePoolI (
  IN EFI_MEMORY_TYPE  PoolType,
  IN UINTN            Size
  );



/**
  Internal function to free a pool entry.
  Caller must have the memory lock held

  @param  Buffer                 The allocated pool entry to free

  @retval EFI_INVALID_PARAMETER  Buffer not valid
  @retval EFI_SUCCESS            Buffer successfully freed.

**/
EFI_STATUS
CoreFreePoolI (
  IN VOID       *Buffer
  );



/**
  Enter critical section by gaining lock on gMemoryLock.

**/
VOID
CoreAcquireMemoryLock (
  VOID
  );


/**
  Exit critical section by releasing lock on gMemoryLock.

**/
VOID
CoreReleaseMemoryLock (
  VOID
  );


//
// Internal Global data
//

extern EFI_LOCK           gMemoryLock;
extern LIST_ENTRY         gMemoryMap;
extern LIST_ENTRY         mGcdMemorySpaceMap;
#endif
