/** @file
  Serial IO protocol as defined in the UEFI 2.0 specification.

  Abstraction of a basic serial device. Targeted at 16550 UART, but
  could be much more generic.

  Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SERIAL_H__
#define __SERIAL_H__

///
/// Parity type that is computed or checked as each character is transmitted or received. If the
/// device does not support parity, the value is the default parity value.
///
typedef enum {
  DefaultParity,
  NoParity,
  EvenParity,
  OddParity,
  MarkParity,
  SpaceParity
} SERIAL_PARITY_TYPE;

///
/// Stop bits type
///
typedef enum {
  DefaultStopBits,
  OneStopBit,
  OneFiveStopBits,
  TwoStopBits
} SERIAL_STOP_BITS_TYPE;

//
// define for Control bits, grouped by read only, write only, and read write
//
//
// Read Only
//
#define SERIAL_CLEAR_TO_SEND        0x00000010
#define SERIAL_DATA_SET_READY       0x00000020
#define SERIAL_RING_INDICATE        0x00000040
#define SERIAL_CARRIER_DETECT       0x00000080
#define SERIAL_INPUT_BUFFER_EMPTY   0x00000100
#define SERIAL_OUTPUT_BUFFER_EMPTY  0x00000200

//
// Write Only
//
#define SERIAL_REQUEST_TO_SEND      0x00000002
#define SERIAL_DATA_TERMINAL_READY  0x00000001

//
// Read Write
//
#define SERIAL_HARDWARE_LOOPBACK_ENABLE     0x00001000
#define SERIAL_SOFTWARE_LOOPBACK_ENABLE     0x00002000
#define SERIAL_HARDWARE_FLOW_CONTROL_ENABLE 0x00004000

#endif
