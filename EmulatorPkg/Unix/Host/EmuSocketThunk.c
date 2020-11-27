/**@file

    Since the SEC is the only program in our emulation we
  must use a Tiano mechanism to export APIs to other modules.

  This file export socket related function to UEFI.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Host.h"
#include <Protocol/EmuSocket.h>

INT32
EFIAPI
SecSocketStartup (
  VOID
  )
{
  return 0;
}

INT32
EFIAPI
SecSocketCleanup (
  VOID
  )
{
  return 0;
}

INT32
EFIAPI
SecSocketGetLastError (
  VOID
  )
{
  return errno;
}

EMU_SOCKET
EFIAPI
SecSocketCreate (
  IN INT32 af,
  IN INT32 type,
  IN INT32 protocol
  )
{
  return socket (af, type, protocol);
}

INT32
EFIAPI
SecSocketBind (
  IN EMU_SOCKET s,
  IN CONST struct sockaddr *addr,
  IN INT32 namelen
  )
{
  return bind ((INT32)s, (CONST struct sockaddr *)addr, namelen);
}

INT32
EFIAPI
SecSocketListen (
  IN EMU_SOCKET s,
  IN INT32 backlog
  )
{
  return (INT32) listen ((INT32)s, (int) backlog);
}

EMU_SOCKET
EFIAPI
SecSocketAccept (
  IN EMU_SOCKET s,
  OUT struct sockaddr *addr,
  IN OUT INT32 *addrlen
  )
{
  return accept ((INT32)s, (struct sockaddr *)addr, (int*) addrlen);
}

INT32
EFIAPI
SecSocketConnect (
  IN EMU_SOCKET s,
  IN CONST struct sockaddr *name,
  IN INT32 namelen
  )
{
  return (INT32) connect ((INT32)s, (CONST struct sockaddr *)name, (int) namelen);
}

INT32
EFIAPI
SecSocketSend (
  IN EMU_SOCKET s,
  IN CONST UINT8 * buf,
  IN INT32 len,
  IN INT32 flags
  )
{
  return (INT32) send ((INT32)s, (const char *) buf, (int) len, (int) flags);
}

INT32
EFIAPI
SecSocketReceive (
  IN EMU_SOCKET s,
  OUT UINT8 * buf,
  IN INT32 len,
  IN INT32 flags
  )
{
  return (INT32) recv ((INT32)s, (char *) buf, (int) len, (int) flags);
}

INT32
EFIAPI
SecSocketClose (
  IN EMU_SOCKET s
  )
{
  return (INT32) close ((INT32)s);
}

EMU_SOCKET_THUNK_PROTOCOL gEmuSocketThunkProtocol = {
  SecSocketCreate,
  SecSocketBind,
  SecSocketListen,
  SecSocketAccept,
  SecSocketConnect,
  SecSocketSend,
  SecSocketReceive,
  SecSocketClose,
  SecSocketStartup,
  SecSocketCleanup,
  SecSocketGetLastError,
};
