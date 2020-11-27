/** @file
  Root include file to support building Contiki.

Copyright (c) 2014, Intel Corporation. All rights reserved.
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __UEFI_CONTIKI_SUPPORT_H
#define __UEFI_CONTIKI_SUPPORT_H

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#define CHAR_BIT 8

#define offsetof(type, member) ( (int) & ((type*)0) -> member )

typedef UINTN  size_t;
typedef INTN   ssize_t;

typedef INT8   int8_t;
typedef INT16  int16_t;
typedef INT32  int32_t;
typedef INT64  int64_t;

typedef UINT8  uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;

int            tolower     (int);
int            strncasecmp (const char *, const char *, size_t);

#define memcpy(dest,source,count)         CopyMem(dest,source,(UINTN)(count))
#define memset(dest,ch,count)             SetMem(dest,(UINTN)(count),(UINT8)(ch))
#define memcmp(buf1,buf2,count)           (int)(CompareMem(buf1,buf2,(UINTN)(count)))
#define memmove(dest,source,count)        CopyMem(dest,source,(UINTN)(count))

#define strncpy(strDest,strSource,count)  AsciiStrnCpy(strDest,strSource,(UINTN)count)
#define strncat(strDest,strSource,count)  AsciiStrnCat(strDest,strSource,(UINTN)count)
#define strcasecmp(strFirst, strSecond)   AsciiStriCmp(strFirst, strSecond)

void srand(unsigned seed);
int rand(void);

int
printf (
  IN CONST CHAR8  *Format,
  ...
  );

#endif
