/** @file

Copyright (c) 2014, Intel Corporation. All rights reserved.
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

/* Sets buffers to a specified character */
void * memset (void *dest, char ch, unsigned int count)
{
  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memset()
  //
  volatile UINT8  *Pointer;

  Pointer = (UINT8 *)dest;
  while (count-- != 0) {
    *(Pointer++) = ch;
  }
  
  return dest;
}

/* Copies bytes between buffers */
void * memcpy (void *dest, const void *src, unsigned int count)
{
  return CopyMem (dest, src, (UINTN)count);
}

/* Copies bytes between buffers */
void * memmove (void *dest, const void *src, unsigned int count)
{
  return CopyMem (dest, src, (UINTN)count);
}

int memcmp(const void* s1, const void* s2, unsigned int n)
{
  return (int)CompareMem (s1, s2, (UINTN)n);
}

char * strcpy(char* s1, const char* s2)
{
  return AsciiStrCpy (s1, s2);
}

char * strcat(char* s1, const char* s2)
{
  return AsciiStrCat (s1, s2);
}
unsigned int strlen(const char* s)
{
  return (unsigned int)AsciiStrLen (s);
}
