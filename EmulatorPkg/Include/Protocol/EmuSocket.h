/**@file

 A protocol to export Win32 socket APIs.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __EMU_SOCKET_PROTOCOL_H__
#define __EMU_SOCKET_PROTOCOL_H__

#define EMU_SOCKET_PROTOCOL_GUID \
  { 0x7418693c, 0x47b7, 0x4abf, { 0xa9, 0x6f, 0xd0, 0xca, 0x25, 0x96, 0x4, 0x98 } }

typedef UINTN EMU_SOCKET;

typedef struct {
        UINT8  s[4];
} EMU_IN_ADDR;

typedef struct {
        UINT8  s[16];
} EMU_IN6_ADDR;

typedef struct {
        UINT16  sin_family;
        UINT16  sin_port;
        EMU_IN_ADDR sin_addr;
        UINT8   sin_zero[8];
} EMU_SOCKADDR_IN;

typedef struct {
        UINT16  sin6_family;
        UINT16  sin6_port;
        UINT32  sin6_flowinfo;
        EMU_IN6_ADDR sin6_addr;
        UINT32  sin6_scope_id;
} EMU_SOCKADDR_IN6;

typedef struct {
        UINT16  sa_family;
        UINT8   sa_data[14];
} EMU_SOCKADDR;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR   -1
#endif

#ifndef AF_INET
#define AF_INET	2
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM	1
#endif
#ifndef SOCK_DGRAM
#define SOCK_DGRAM	2
#endif
#ifndef IPPROTO_TCP
#define IPPROTO_TCP	6
#endif
#ifndef IPPROTO_UDP
#define IPPROTO_UDP	17
#endif

typedef
INT32
(EFIAPI *EMU_SOCKET_STARTUP) (
  VOID
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_CLEANUP) (
  VOID
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_GET_LAST_ERROR) (
  VOID
  );

typedef
EMU_SOCKET
(EFIAPI *EMU_SOCKET_CREATE) (
  IN INT32 af,
  IN INT32 type,
  IN INT32 protocol
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_BIND) (
  IN EMU_SOCKET s,
  IN CONST EMU_SOCKADDR *addr,
  IN INT32 namelen
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_LISTEN) (
  IN EMU_SOCKET s,
  IN INT32 backlog
  );

typedef
EMU_SOCKET
(EFIAPI *EMU_SOCKET_ACCEPT) (
  IN EMU_SOCKET s,
  OUT EMU_SOCKADDR *addr,
  IN OUT INT32 *addrlen
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_CONNECT) (
  IN EMU_SOCKET s,
  IN CONST EMU_SOCKADDR *name,
  IN INT32 namelen
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_SEND) (
  IN EMU_SOCKET s,
  IN CONST UINT8 * buf,
  IN INT32 len,
  IN INT32 flags
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_RECEIVE) (
  IN EMU_SOCKET s,
  OUT UINT8 * buf,
  IN INT32 len,
  IN INT32 flags
  );

typedef
INT32
(EFIAPI *EMU_SOCKET_CLOSE) (
  IN EMU_SOCKET s
  );

typedef struct {
  //
  // Standard Socket APIs
  //
  EMU_SOCKET_CREATE                         Create;
  EMU_SOCKET_BIND                           Bind;
  EMU_SOCKET_LISTEN                         Listen;
  EMU_SOCKET_ACCEPT                         Accept;
  EMU_SOCKET_CONNECT                        Connect;
  EMU_SOCKET_SEND                           Send;
  EMU_SOCKET_RECEIVE                        Receive;
  EMU_SOCKET_CLOSE                          Close;
  //
  // Additional Win32 Socket APIs
  //
  EMU_SOCKET_STARTUP                        Startup;
  EMU_SOCKET_CLEANUP                        Cleanup;
  EMU_SOCKET_GET_LAST_ERROR                 GetLastError;
} EMU_SOCKET_THUNK_PROTOCOL;

extern EMU_SOCKET_THUNK_PROTOCOL gEmuSocketThunkProtocol;

#endif // __EMU_SOCKET_PROTOCOL_H__