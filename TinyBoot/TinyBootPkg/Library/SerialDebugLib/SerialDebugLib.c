/** @file

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

--*/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/SerialPortLib.h>

VOID
DebugWriteChar (
  IN CHAR8  Char
  )
{
  SerialPortWrite (&Char, 1);
}

VOID
DebugWriteString (
  IN CHAR8  *String
  )
{
  UINTN Len;

  Len = AsciiStrLen (String);
  if (Len == 0) {
    return ;
  }
  SerialPortWrite (String, Len - 1);
  if (String[Len - 1] == '\n') {
    DebugWriteChar ('\r');
  }
  DebugWriteChar (String[Len - 1]);
}

 GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8 mHexConvert[] = "0123456789abcdef";

VOID
DebugWriteUint8 (
  IN UINT8   Data
  )
{
  DebugWriteChar (mHexConvert[Data >> 4]);
  DebugWriteChar (mHexConvert[Data & 0xF]);
}

VOID
DebugWriteUint16 (
  IN UINT16  Data
  )
{
  DebugWriteUint8 ((UINT8)(Data >> 8));
  DebugWriteUint8 ((UINT8)(Data));
}

VOID
DebugWriteUint32 (
  IN UINT32  Data
  )
{
  DebugWriteUint16 ((UINT16)(Data >> 16));
  DebugWriteUint16 ((UINT16)(Data));
}

VOID
DebugWriteUint64 (
  IN UINT64  Data
  )
{
  DebugWriteUint32 ((UINT32)(Data >> 32));
  DebugWriteUint32 ((UINT32)(Data));
}
