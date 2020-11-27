/**@file
  Serial driver that layers on top of a Serial Port Library instance

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

#include <PiDxe.h>

#include <Protocol/SerialIo.h>
#include <Protocol/DevicePath.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

//
// Serial Driver Defaults
//
#define SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH  1
#define SERIAL_PORT_DEFAULT_TIMEOUT             1000000
#define SERIAL_PORT_DEFAULT_CONTROL_MASK        0

//
// Serial I/O Protocol Interface
//
EFI_STATUS
EFIAPI
SerialReset (
  IN EFI_SERIAL_IO_PROTOCOL         *This
  );

EFI_STATUS
EFIAPI
SerialSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL         *This,
  IN UINT64                         BaudRate,
  IN UINT32                         ReceiveFifoDepth,
  IN UINT32                         Timeout,
  IN EFI_PARITY_TYPE                Parity,
  IN UINT8                          DataBits,
  IN EFI_STOP_BITS_TYPE             StopBits
  );

EFI_STATUS
EFIAPI
SerialSetControl (
  IN EFI_SERIAL_IO_PROTOCOL         *This,
  IN UINT32                         Control
  );

EFI_STATUS
EFIAPI
SerialGetControl (
  IN EFI_SERIAL_IO_PROTOCOL         *This,
  OUT UINT32                        *Control
  );

EFI_STATUS
EFIAPI
SerialWrite (
  IN EFI_SERIAL_IO_PROTOCOL         *This,
  IN OUT UINTN                      *BufferSize,
  IN VOID                           *Buffer
  );

EFI_STATUS
EFIAPI
SerialRead (
  IN EFI_SERIAL_IO_PROTOCOL         *This,
  IN OUT UINTN                      *BufferSize,
  OUT VOID                          *Buffer
  );

//
// Global Varibles
//
UINTN  mControl = SERIAL_PORT_DEFAULT_CONTROL_MASK;

EFI_HANDLE  mSerialIoHandle = NULL;

EFI_SERIAL_IO_MODE  mSerialIoMode = {
  SERIAL_PORT_DEFAULT_CONTROL_MASK,
  SERIAL_PORT_DEFAULT_TIMEOUT,
  FixedPcdGet64 (PcdUartDefaultBaudRate),     // BaudRate
  SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH,
  FixedPcdGet8 (PcdUartDefaultDataBits),      // DataBits
  FixedPcdGet8 (PcdUartDefaultParity),        // Parity
  FixedPcdGet8 (PcdUartDefaultStopBits)       // StopBits
};

EFI_SERIAL_IO_PROTOCOL mSerialIo = {
  SERIAL_IO_INTERFACE_REVISION,
  SerialReset,
  SerialSetAttributes,
  SerialSetControl,
  SerialGetControl,
  SerialWrite,
  SerialRead,
  &mSerialIoMode
};

typedef struct {
  UART_DEVICE_PATH          UartDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  EndDevicePath;
} SERIAL_IO_DEVICE_PATH;

GLOBAL_REMOVE_IF_UNREFERENCED SERIAL_IO_DEVICE_PATH mSerialIoDevicePath = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_UART_DP,
      {
        (UINT8) (sizeof (UART_DEVICE_PATH)),
        (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8)
      }
    },
    0,
    FixedPcdGet64 (PcdUartDefaultBaudRate),    
    FixedPcdGet8 (PcdUartDefaultDataBits),
    FixedPcdGet8 (PcdUartDefaultParity),
    FixedPcdGet8 (PcdUartDefaultStopBits)
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};

/**
  Reset serial device

  @param This             - Pointer to EFI_SERIAL_IO_PROTOCOL

  @retval EFI_SUCCESS      - Reset successfully
  @retval EFI_DEVICE_ERROR - Failed to reset

**/
EFI_STATUS
EFIAPI
SerialReset (
  IN EFI_SERIAL_IO_PROTOCOL  *This
  )
{
  mControl = SERIAL_PORT_DEFAULT_CONTROL_MASK;
  return SerialPortInitialize ();
}

/**
  Set new attributes to a serial device

  @param This                    - Pointer to EFI_SERIAL_IO_PROTOCOL
  @param  BaudRate               - The baudrate of the serial device
  @param  ReceiveFifoDepth       - The depth of receive FIFO buffer
  @param  Timeout                - The request timeout for a single char
  @param  Parity                 - The type of parity used in serial device
  @param  DataBits               - Number of databits used in serial device
  @param  StopBits               - Number of stopbits used in serial device

  @retval  EFI_SUCCESS            - The new attributes were set
  @retval  EFI_INVALID_PARAMETERS - One or more attributes have an unsupported value
  @retval  EFI_UNSUPPORTED        - Data Bits can not set to 5 or 6
  @retval  EFI_DEVICE_ERROR       - The serial device is not functioning correctly (no return)

**/
EFI_STATUS
EFIAPI
SerialSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN UINT64                  BaudRate,
  IN UINT32                  ReceiveFifoDepth,
  IN UINT32                  Timeout,
  IN EFI_PARITY_TYPE         Parity,
  IN UINT8                   DataBits,
  IN EFI_STOP_BITS_TYPE      StopBits
  )
{
  return EFI_SUCCESS;
}

/**
  Set Control Bits

  @param This            - Pointer to EFI_SERIAL_IO_PROTOCOL
  @param Control         - Control bits that can be settable

  @retval EFI_SUCCESS     - New Control bits were set successfully
  @retval EFI_UNSUPPORTED - The Control bits wanted to set are not supported

**/
EFI_STATUS
EFIAPI
SerialSetControl (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN UINT32                  Control
  )
{
  return EFI_SUCCESS;
}

/**
  Get ControlBits

  @param This        - Pointer to EFI_SERIAL_IO_PROTOCOL
  @param Control     - Control signals of the serial device

  @retval EFI_SUCCESS - Get Control signals successfully

**/
EFI_STATUS
EFIAPI
SerialGetControl (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  OUT UINT32                 *Control
  )
{
  *Control = (UINT32)mControl;
  if (!SerialPortPoll ()) {
    *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
  }
  return EFI_SUCCESS;
}

/**
  Write the specified number of bytes to serial device

  @param This             - Pointer to EFI_SERIAL_IO_PROTOCOL
  @param  BufferSize       - On input the size of Buffer, on output the amount of
                       data actually written
  @param  Buffer           - The buffer of data to write

  @retval EFI_SUCCESS      - The data were written successfully
  @retval EFI_DEVICE_ERROR - The device reported an error
  @retval EFI_TIMEOUT      - The write operation was stopped due to timeout

**/
EFI_STATUS
EFIAPI
SerialWrite (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN OUT UINTN               *BufferSize,
  IN VOID                    *Buffer
  )
{
  *BufferSize = SerialPortWrite (Buffer, *BufferSize);
  return EFI_SUCCESS;
}

/**
  Read the specified number of bytes from serial device

  @param This             - Pointer to EFI_SERIAL_IO_PROTOCOL
  @param BufferSize       - On input the size of Buffer, on output the amount of
                       data returned in buffer
  @param Buffer           -  The buffer to return the data into

  @retval EFI_SUCCESS      - The data were read successfully
  @retval EFI_DEVICE_ERROR - The device reported an error
  @retval EFI_TIMEOUT      - The read operation was stopped due to timeout

**/
EFI_STATUS
EFIAPI
SerialRead (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN OUT UINTN               *BufferSize,
  OUT VOID                   *Buffer
  )
{
  UINTN  Index;
  UINT8  *Uint8Buffer;

  for (Index = 0, Uint8Buffer = (UINT8 *)Buffer; Index < *BufferSize; Index++, Uint8Buffer++) {
    if (!SerialPortPoll ()) {
      break;
    }
    SerialPortRead (Uint8Buffer, 1);
  }
  *BufferSize = Index;
  return EFI_SUCCESS;
}
