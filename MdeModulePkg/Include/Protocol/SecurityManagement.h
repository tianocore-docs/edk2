/** @file
  This protocol defines the interfaces to register security(2) handler.

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_SECURITY_MANAGEMENT_PROTOCOL_H__
#define __EDKII_SECURITY_MANAGEMENT_PROTOCOL_H__

#include <Library/SecurityManagementLib.h>

typedef struct _EDKII_SECURITY_MANAGEMENT_PROTOCOL EDKII_SECURITY_MANAGEMENT_PROTOCOL;

#define EDKII_SECURITY_MANAGEMENT_PROTOCOL_GUID { \
  0xc6c0f803, 0x3d26, 0x4fca, { 0x8a, 0x89, 0x1d, 0x34, 0xf8, 0xa1, 0x61, 0x60 } \
};

/**
  Register security measurement handler with its operation type. Different
  handlers with the same operation can all be registered.

  If SecurityHandler is NULL, then ASSERT().
  If no enough resources available to register new handler, then ASSERT().
  If AuthenticationOperation is not recongnized, then ASSERT().
  If the previous register handler can't be executed before the later register handler, then ASSERT().

  @param[in]  SecurityHandler           The security measurement service handler to be registered.
  @param[in]  AuthenticationOperation   Theoperation type is specified for the registered handler.

  @retval EFI_SUCCESS              The handlers were registered successfully.

**/
typedef
EFI_STATUS
(EFIAPI *EDKII_REGISTER_SECURITY_HANDLER)(
  IN  SECURITY_FILE_AUTHENTICATION_STATE_HANDLER  SecurityHandler,
  IN  UINT32                                      AuthenticationOperation
  );

/**
  Register security2 measurement handler with its operation type. Different
  handlers with the same operation can all be registered.

  If SecurityHandler is NULL, then ASSERT().
  If no enough resources available to register new handler, then ASSERT().
  If AuthenticationOperation is not recongnized, then ASSERT().
  If AuthenticationOperation is EFI_AUTH_OPERATION_NONE, then ASSERT().
  If the previous register handler can't be executed before the later register handler, then ASSERT().

  @param[in]  Security2Handler          The security measurement service handler to be registered.
  @param[in]  AuthenticationOperation   The operation type is specified for the registered handler.

  @retval EFI_SUCCESS              The handlers were registered successfully.

**/
typedef
EFI_STATUS
(EFIAPI *EDKII_REGISTER_SECURITY2_HANDLER)(
  IN  SECURITY2_FILE_AUTHENTICATION_HANDLER       Security2Handler,
  IN  UINT32                                      AuthenticationOperation
  );

struct _EDKII_SECURITY_MANAGEMENT_PROTOCOL {
  EDKII_REGISTER_SECURITY_HANDLER                 RegisterSecurityHandler;
  EDKII_REGISTER_SECURITY2_HANDLER                RegisterSecurity2Handler;
};

extern EFI_GUID gEdkiiSecurityManagementProtocolGuid;

#endif
