/** @file
  Implementation for EFI_SIMPLE_TEXT_INPUT_PROTOCOL protocol.

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
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existence of a keystroke via WaitForEvent () call.

  @param  TerminalDevice           Terminal driver private structure
  @param  KeyData                  A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   pressed.

  @retval EFI_SUCCESS              The keystroke information was returned.
  @retval EFI_NOT_READY            There was no keystroke data available.
  @retval EFI_INVALID_PARAMETER    KeyData is NULL.

**/
EFI_STATUS
ReadKeyStrokeWorker (
  IN  TERMINAL_DEV *TerminalDevice,
  OUT EFI_KEY_DATA *KeyData
  )
{
  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!EfiKeyFiFoRemoveOneKey (TerminalDevice, &KeyData->Key)) {
    return EFI_NOT_READY;
  }

  KeyData->KeyState.KeyShiftState  = 0;
  KeyData->KeyState.KeyToggleState = 0;


  return EFI_SUCCESS;

}

/**
  Implements EFI_SIMPLE_TEXT_INPUT_PROTOCOL.Reset().
  This driver only perform dependent serial device reset regardless of
  the value of ExtendeVerification

  @param  This                     Indicates the calling context.
  @param  ExtendedVerification     Skip by this driver.

  @retval EFI_SUCCESS              The reset operation succeeds.
  @retval EFI_DEVICE_ERROR         The dependent serial port reset fails.

**/
EFI_STATUS
EFIAPI
TerminalConInReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                         ExtendedVerification
  )
{
  EFI_STATUS    Status;
  TERMINAL_DEV  *TerminalDevice;

  TerminalDevice = TERMINAL_CON_IN_DEV_FROM_THIS (This);

  Status = TerminalDevice->SerialIo->Reset (TerminalDevice->SerialIo);

  //
  // Make all the internal buffer empty for keys
  //
  TerminalDevice->RawFiFo->Head     = TerminalDevice->RawFiFo->Tail;
  TerminalDevice->UnicodeFiFo->Head = TerminalDevice->UnicodeFiFo->Tail;
  TerminalDevice->EfiKeyFiFo->Head  = TerminalDevice->EfiKeyFiFo->Tail;

  return Status;
}

/**
  Implements EFI_SIMPLE_TEXT_INPUT_PROTOCOL.ReadKeyStroke().

  @param  This                Indicates the calling context.
  @param  Key                 A pointer to a buffer that is filled in with the
                              keystroke information for the key that was sent
                              from terminal.

  @retval EFI_SUCCESS         The keystroke information is returned successfully.
  @retval EFI_NOT_READY       There is no keystroke data available.
  @retval EFI_DEVICE_ERROR    The dependent serial device encounters error.

**/
EFI_STATUS
EFIAPI
TerminalConInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                   *Key
  )
{
  TERMINAL_DEV  *TerminalDevice;
  EFI_STATUS    Status;
  EFI_KEY_DATA  KeyData;

  //
  //  get TERMINAL_DEV from "This" parameter.
  //
  TerminalDevice  = TERMINAL_CON_IN_DEV_FROM_THIS (This);

  Status = ReadKeyStrokeWorker (TerminalDevice, &KeyData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (Key, &KeyData.Key, sizeof (EFI_INPUT_KEY));

  return EFI_SUCCESS;

}

/**
  Check if the key already has been registered.

  If both RegsiteredData and InputData is NULL, then ASSERT().

  @param  RegsiteredData           A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   registered.
  @param  InputData                A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   pressed.

  @retval TRUE                     Key be pressed matches a registered key.
  @retval FALSE                    Match failed.

**/
BOOLEAN
IsKeyRegistered (
  IN EFI_KEY_DATA  *RegsiteredData,
  IN EFI_KEY_DATA  *InputData
  )
{
  ASSERT (RegsiteredData != NULL && InputData != NULL);

  if ((RegsiteredData->Key.ScanCode    != InputData->Key.ScanCode) ||
      (RegsiteredData->Key.UnicodeChar != InputData->Key.UnicodeChar)) {
    return FALSE;
  }

  return TRUE;
}

/**
  Translate raw data into Unicode (according to different encode), and
  translate Unicode into key information. (according to different standard).

  @param  TerminalDevice       Terminal driver private structure.

**/
VOID
TranslateRawDataToEfiKey (
  IN  TERMINAL_DEV      *TerminalDevice
  )
{
    AnsiRawDataToUnicode (TerminalDevice);
    UnicodeToEfiKey (TerminalDevice);
}

/**
  Event notification function for EFI_SIMPLE_TEXT_INPUT_PROTOCOL.WaitForKey event
  Signal the event if there is key available

  @param  Event                    Indicates the event that invoke this function.
  @param  Context                  Indicates the calling context.

**/
VOID
EFIAPI
TerminalConInWaitForKey (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  )
{
  //
  // Someone is waiting on the keystroke event, if there's
  // a key pending, signal the event
  //
  if (!IsEfiKeyFiFoEmpty ((TERMINAL_DEV *) Context)) {

    gBS->SignalEvent (Event);
  }
}

/**
  Timer handler to poll the key from serial.

  @param  Event                    Indicates the event that invoke this function.
  @param  Context                  Indicates the calling context.
**/
VOID
EFIAPI
TerminalConInTimerHandler (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
{
  EFI_STATUS              Status;
  TERMINAL_DEV            *TerminalDevice;
  UINT32                  Control;
  UINT8                   Input;
  EFI_SERIAL_IO_MODE      *Mode;
  EFI_SERIAL_IO_PROTOCOL  *SerialIo;
  UINTN                   SerialInTimeOut;

  TerminalDevice  = (TERMINAL_DEV *) Context;

  SerialIo        = TerminalDevice->SerialIo;
  if (SerialIo == NULL) {
    return ;
  }
  //
  //  if current timeout value for serial device is not identical with
  //  the value saved in TERMINAL_DEV structure, then recalculate the
  //  timeout value again and set serial attribute according to this value.
  //
  Mode = SerialIo->Mode;
  if (Mode->Timeout != TerminalDevice->SerialInTimeOut) {

    SerialInTimeOut = 0;
    if (Mode->BaudRate != 0) {
      //
      // According to BAUD rate to calculate the timeout value.
      //
      SerialInTimeOut = (1 + Mode->DataBits + Mode->StopBits) * 2 * 1000000 / (UINTN) Mode->BaudRate;
    }

    Status = SerialIo->SetAttributes (
                        SerialIo,
                        Mode->BaudRate,
                        Mode->ReceiveFifoDepth,
                        (UINT32) SerialInTimeOut,
                        (EFI_PARITY_TYPE) (Mode->Parity),
                        (UINT8) Mode->DataBits,
                        (EFI_STOP_BITS_TYPE) (Mode->StopBits)
                        );

    if (EFI_ERROR (Status)) {
      TerminalDevice->SerialInTimeOut = 0;
    } else {
      TerminalDevice->SerialInTimeOut = SerialInTimeOut;
    }
  }
  //
  // Check whether serial buffer is empty.
  //
  Status = SerialIo->GetControl (SerialIo, &Control);

  if ((Control & EFI_SERIAL_INPUT_BUFFER_EMPTY) == 0) {
    //
    // Fetch all the keys in the serial buffer,
    // and insert the byte stream into RawFIFO.
    //
    while (!IsRawFiFoFull (TerminalDevice)) {

      Status = GetOneKeyFromSerial (TerminalDevice->SerialIo, &Input);

      if (EFI_ERROR (Status)) {
        break;
      }

      RawFiFoInsertOneKey (TerminalDevice, Input);
    }
  }

  //
  // Translate all the raw data in RawFIFO into EFI Key,
  // according to different terminal type supported.
  //
  TranslateRawDataToEfiKey (TerminalDevice);
}

/**
  Get one key out of serial buffer.

  @param  SerialIo           Serial I/O protocol attached to the serial device.
  @param  Output             The fetched key.

  @retval EFI_NOT_READY      If serial buffer is empty.
  @retval EFI_DEVICE_ERROR   If reading serial buffer encounter error.
  @retval EFI_SUCCESS        If reading serial buffer successfully, put
                             the fetched key to the parameter output.

**/
EFI_STATUS
GetOneKeyFromSerial (
  EFI_SERIAL_IO_PROTOCOL  *SerialIo,
  UINT8                   *Output
  )
{
  EFI_STATUS  Status;
  UINTN       Size;

  Size    = 1;
  *Output = 0;

  //
  // Read one key from serial I/O device.
  //
  Status  = SerialIo->Read (SerialIo, &Size, Output);

  if (EFI_ERROR (Status)) {

    if (Status == EFI_TIMEOUT) {
      return EFI_NOT_READY;
    }

    return EFI_DEVICE_ERROR;

  }

  if (*Output == 0) {
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

/**
  Insert one byte raw data into the Raw Data FIFO.

  @param  TerminalDevice       Terminal driver private structure.
  @param  Input                The key will be input.

  @retval TRUE                 If insert successfully.
  @retval FALSE                If Raw Data buffer is full before key insertion,
                               and the key is lost.

**/
BOOLEAN
RawFiFoInsertOneKey (
  TERMINAL_DEV      *TerminalDevice,
  UINT8             Input
  )
{
  UINT8 Tail;

  Tail = TerminalDevice->RawFiFo->Tail;

  if (IsRawFiFoFull (TerminalDevice)) {
    //
    // Raw FIFO is full
    //
    return FALSE;
  }

  TerminalDevice->RawFiFo->Data[Tail]  = Input;

  TerminalDevice->RawFiFo->Tail        = (UINT8) ((Tail + 1) % (RAW_FIFO_MAX_NUMBER + 1));

  return TRUE;
}

/**
  Remove one pre-fetched key out of the Raw Data FIFO.

  @param  TerminalDevice       Terminal driver private structure.
  @param  Output               The key will be removed.

  @retval TRUE                 If insert successfully.
  @retval FALSE                If Raw Data FIFO buffer is empty before remove operation.

**/
BOOLEAN
RawFiFoRemoveOneKey (
  TERMINAL_DEV  *TerminalDevice,
  UINT8         *Output
  )
{
  UINT8 Head;

  Head = TerminalDevice->RawFiFo->Head;

  if (IsRawFiFoEmpty (TerminalDevice)) {
    //
    //  FIFO is empty
    //
    *Output = 0;
    return FALSE;
  }

  *Output                       = TerminalDevice->RawFiFo->Data[Head];

  TerminalDevice->RawFiFo->Head  = (UINT8) ((Head + 1) % (RAW_FIFO_MAX_NUMBER + 1));

  return TRUE;
}

/**
  Clarify whether Raw Data FIFO buffer is empty.

  @param  TerminalDevice       Terminal driver private structure

  @retval TRUE                 If Raw Data FIFO buffer is empty.
  @retval FALSE                If Raw Data FIFO buffer is not empty.

**/
BOOLEAN
IsRawFiFoEmpty (
  TERMINAL_DEV  *TerminalDevice
  )
{
  if (TerminalDevice->RawFiFo->Head == TerminalDevice->RawFiFo->Tail) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Clarify whether Raw Data FIFO buffer is full.

  @param  TerminalDevice       Terminal driver private structure

  @retval TRUE                 If Raw Data FIFO buffer is full.
  @retval FALSE                If Raw Data FIFO buffer is not full.

**/
BOOLEAN
IsRawFiFoFull (
  TERMINAL_DEV  *TerminalDevice
  )
{
  UINT8 Tail;
  UINT8 Head;

  Tail  = TerminalDevice->RawFiFo->Tail;
  Head  = TerminalDevice->RawFiFo->Head;

  if (((Tail + 1) % (RAW_FIFO_MAX_NUMBER + 1)) == Head) {

    return TRUE;
  }

  return FALSE;
}

/**
  Insert one pre-fetched key into the FIFO buffer.

  @param  TerminalDevice       Terminal driver private structure.
  @param  Key                  The key will be input.

  @retval TRUE                 If insert successfully.
  @retval FALSE                If FIFO buffer is full before key insertion,
                               and the key is lost.

**/
BOOLEAN
EfiKeyFiFoInsertOneKey (
  TERMINAL_DEV                    *TerminalDevice,
  EFI_INPUT_KEY                   *Key
  )
{
  UINT8                           Tail;
  EFI_KEY_DATA                    KeyData;

  Tail = TerminalDevice->EfiKeyFiFo->Tail;

  CopyMem (&KeyData.Key, Key, sizeof (EFI_INPUT_KEY));
  KeyData.KeyState.KeyShiftState  = 0;
  KeyData.KeyState.KeyToggleState = 0;

  //
  // Invoke notification functions if exist
  //
  if (IsEfiKeyFiFoFull (TerminalDevice)) {
    //
    // Efi Key FIFO is full
    //
    return FALSE;
  }

  CopyMem (&TerminalDevice->EfiKeyFiFo->Data[Tail], Key, sizeof (EFI_INPUT_KEY));

  TerminalDevice->EfiKeyFiFo->Tail = (UINT8) ((Tail + 1) % (FIFO_MAX_NUMBER + 1));

  return TRUE;
}

/**
  Remove one pre-fetched key out of the FIFO buffer.

  @param  TerminalDevice       Terminal driver private structure.
  @param  Output               The key will be removed.

  @retval TRUE                 If insert successfully.
  @retval FALSE                If FIFO buffer is empty before remove operation.

**/
BOOLEAN
EfiKeyFiFoRemoveOneKey (
  TERMINAL_DEV  *TerminalDevice,
  EFI_INPUT_KEY *Output
  )
{
  UINT8 Head;

  Head = TerminalDevice->EfiKeyFiFo->Head;
  ASSERT (Head < FIFO_MAX_NUMBER + 1);

  if (IsEfiKeyFiFoEmpty (TerminalDevice)) {
    //
    //  FIFO is empty
    //
    Output->ScanCode    = SCAN_NULL;
    Output->UnicodeChar = 0;
    return FALSE;
  }

  *Output                         = TerminalDevice->EfiKeyFiFo->Data[Head];

  TerminalDevice->EfiKeyFiFo->Head = (UINT8) ((Head + 1) % (FIFO_MAX_NUMBER + 1));

  return TRUE;
}

/**
  Clarify whether FIFO buffer is empty.

  @param  TerminalDevice       Terminal driver private structure

  @retval TRUE                 If FIFO buffer is empty.
  @retval FALSE                If FIFO buffer is not empty.

**/
BOOLEAN
IsEfiKeyFiFoEmpty (
  TERMINAL_DEV  *TerminalDevice
  )
{
  if (TerminalDevice->EfiKeyFiFo->Head == TerminalDevice->EfiKeyFiFo->Tail) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Clarify whether FIFO buffer is full.

  @param  TerminalDevice       Terminal driver private structure

  @retval TRUE                 If FIFO buffer is full.
  @retval FALSE                If FIFO buffer is not full.

**/
BOOLEAN
IsEfiKeyFiFoFull (
  TERMINAL_DEV  *TerminalDevice
  )
{
  UINT8 Tail;
  UINT8 Head;

  Tail  = TerminalDevice->EfiKeyFiFo->Tail;
  Head  = TerminalDevice->EfiKeyFiFo->Head;

  if (((Tail + 1) % (FIFO_MAX_NUMBER + 1)) == Head) {

    return TRUE;
  }

  return FALSE;
}

/**
  Insert one pre-fetched key into the Unicode FIFO buffer.

  @param  TerminalDevice       Terminal driver private structure.
  @param  Input                The key will be input.

  @retval TRUE                 If insert successfully.
  @retval FALSE                If Unicode FIFO buffer is full before key insertion,
                               and the key is lost.

**/
BOOLEAN
UnicodeFiFoInsertOneKey (
  TERMINAL_DEV      *TerminalDevice,
  UINT16            Input
  )
{
  UINT8 Tail;

  Tail = TerminalDevice->UnicodeFiFo->Tail;
  ASSERT (Tail < FIFO_MAX_NUMBER + 1);


  if (IsUnicodeFiFoFull (TerminalDevice)) {
    //
    // Unicode FIFO is full
    //
    return FALSE;
  }

  TerminalDevice->UnicodeFiFo->Data[Tail]  = Input;

  TerminalDevice->UnicodeFiFo->Tail        = (UINT8) ((Tail + 1) % (FIFO_MAX_NUMBER + 1));

  return TRUE;
}

/**
  Remove one pre-fetched key out of the Unicode FIFO buffer.
  The caller should guarantee that Unicode FIFO buffer is not empty 
  by IsUnicodeFiFoEmpty ().

  @param  TerminalDevice       Terminal driver private structure.
  @param  Output               The key will be removed.

**/
VOID
UnicodeFiFoRemoveOneKey (
  TERMINAL_DEV  *TerminalDevice,
  UINT16        *Output
  )
{
  UINT8 Head;

  Head = TerminalDevice->UnicodeFiFo->Head;
  ASSERT (Head < FIFO_MAX_NUMBER + 1);

  *Output = TerminalDevice->UnicodeFiFo->Data[Head];

  TerminalDevice->UnicodeFiFo->Head = (UINT8) ((Head + 1) % (FIFO_MAX_NUMBER + 1));
}

/**
  Clarify whether Unicode FIFO buffer is empty.

  @param  TerminalDevice       Terminal driver private structure

  @retval TRUE                 If Unicode FIFO buffer is empty.
  @retval FALSE                If Unicode FIFO buffer is not empty.

**/
BOOLEAN
IsUnicodeFiFoEmpty (
  TERMINAL_DEV  *TerminalDevice
  )
{
  if (TerminalDevice->UnicodeFiFo->Head == TerminalDevice->UnicodeFiFo->Tail) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Clarify whether Unicode FIFO buffer is full.

  @param  TerminalDevice       Terminal driver private structure

  @retval TRUE                 If Unicode FIFO buffer is full.
  @retval FALSE                If Unicode FIFO buffer is not full.

**/
BOOLEAN
IsUnicodeFiFoFull (
  TERMINAL_DEV  *TerminalDevice
  )
{
  UINT8 Tail;
  UINT8 Head;

  Tail  = TerminalDevice->UnicodeFiFo->Tail;
  Head  = TerminalDevice->UnicodeFiFo->Head;

  if (((Tail + 1) % (FIFO_MAX_NUMBER + 1)) == Head) {

    return TRUE;
  }

  return FALSE;
}

/**
  Count Unicode FIFO buffer.

  @param  TerminalDevice       Terminal driver private structure

  @return The count in bytes of Unicode FIFO.

**/
UINT8
UnicodeFiFoGetKeyCount (
  TERMINAL_DEV    *TerminalDevice
  )
{
  UINT8 Tail;
  UINT8 Head;

  Tail  = TerminalDevice->UnicodeFiFo->Tail;
  Head  = TerminalDevice->UnicodeFiFo->Head;

  if (Tail >= Head) {
    return (UINT8) (Tail - Head);
  } else {
    return (UINT8) (Tail + FIFO_MAX_NUMBER + 1 - Head);
  }
}

/**
  Update the Unicode characters from a terminal input device into EFI Keys FIFO.

  @param TerminalDevice   The terminal device to use to translate raw input into EFI Keys

**/
VOID
UnicodeToEfiKeyFlushState (
  IN  TERMINAL_DEV    *TerminalDevice
  )
{
  EFI_INPUT_KEY Key;
  UINT32        InputState;

  InputState = TerminalDevice->InputState;

  if (IsEfiKeyFiFoFull (TerminalDevice)) {
    return;
  }

  if ((InputState & INPUT_STATE_ESC) != 0) {
    Key.ScanCode    = SCAN_ESC;
    Key.UnicodeChar = 0;
    EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
  }

  if ((InputState & INPUT_STATE_CSI) != 0) {
    Key.ScanCode    = SCAN_NULL;
    Key.UnicodeChar = CSI;
    EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
  }

  if ((InputState & INPUT_STATE_LEFTOPENBRACKET) != 0) {
    Key.ScanCode    = SCAN_NULL;
    Key.UnicodeChar = LEFTOPENBRACKET;
    EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
  }

  if ((InputState & INPUT_STATE_O) != 0) {
    Key.ScanCode    = SCAN_NULL;
    Key.UnicodeChar = 'O';
    EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
  }

  if ((InputState & INPUT_STATE_2) != 0) {
    Key.ScanCode    = SCAN_NULL;
    Key.UnicodeChar = '2';
    EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
  }

  //
  // Cancel the timer.
  //
  gBS->SetTimer (
        TerminalDevice->TwoSecondTimeOut,
        TimerCancel,
        0
        );

  TerminalDevice->InputState = INPUT_STATE_DEFAULT;
}

typedef struct {
  UINT8  UnicodeChar;
  UINT8  ScanCode;
} UNICODE_TO_SCAN_CODE_TABLE;

UNICODE_TO_SCAN_CODE_TABLE  mAnsiUnicodeToScanCodeTable[] = {
  {'A', SCAN_UP},
  {'B', SCAN_DOWN},
  {'C', SCAN_RIGHT},
  {'D', SCAN_LEFT},
  {'H', SCAN_HOME},
  {'F', SCAN_END},
  {'L', SCAN_INSERT},
  {'@', SCAN_INSERT},
  {'X', SCAN_DELETE},
  {'P', SCAN_F4},
  {'I', SCAN_PAGE_UP},
  {'V', SCAN_F10},
  {'G', SCAN_PAGE_DOWN},
  {'U', SCAN_F9},
  {'M', SCAN_F1},
  {'N', SCAN_F2},
  {'O', SCAN_F3},
  {'Q', SCAN_F5},
  {'R', SCAN_F6},
  {'S', SCAN_F7},
  {'T', SCAN_F8},
};

UINT16
AnsiUnicodeToScanCode (
  IN UINT16  UnicodeChar
  )
{
  UINTN  Index;
  for (Index = 0; Index < sizeof(mAnsiUnicodeToScanCodeTable)/sizeof(mAnsiUnicodeToScanCodeTable[0]); Index++) {
    if (mAnsiUnicodeToScanCodeTable[Index].UnicodeChar == UnicodeChar) {
      return mAnsiUnicodeToScanCodeTable[Index].ScanCode;
    }
  }
  return SCAN_NULL;
}


/**
  Converts a stream of Unicode characters from a terminal input device into EFI Keys that
  can be read through the Simple Input Protocol.

  The table below shows the keyboard input mappings that this function supports.
  If the ESC sequence listed in one of the columns is presented, then it is translated
  into the corresponding EFI Scan Code.  If a matching sequence is not found, then the raw
  key strokes are converted into EFI Keys.

  2 seconds are allowed for an ESC sequence to be completed.  If the ESC sequence is not
  completed in 2 seconds, then the raw key strokes of the partial ESC sequence are
  converted into EFI Keys.
  There is one special input sequence that will force the system to reset.
  This is ESC R ESC r ESC R.

  Note: current implementation support terminal types include: PC ANSI, VT100+/VTUTF8, VT100. 
        The table below is not same with UEFI Spec 2.3 Appendix B Table 201(not support ANSI X3.64 /
        DEC VT200-500 and extra support PC ANSI, VT100)since UEFI Table 201 is just an example.
        
  Symbols used in table below
  ===========================
    ESC = 0x1B
    CSI = 0x9B
    DEL = 0x7f
    ^   = CTRL

  +=========+======+===========+==========+==========+
  |         | EFI  | UEFI 2.0  |          |          |
  |         | Scan |           |  VT100+  |          |
  |   KEY   | Code |  PC ANSI  |  VTUTF8  |   VT100  |
  +=========+======+===========+==========+==========+
  | NULL    | 0x00 |           |          |          |
  | UP      | 0x01 | ESC [ A   | ESC [ A  | ESC [ A  |
  | DOWN    | 0x02 | ESC [ B   | ESC [ B  | ESC [ B  |
  | RIGHT   | 0x03 | ESC [ C   | ESC [ C  | ESC [ C  |
  | LEFT    | 0x04 | ESC [ D   | ESC [ D  | ESC [ D  |
  | HOME    | 0x05 | ESC [ H   | ESC h    | ESC [ H  |
  | END     | 0x06 | ESC [ F   | ESC k    | ESC [ K  |
  | INSERT  | 0x07 | ESC [ @   | ESC +    | ESC [ @  |
  |         |      | ESC [ L   |          | ESC [ L  |
  | DELETE  | 0x08 | ESC [ X   | ESC -    | ESC [ P  |
  | PG UP   | 0x09 | ESC [ I   | ESC ?    | ESC [ V  |
  |         |      |           |          | ESC [ ?  |
  | PG DOWN | 0x0A | ESC [ G   | ESC /    | ESC [ U  |
  |         |      |           |          | ESC [ /  |
  | F1      | 0x0B | ESC [ M   | ESC 1    | ESC O P  |
  | F2      | 0x0C | ESC [ N   | ESC 2    | ESC O Q  |
  | F3      | 0x0D | ESC [ O   | ESC 3    | ESC O w  |
  | F4      | 0x0E | ESC [ P   | ESC 4    | ESC O x  |
  | F5      | 0x0F | ESC [ Q   | ESC 5    | ESC O t  |
  | F6      | 0x10 | ESC [ R   | ESC 6    | ESC O u  |
  | F7      | 0x11 | ESC [ S   | ESC 7    | ESC O q  |
  | F8      | 0x12 | ESC [ T   | ESC 8    | ESC O r  |
  | F9      | 0x13 | ESC [ U   | ESC 9    | ESC O p  |
  | F10     | 0x14 | ESC [ V   | ESC 0    | ESC O M  |
  | Escape  | 0x17 | ESC       | ESC      | ESC      |
  | F11     | 0x15 |           | ESC !    |          |
  | F12     | 0x16 |           | ESC @    |          |
  +=========+======+===========+==========+==========+

  Special Mappings
  ================
  ESC R ESC r ESC R = Reset System

  @param TerminalDevice   The terminal device to use to translate raw input into EFI Keys

**/
VOID
UnicodeToEfiKey (
  IN  TERMINAL_DEV    *TerminalDevice
  )
{
  EFI_STATUS          Status;
  EFI_STATUS          TimerStatus;
  UINT16              UnicodeChar;
  EFI_INPUT_KEY       Key;
  BOOLEAN             SetDefaultResetState;

  TimerStatus = gBS->CheckEvent (TerminalDevice->TwoSecondTimeOut);

  if (!EFI_ERROR (TimerStatus)) {
    UnicodeToEfiKeyFlushState (TerminalDevice);
    TerminalDevice->ResetState = RESET_STATE_DEFAULT;
  }

  while (!IsUnicodeFiFoEmpty (TerminalDevice) && !IsEfiKeyFiFoFull (TerminalDevice)) {

    if (TerminalDevice->InputState != INPUT_STATE_DEFAULT) {
      //
      // Check to see if the 2 seconds timer has expired
      //
      TimerStatus = gBS->CheckEvent (TerminalDevice->TwoSecondTimeOut);
      if (!EFI_ERROR (TimerStatus)) {
        UnicodeToEfiKeyFlushState (TerminalDevice);
        TerminalDevice->ResetState = RESET_STATE_DEFAULT;
      }
    }

    //
    // Fetch one Unicode character from the Unicode FIFO
    //
    UnicodeFiFoRemoveOneKey (TerminalDevice, &UnicodeChar);

    SetDefaultResetState = TRUE;

    switch (TerminalDevice->InputState) {
    case INPUT_STATE_DEFAULT:

      break;

    case INPUT_STATE_ESC:

      if (UnicodeChar == LEFTOPENBRACKET) {
        TerminalDevice->InputState |= INPUT_STATE_LEFTOPENBRACKET;
        TerminalDevice->ResetState = RESET_STATE_DEFAULT;
        continue;
      }

      Key.ScanCode = SCAN_NULL;

      switch (UnicodeChar) {
      case 'R':
        if (TerminalDevice->ResetState == RESET_STATE_DEFAULT) {
          TerminalDevice->ResetState = RESET_STATE_ESC_R;
          SetDefaultResetState = FALSE;
        } else if (TerminalDevice->ResetState == RESET_STATE_ESC_R_ESC_R) {
          gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
        }
        Key.ScanCode = SCAN_NULL;
        break;
      case 'r':
        if (TerminalDevice->ResetState == RESET_STATE_ESC_R) {
          TerminalDevice->ResetState = RESET_STATE_ESC_R_ESC_R;
          SetDefaultResetState = FALSE;
        }
        Key.ScanCode = SCAN_NULL;
        break;
      default :
        break;
      }

      if (SetDefaultResetState) {
        TerminalDevice->ResetState = RESET_STATE_DEFAULT;
      }

      if (Key.ScanCode != SCAN_NULL) {
        Key.UnicodeChar = 0;
        EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
        TerminalDevice->InputState = INPUT_STATE_DEFAULT;
        UnicodeToEfiKeyFlushState (TerminalDevice);
        continue;
      }

      UnicodeToEfiKeyFlushState (TerminalDevice);

      break;

    case INPUT_STATE_ESC | INPUT_STATE_O:

      TerminalDevice->ResetState = RESET_STATE_DEFAULT;

      Key.ScanCode = SCAN_NULL;

      if (Key.ScanCode != SCAN_NULL) {
        Key.UnicodeChar = 0;
        EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
        TerminalDevice->InputState = INPUT_STATE_DEFAULT;
        UnicodeToEfiKeyFlushState (TerminalDevice);
        continue;
      }

      UnicodeToEfiKeyFlushState (TerminalDevice);

      break;

    case INPUT_STATE_ESC | INPUT_STATE_LEFTOPENBRACKET:

      TerminalDevice->ResetState = RESET_STATE_DEFAULT;

      Key.ScanCode = AnsiUnicodeToScanCode (UnicodeChar);

      if (Key.ScanCode != SCAN_NULL) {
        Key.UnicodeChar = 0;
        EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
        TerminalDevice->InputState = INPUT_STATE_DEFAULT;
        UnicodeToEfiKeyFlushState (TerminalDevice);
        continue;
      }

      UnicodeToEfiKeyFlushState (TerminalDevice);

      break;


    default:
      //
      // Invalid state. This should never happen.
      //
      ASSERT (FALSE);

      UnicodeToEfiKeyFlushState (TerminalDevice);

      break;
    }

    if (UnicodeChar == ESC) {
      TerminalDevice->InputState = INPUT_STATE_ESC;
    }

    if (UnicodeChar == CSI) {
      TerminalDevice->InputState = INPUT_STATE_CSI;
    }

    if (TerminalDevice->InputState != INPUT_STATE_DEFAULT) {
      Status = gBS->SetTimer(
                      TerminalDevice->TwoSecondTimeOut,
                      TimerRelative,
                      (UINT64)20000000
                      );
      ASSERT_EFI_ERROR (Status);
      continue;
    }

    if (SetDefaultResetState) {
      TerminalDevice->ResetState = RESET_STATE_DEFAULT;
    }

    if (UnicodeChar == DEL) {
      Key.ScanCode    = SCAN_DELETE;
      Key.UnicodeChar = 0;
    } else {
      Key.ScanCode    = SCAN_NULL;
      Key.UnicodeChar = UnicodeChar;
    }

    EfiKeyFiFoInsertOneKey (TerminalDevice, &Key);
  }
}
