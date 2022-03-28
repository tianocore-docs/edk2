/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_SECURITY_LIB_H__
#define __SPDM_SECURITY_LIB_H__

#include <Protocol/DeviceSecurity.h>
#include <Protocol/DeviceSecurityPolicy.h>

typedef struct {
  UINT32                                          Version;
  //
  // DeviceType is used to create TCG event log context_data.
  // DeviceHandle is used to create TCG event log device_path information.
  //
  EDKII_DEVICE_IDENTIFIER                         *DeviceId;

  //
  // TRUE  means to use PCR 0 (code) / 1 (config).
  // FALSE means to use PCR 2 (code) / 3 (config).
  //
  BOOLEAN                                         IsEmbeddedDevice;

  //
  // API required by SpdmRegisterDeviceIoFunc in libspdm
  // It is used to send/receive transport message (SPDM + transport header).
  //
  libspdm_device_send_message_func                SendMessage;
  libspdm_device_receive_message_func             ReceiveMessage;
  //
  // API required by SpdmRegisterTransportLayerFunc in libspdm
  // It is used to add/remove transport header for SPDM.
  //
  libspdm_transport_encode_message_func           TransportEncodeMessage;
  libspdm_transport_decode_message_func           TransportDecodeMessage;
  libspdm_transport_get_header_size_func          TransportGetHeaderSize;
  //
  // API required by SpdmRegisterDeviceBufferFunc in libspdm
  // It is used to get the sender/receiver buffer for transport message (SPDM + transport header).
  // The size MUST be big enough to send or receive one transport message (SPDM + transport header).
  // Tthe sender/receiver buffer MAY be overlapped.
  //
  libspdm_device_acquire_sender_buffer_func       AcquireSenderBuffer;
  libspdm_device_release_sender_buffer_func       ReleaseSenderBuffer;
  libspdm_device_acquire_receiver_buffer_func     AcquireReceiverBuffer;
  libspdm_device_release_receiver_buffer_func     ReleaseReceiverBuffer;

} EDKII_SPDM_DEVICE_INFO;

/*
  This function will send SPDM VCA, GET_CERTIFICATE, CHALLENGE, GET_MEASUREMENT,
  The certificate and measurement will be extended to TPM PCR/NvIndex.
*/
RETURN_STATUS
EFIAPI
SpdmDeviceAuthenticationAndMeasurement (
  IN  EDKII_SPDM_DEVICE_INFO         *SpdmDeviceInfo,
  IN  EDKII_DEVICE_SECURITY_POLICY   *SecuriryPolicy,
  OUT EDKII_DEVICE_SECURITY_STATE    *SecuriryState
  );

#endif
