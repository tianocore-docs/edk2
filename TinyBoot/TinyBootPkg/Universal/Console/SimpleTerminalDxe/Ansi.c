/** @file
  Implementation of translation upon PC ANSI.

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


#include "Terminal.h"

/**
  Translate all raw data in the Raw FIFO into unicode, and insert
  them into Unicode FIFO.

  @param TerminalDevice          The terminal device.

**/
VOID
AnsiRawDataToUnicode (
  IN  TERMINAL_DEV    *TerminalDevice
  )
{
  UINT8 RawData;

  //
  // pop the raw data out from the raw fifo,
  // and translate it into unicode, then push
  // the unicode into unicode fifo, until the raw fifo is empty.
  //
  while (!IsRawFiFoEmpty (TerminalDevice) && !IsUnicodeFiFoFull (TerminalDevice)) {

    RawFiFoRemoveOneKey (TerminalDevice, &RawData);

    UnicodeFiFoInsertOneKey (TerminalDevice, (UINT16) RawData);
  }
}

/**
  Check if input string is valid Ascii string, valid EFI control characters
  or valid text graphics.

  @param  TerminalDevice          The terminal device.
  @param  WString                 The input string.          
 
  @retval EFI_UNSUPPORTED         If not all input characters are valid.
  @retval EFI_SUCCESS             If all input characters are valid.

**/
EFI_STATUS
AnsiTestString (
  IN  TERMINAL_DEV    *TerminalDevice,
  IN  CHAR16          *WString
  )
{
  CHAR8 GraphicChar;

  //
  // support three kind of character:
  // valid ascii, valid efi control char, valid text graphics.
  //
  for (; *WString != CHAR_NULL; WString++) {

    if ( !(TerminalIsValidAscii (*WString) ||
        TerminalIsValidEfiCntlChar (*WString) ||
        TerminalIsValidTextGraphics (*WString, &GraphicChar, NULL) )) {

      return EFI_UNSUPPORTED;
    }
  }

  return EFI_SUCCESS;
}
