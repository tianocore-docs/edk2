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

#include <UefiContikiSupport.h>

#include <Library/PrintLib.h>
#include <Library/DebugLib.h>

#define MAX_DEBUG_MESSAGE_LENGTH  0x100

CHAR8 gLastChar = '\n';
extern UINT8                  mMyNodeId;

int
printf (
  IN CONST CHAR8  *Format,
  ...
  )
{
  CHAR8    Buffer[MAX_DEBUG_MESSAGE_LENGTH];
  VA_LIST  Marker;

  VA_START(Marker, Format);
  AsciiVSPrint (Buffer, sizeof(Buffer), Format, Marker);
  VA_END(Marker);

  if (gLastChar == '\n') {
    DebugPrint (DEBUG_ERROR, "[Node%d] ", mMyNodeId);
  }
  DebugPrint (DEBUG_ERROR, "%a", Buffer);

  {
    UINTN  Len = AsciiStrLen (Format);
    if (Len > 0) {
      gLastChar = Format[Len - 1];
    }
  }

  return 0;
}

////////////////////////////////////////////////////
int tolower (int c)
{
  if (('A' <= (c)) && ((c) <= 'Z')) {
    return (c - ('A' - 'a'));
  }
  return (c);
}

int strncasecmp (const char *c, const char *s, size_t l)
{
  char  uc;
  char  us;

  if (l == 0) {
    return 0;
  }

  uc  = tolower (*c);
  us = tolower (*s);
  while ((*c != '\0') && (uc == us) && (l > 1)) {
    c++;
    s++;
    uc  = tolower (*c);
    us = tolower (*s);
    l--;
  }

  return uc - us;
}
