/** @file
  Base Debug library instance base on Serial Port library.
  It uses PrintLib to send debug messages to serial port device.
  
  NOTE: If the Serial Port library enables hardware flow control, then a call 
  to DebugPrint() or DebugAssert() may hang if writes to the serial port are 
  being blocked.  This may occur if a key(s) are pressed in a terminal emulator
  used to monitor the DEBUG() and ASSERT() messages. 

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

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SerialPortLib.h>
#include <Library/DebugPrintErrorLevelLib.h>

//
// Define the maximum debug and assert message length that this library supports 
//
#define MAX_DEBUG_MESSAGE_LENGTH  0x100

/**
  The constructor function initialize the Serial Port Library

  @retval EFI_SUCCESS   The constructor always returns RETURN_SUCCESS.

**/
RETURN_STATUS
EFIAPI
SimpleDebugLibSerialPortConstructor (
  VOID
  )
{
  return SerialPortInitialize ();
}

VOID
PrintC (
  IN CHAR8 Char
  )
{
  SerialPortWrite (&Char, 1);
}

VOID
PrintStr (
  IN CHAR8 *Str
  )
{
  while (*Str) {
    PrintC (*Str++);
  }
}

VOID
PrintUniStr (
  IN CHAR16 *Str
  )
{
  while (*Str) {
    PrintC ((CHAR8)*Str++);
  }
}

VOID
PrintHex64 (
  IN UINT64 Val,
  IN UINT32 Width
  )
{
  UINT32   Index;
  UINT8    Char;
  UINT8    Empty = 1;

  for (Index = sizeof(UINT64) * 2; Index > 0; Index--) {
    Char = (UINT8) ((Val >> (sizeof(UINT64) * 8 - 4)) & 0xF);
    if (Char > 9) {
      Char += 'A' - 10;
    } else {
      Char += '0';
    }

    Val = Val << 4;

    if (Char != '0') {
      Empty = 0;
    }

    if (!Empty || Index <= Width) {
      PrintC (Char);
    }
  }
}

VOID
PrintHex (
  IN UINTN  Val,
  IN UINT32 Width
  )
{
  UINT32  Index;
  UINT8   Char;
  UINT8   Empty = 1;

  for (Index = sizeof(UINTN) * 2; Index > 0; Index--) {
    Char = (UINT8) ((Val >> (sizeof(UINTN) * 8 - 4)) & 0xF);
    if (Char > 9) {
      Char += 'A' - 10;
    } else {
      Char += '0';
    }

    Val = Val << 4;

    if (Char != '0') {
      Empty = 0;
    }

    if (!Empty || Index <= Width) {
      PrintC (Char);
    }
  }
}

VOID
PrintDec (
  IN UINTN  Val,
  IN UINT32 Width
  )
{
  UINT32  Index;
  UINT8   Char;
  UINT8   Empty = 1;
  UINT8   Buf[20];

  for (Index = 0; Index < sizeof(Buf); Index++) {
    Char = (UINT8) (Val % 10);
    Buf[Index] = Char + '0';
    Val = Val / 10;
  }

  while (Index > 0) {
    Char = Buf[--Index];

    if (Char != '0') {
      Empty = 0;
    }

    if (!Empty || Index < Width) {
      PrintC (Char);
    }
  }
}

VOID
PrintGuid (
  IN GUID *Guid
  )
{
  UINTN  Index;
  PrintHex (Guid->Data1, 8);
  PrintC ('-');
  PrintHex (Guid->Data2, 4);
  PrintC ('-');
  PrintHex (Guid->Data3, 4);
  PrintC ('-');
  for (Index = 0; Index < sizeof(Guid->Data4); Index++) {
    if (Index == 2) {
      PrintC ('-');
    }
    PrintHex (Guid->Data4[Index], 2);
  }
}

CONST CHAR8 *
GetWidth (
  IN CONST CHAR8   *Format,
  OUT UINT32       *Width
  )
{
  UINT32 Val = 0;

  while (*Format >= '0' && *Format <= '9') {
    Val = Val * 10 + *Format - '0';
    Format += 1;
  }

  if (Val > 0) {
    *Width = Val;
  }
  return Format;
}

// input fmt
// ----- ---
//  s   -> s
//  a   -> a
//  d   -> d
//  X   -> X
//  p   -> X
//  r   -> X
//  llX -> L
//  g   -> g
CONST CHAR8 *
GetFormat (
  IN CONST CHAR8  *Format,
  OUT UINT8       *Fmt
  )
{
  if (Format[0] == 's' || Format[0] == 'S') {
    Format += 1;
    *Fmt = 's';
  } else if (Format[0] == 'a') {
    Format += 1;
    *Fmt = 'a';
  } else if (Format[0] == 'd') {
    Format += 1;
    *Fmt = 'd';
  } else if (Format[0] == 'X' || Format[0] == 'x' || Format[0] == 'p' || Format[0] == 'r') {
    Format += 1;
    *Fmt = 'X';
  } else if (Format[0] == 'l' && (Format[1] == 'X' || Format[1] == 'x')) {
    Format += 2;
    *Fmt = 'L';
  } else if (Format[0] == 'g') {
    Format += 1;
    *Fmt = 'g';
  }

  return Format;
}

/**
  Prints a debug message to the debug output device if the specified error level is enabled.

  If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function 
  GetDebugPrintErrorLevel (), then print the message specified by Format and the 
  associated variable argument list to the debug output device.

  If Format is NULL, then ASSERT().

  @param  ErrorLevel  The error level of the debug message.
  @param  Format      Format string for the debug message to print.
  @param  ...         Variable argument list whose contents are accessed 
                      based on the format string specified by Format.

**/
VOID
EFIAPI
DebugPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  ...
  )
{
  UINTN    *Arg = (UINTN *)(&Format + 1);

  //
  // If Format is NULL, then ASSERT().
  //
  ASSERT (Format != NULL);

  //
  // Check driver debug mask value and global mask
  //
  if ((ErrorLevel & GetDebugPrintErrorLevel ()) == 0) {
    return;
  }

  //
  // Convert the DEBUG() message to an ASCII String
  //
  for (;;) {
    UINT8    Char = *Format++;
    if (Char == 0) {
      break;
    }

    if (Char == '\n') {
      PrintC ('\r');
      PrintC ('\n');
    } else if (Char == '%') {
      UINT8    Fmt = 0;
      UINT32   Width = 1;

      Format = GetWidth (Format, &Width);
      Format = GetFormat (Format, &Fmt);

      // Print value
      if (Fmt == 'd') {
        PrintDec (*Arg, Width);
        Arg += 1;
      } else if (Fmt == 'X') {
        PrintHex (*Arg, Width);
        Arg += 1;
      } else if (Fmt == 'L') {
        PrintHex64 (*(UINT64 *)Arg, Width);
        Arg += sizeof(UINT64)/sizeof(UINTN);
      } else if (Fmt == 's') {
        PrintUniStr (*(CHAR16 **)Arg);
        Arg += 1;
      } else if (Fmt == 'a') {
        PrintStr (*(CHAR8 **)Arg);
        Arg += 1;
      } else if (Fmt == 'g') {
        PrintGuid (*(GUID **)Arg);
        Arg += 1;
      }
    } else {
      PrintC (Char);
    }
  }
}


/**
  Prints an assert message containing a filename, line number, and description.  
  This may be followed by a breakpoint or a dead loop.

  Print a message of the form "ASSERT <FileName>(<LineNumber>): <Description>\n"
  to the debug output device.  If DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED bit of 
  PcdDebugProperyMask is set then CpuBreakpoint() is called. Otherwise, if 
  DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED bit of PcdDebugProperyMask is set then 
  CpuDeadLoop() is called.  If neither of these bits are set, then this function 
  returns immediately after the message is printed to the debug output device.
  DebugAssert() must actively prevent recursion.  If DebugAssert() is called while
  processing another DebugAssert(), then DebugAssert() must return immediately.

  If FileName is NULL, then a <FileName> string of "(NULL) Filename" is printed.
  If Description is NULL, then a <Description> string of "(NULL) Description" is printed.

  @param  FileName     The pointer to the name of the source file that generated the assert condition.
  @param  LineNumber   The line number in the source file that generated the assert condition
  @param  Description  The pointer to the description of the assert condition.

**/
VOID
EFIAPI
DebugAssert (
  IN CONST CHAR8  *FileName,
  IN UINTN        LineNumber,
  IN CONST CHAR8  *Description
  )
{
#if 1
  PrintStr ("ASSERT ");
  PrintStr ((CHAR8 *)FileName);
  PrintStr ("(");
  PrintDec (LineNumber, 10);
  PrintStr ("): ");
  PrintStr ((CHAR8 *)Description);
  PrintStr ("\n");
  //
  // Generate a Breakpoint, DeadLoop, or NOP based on PCD settings
  //
  if ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED) != 0) {
    CpuBreakpoint ();
  } else if ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED) != 0) {
    CpuDeadLoop ();
  }
#else
  PrintStr ("ASSERT\r\n");
  CpuDeadLoop ();
#endif
}


/**
  Fills a target buffer with PcdDebugClearMemoryValue, and returns the target buffer.

  This function fills Length bytes of Buffer with the value specified by 
  PcdDebugClearMemoryValue, and returns Buffer.

  If Buffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT(). 

  @param   Buffer  The pointer to the target buffer to be filled with PcdDebugClearMemoryValue.
  @param   Length  The number of bytes in Buffer to fill with zeros PcdDebugClearMemoryValue. 

  @return  Buffer  The pointer to the target buffer filled with PcdDebugClearMemoryValue.

**/
VOID *
EFIAPI
DebugClearMemory (
  OUT VOID  *Buffer,
  IN UINTN  Length
  )
{
  return Buffer;
}


/**
  Returns TRUE if ASSERT() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of 
  PcdDebugProperyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugProperyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugProperyMask is clear.

**/
BOOLEAN
EFIAPI
DebugAssertEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED) != 0);
}


/**  
  Returns TRUE if DEBUG() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of 
  PcdDebugProperyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of PcdDebugProperyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of PcdDebugProperyMask is clear.

**/
BOOLEAN
EFIAPI
DebugPrintEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_PRINT_ENABLED) != 0);
}


/**  
  Returns TRUE if DEBUG_CODE() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of 
  PcdDebugProperyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of PcdDebugProperyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of PcdDebugProperyMask is clear.

**/
BOOLEAN
EFIAPI
DebugCodeEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_CODE_ENABLED) != 0);
}


/**  
  Returns TRUE if DEBUG_CLEAR_MEMORY() macro is enabled.

  This function returns TRUE if the DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of 
  PcdDebugProperyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of PcdDebugProperyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of PcdDebugProperyMask is clear.

**/
BOOLEAN
EFIAPI
DebugClearMemoryEnabled (
  VOID
  )
{
  return FALSE;
}
