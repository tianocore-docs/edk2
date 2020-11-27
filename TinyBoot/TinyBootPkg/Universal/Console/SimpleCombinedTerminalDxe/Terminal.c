/** @file
  Produces Simple Text Input Protocol, Simple Text Input Extended Protocol and
  Simple Text Output Protocol upon Serial IO Protocol.

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

extern EFI_SERIAL_IO_PROTOCOL mSerialIo;

TERMINAL_DEV  mTerminalDevTemplate = {
  TERMINAL_DEV_SIGNATURE,
  NULL,
  0,
  NULL,
  {   // SimpleTextInput
    TerminalConInReset,
    TerminalConInReadKeyStroke,
    NULL
  },
  {   // SimpleTextOutput
    TerminalConOutReset,
    TerminalConOutOutputString,
    TerminalConOutTestString,
    TerminalConOutQueryMode,
    TerminalConOutSetMode,
    TerminalConOutSetAttribute,
    TerminalConOutClearScreen,
    TerminalConOutSetCursorPosition,
    TerminalConOutEnableCursor,
    NULL
  },
  {   // SimpleTextOutputMode
    1,                                           // MaxMode
    0,                                           // Mode
    EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK),    // Attribute
    0,                                           // CursorColumn
    0,                                           // CursorRow
    TRUE                                         // CursorVisible
  },
  NULL, // TerminalConsoleModeData
  0,  // SerialInTimeOut

  NULL, // RawFifo
  NULL, // UnicodeFiFo
  NULL, // EfiKeyFiFo

  NULL, // TimerEvent
  NULL, // TwoSecondTimeOut
  INPUT_STATE_DEFAULT,
  RESET_STATE_DEFAULT,
  FALSE,
};

TERMINAL_CONSOLE_MODE_DATA mTerminalConsoleModeData[] = {
  {100, 31},
  //
  // New modes can be added here.
  //
};

/**
  Initialize all the text modes which the terminal console supports.

  It returns information for available text modes that the terminal can support.

  @param[out] TextModeCount      The total number of text modes that terminal console supports.
  @param[out] TextModeData       The buffer to the text modes column and row information.
                                 Caller is responsible to free it when it's non-NULL.

  @retval EFI_SUCCESS            The supporting mode information is returned.
  @retval EFI_INVALID_PARAMETER  The parameters are invalid.

**/
EFI_STATUS
InitializeTerminalConsoleTextMode (
  OUT UINTN                         *TextModeCount,
  OUT TERMINAL_CONSOLE_MODE_DATA    **TextModeData
  )
{
  UINTN                       Index;
  UINTN                       Count;
  TERMINAL_CONSOLE_MODE_DATA  *ModeBuffer;
  TERMINAL_CONSOLE_MODE_DATA  *NewModeBuffer;
  UINTN                       ValidCount;
  UINTN                       ValidIndex;
  
  if ((TextModeCount == NULL) || (TextModeData == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  Count = sizeof (mTerminalConsoleModeData) / sizeof (TERMINAL_CONSOLE_MODE_DATA);
  
  //
  // Get defined mode buffer pointer.
  //
  ModeBuffer = mTerminalConsoleModeData;
    
  //
  // Here we make sure that the final mode exposed does not include the duplicated modes,
  // and does not include the invalid modes which exceed the max column and row.
  // Reserve 2 modes for 80x25, 80x50 of terminal console.
  //
  NewModeBuffer = AllocateZeroPool (sizeof (TERMINAL_CONSOLE_MODE_DATA) * (Count + 2));
  ASSERT (NewModeBuffer != NULL);

  //
  // Mode 0 and mode 1 is for 80x25, 80x50 according to UEFI spec.
  //
  ValidCount = 0;  

  NewModeBuffer[ValidCount].Columns = 80;
  NewModeBuffer[ValidCount].Rows    = 25;
  ValidCount++;

  NewModeBuffer[ValidCount].Columns = 80;
  NewModeBuffer[ValidCount].Rows    = 50;
  ValidCount++;
  
  //
  // Start from mode 2 to put the valid mode other than 80x25 and 80x50 in the output mode buffer.
  //
  for (Index = 0; Index < Count; Index++) {
    if ((ModeBuffer[Index].Columns == 0) || (ModeBuffer[Index].Rows == 0)) {
      //
      // Skip the pre-defined mode which is invalid.
      //
      continue;
    }
    for (ValidIndex = 0; ValidIndex < ValidCount; ValidIndex++) {
      if ((ModeBuffer[Index].Columns == NewModeBuffer[ValidIndex].Columns) &&
          (ModeBuffer[Index].Rows == NewModeBuffer[ValidIndex].Rows)) {
        //
        // Skip the duplicated mode.
        //
        break;
      }
    }
    if (ValidIndex == ValidCount) {
      NewModeBuffer[ValidCount].Columns = ModeBuffer[Index].Columns;
      NewModeBuffer[ValidCount].Rows    = ModeBuffer[Index].Rows;
      ValidCount++;
    }
  }
 
  DEBUG_CODE (
    for (Index = 0; Index < ValidCount; Index++) {
      DEBUG ((EFI_D_INFO, "Terminal - Mode %d, Column = %d, Row = %d\n", 
                           Index, NewModeBuffer[Index].Columns, NewModeBuffer[Index].Rows));  
    }
  );
  
  //
  // Return valid mode count and mode information buffer.
  //
  *TextModeCount = ValidCount;
  *TextModeData  = NewModeBuffer;
  return EFI_SUCCESS;
}

/**
  Start this driver on Controller by opening a Serial IO protocol,
  reading Device Path, and creating a child handle with a Simple Text In,
  Simple Text In Ex and Simple Text Out protocol, and device path protocol.
  And store Console Device Environment Variables.

  @param  This                 Protocol instance pointer.
  @param  Controller           Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          This driver is added to Controller.
  @retval EFI_ALREADY_STARTED  This driver is already running on Controller.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
TerminalDriverStart (
  IN EFI_SERIAL_IO_PROTOCOL              *SerialIo
  )
{
  EFI_STATUS                          Status;
  EFI_SERIAL_IO_MODE                  *Mode;
  UINTN                               SerialInTimeOut;
  TERMINAL_DEV                        *TerminalDevice;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *SimpleTextOutput;
  UINTN                               ModeCount;

    //
    // Initialize the Terminal Dev
    //
    TerminalDevice = AllocateCopyPool (sizeof (TERMINAL_DEV), &mTerminalDevTemplate);
    ASSERT (TerminalDevice != NULL);

    TerminalDevice->TerminalType  = PCANSITYPE;
    TerminalDevice->SerialIo      = SerialIo;

    Status = gBS->CreateEvent (
                    EVT_NOTIFY_WAIT,
                    TPL_NOTIFY,
                    TerminalConInWaitForKey,
                    TerminalDevice,
                    &TerminalDevice->SimpleInput.WaitForKey
                    );
    ASSERT_EFI_ERROR (Status);

    //
    // Allocates and initializes the FIFO buffer to be zero, used for accommodating
    // the pre-read pending characters.
    //
    TerminalDevice->RawFiFo     = AllocateZeroPool (sizeof (RAW_DATA_FIFO));
    ASSERT (TerminalDevice->RawFiFo != NULL);

    TerminalDevice->UnicodeFiFo = AllocateZeroPool (sizeof (UNICODE_FIFO));
    ASSERT (TerminalDevice->UnicodeFiFo != NULL);

    TerminalDevice->EfiKeyFiFo  = AllocateZeroPool (sizeof (EFI_KEY_FIFO));
    ASSERT (TerminalDevice->EfiKeyFiFo != NULL);

    //
    // Set the timeout value of serial buffer for
    // keystroke response performance issue
    //
    Mode            = TerminalDevice->SerialIo->Mode;

    SerialInTimeOut = 0;
    if (Mode->BaudRate != 0) {
      SerialInTimeOut = (1 + Mode->DataBits + Mode->StopBits) * 2 * 1000000 / (UINTN) Mode->BaudRate;
    }

    Status = TerminalDevice->SerialIo->SetAttributes (
                                        TerminalDevice->SerialIo,
                                        Mode->BaudRate,
                                        Mode->ReceiveFifoDepth,
                                        (UINT32) SerialInTimeOut,
                                        (EFI_PARITY_TYPE) (Mode->Parity),
                                        (UINT8) Mode->DataBits,
                                        (EFI_STOP_BITS_TYPE) (Mode->StopBits)
                                        );
    if (EFI_ERROR (Status)) {
      //
      // if set attributes operation fails, invalidate
      // the value of SerialInTimeOut,thus make it
      // inconsistent with the default timeout value
      // of serial buffer. This will invoke the recalculation
      // in the readkeystroke routine.
      //
      TerminalDevice->SerialInTimeOut = 0;
    } else {
      TerminalDevice->SerialInTimeOut = SerialInTimeOut;
    }
    //
    // Set Simple Text Output Protocol from template.
    //
    SimpleTextOutput = CopyMem (
                         &TerminalDevice->SimpleTextOutput,
                         &mTerminalDevTemplate.SimpleTextOutput,
                         sizeof (mTerminalDevTemplate.SimpleTextOutput)
                         );
    SimpleTextOutput->Mode = &TerminalDevice->SimpleTextOutputMode;
    
    Status = InitializeTerminalConsoleTextMode (&ModeCount, &TerminalDevice->TerminalConsoleModeData);
    ASSERT_EFI_ERROR (Status);

    TerminalDevice->SimpleTextOutputMode.MaxMode = (INT32) ModeCount;
    
    //
    // For terminal devices, cursor is always visible
    //
    TerminalDevice->SimpleTextOutputMode.CursorVisible = TRUE;
    Status = TerminalConOutSetAttribute (
               SimpleTextOutput,
               EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK)
               );
    ASSERT_EFI_ERROR (Status);

    Status = TerminalConOutReset (SimpleTextOutput, FALSE);
    ASSERT_EFI_ERROR (Status);

    Status = TerminalConOutSetMode (SimpleTextOutput, 0);
    ASSERT_EFI_ERROR (Status);

    Status = TerminalConOutEnableCursor (SimpleTextOutput, TRUE);
    ASSERT_EFI_ERROR (Status);

    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    TerminalConInTimerHandler,
                    TerminalDevice,
                    &TerminalDevice->TimerEvent
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->SetTimer (
                    TerminalDevice->TimerEvent,
                    TimerPeriodic,
                    KEYBOARD_TIMER_INTERVAL
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->CreateEvent (
                    EVT_TIMER,
                    TPL_CALLBACK,
                    NULL,
                    NULL,
                    &TerminalDevice->TwoSecondTimeOut
                    );
    ASSERT_EFI_ERROR (Status);

  //
  // Only do the reset if the device path is in the Conout variable
  //
    Status = TerminalDevice->SimpleInput.Reset (
                                          &TerminalDevice->SimpleInput,
                                          FALSE
                                          );
    ASSERT_EFI_ERROR (Status);

  //
  // Only output the configure string to remote terminal if the device path
  // is in the Conout variable
  //
    Status = TerminalDevice->SimpleTextOutput.SetAttribute (
                                                        &TerminalDevice->SimpleTextOutput,
                                                        EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK)
                                                        );
    ASSERT_EFI_ERROR (Status);

    Status = TerminalDevice->SimpleTextOutput.Reset (
                                                &TerminalDevice->SimpleTextOutput,
                                                FALSE
                                                );
    ASSERT_EFI_ERROR (Status);

    Status = TerminalDevice->SimpleTextOutput.SetMode (
                                                &TerminalDevice->SimpleTextOutput,
                                                0
                                                );
    ASSERT_EFI_ERROR (Status);

    Status = TerminalDevice->SimpleTextOutput.EnableCursor (
                                                &TerminalDevice->SimpleTextOutput,
                                                TRUE
                                                );
    ASSERT_EFI_ERROR (Status);

  //
  // Simple In/Out Protocol will not be installed onto the handle if the
  // device path to the handle is not present in the ConIn/ConOut
  // environment variable. But If RemainingDevicePath is NULL, then always
  // produce both Simple In and Simple Text Output Protocols. This is required
  // for the connect all sequences to make sure all possible consoles are
  // produced no matter what the current values of ConIn, ConOut, or StdErr are.
  //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &TerminalDevice->Handle,
                    &gEfiSimpleTextInProtocolGuid,
                    &TerminalDevice->SimpleInput,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->InstallProtocolInterface (
                    &TerminalDevice->Handle,
                    &gEfiSimpleTextOutProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &TerminalDevice->SimpleTextOutput
                    );
    ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
  The user Entry Point for module Terminal. The user code starts with this function.

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeTerminal(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  
  SerialPortInitialize ();

  Status = TerminalDriverStart (&mSerialIo);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

