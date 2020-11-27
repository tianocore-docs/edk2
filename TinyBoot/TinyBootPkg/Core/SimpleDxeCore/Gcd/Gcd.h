/** @file
  GCD Operations and data structure used to
  convert from GCD attributes to EFI Memory Map attributes.

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

#ifndef _GCD_H_
#define _GCD_H_

//
// GCD Operations
//
#define GCD_MEMORY_SPACE_OPERATION 0x20
#define GCD_IO_SPACE_OPERATION     0x40

#define GCD_ADD_MEMORY_OPERATION             (GCD_MEMORY_SPACE_OPERATION | 0)
#define GCD_ALLOCATE_MEMORY_OPERATION        (GCD_MEMORY_SPACE_OPERATION | 1)
#define GCD_FREE_MEMORY_OPERATION            (GCD_MEMORY_SPACE_OPERATION | 2)
#define GCD_REMOVE_MEMORY_OPERATION          (GCD_MEMORY_SPACE_OPERATION | 3)
#define GCD_SET_ATTRIBUTES_MEMORY_OPERATION  (GCD_MEMORY_SPACE_OPERATION | 4)

#define GCD_ADD_IO_OPERATION                 (GCD_IO_SPACE_OPERATION | 0)
#define GCD_ALLOCATE_IO_OPERATION            (GCD_IO_SPACE_OPERATION | 1)
#define GCD_FREE_IO_OPERATION                (GCD_IO_SPACE_OPERATION | 2)
#define GCD_REMOVE_IO_OPERATION              (GCD_IO_SPACE_OPERATION | 3)

//
// The data structure used to convert from GCD attributes to EFI Memory Map attributes
//
typedef struct {
  UINT64   Attribute;
  UINT64   Capability;
  BOOLEAN  Memory;
} GCD_ATTRIBUTE_CONVERSION_ENTRY;

#endif
