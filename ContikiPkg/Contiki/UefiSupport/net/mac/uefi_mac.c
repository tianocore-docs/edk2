/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A MAC stack protocol implemented in UEFI
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

#include "net/mac/uefi_mac.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/SynchronizationLib.h>
#include <Library/DebugLib.h>

#define DEBUG 1
#include "net/uip-debug.h"

#define FAKE_MAC_NODE_COUNT  4
#define FAKE_MAC_MESSAGE_COUNT  256

typedef struct {
  UINTN                 DataSize;
  UINT8                 Data[PACKETBUF_SIZE + PACKETBUF_HDR_SIZE];
} FAKE_MAC_MESSAGE;

typedef struct {
  //
  //       WriteIndex
  //            |
  //            V
  // +---------------+
  // | | |D|D|D| | | |
  // +---------------+
  //      ^
  //      |
  //  ReadIndex
  //
  // WriteIndex is pointer to first invalid message
  // ReadIndex is pointer to fist valid message 
  //
  // [ReadIndex, WriteIndex) is valid message queue.
  // [WriteIndex, ReadIndex) is invalid message queue.
  //
  // If WriteIndex == ReadIndex, it means queue is empty.
  // If WriteIndex + 1 == ReadIndex, it means queue is full
  //
  SPIN_LOCK             Lock;
  UINT8                 WriteIndex;
  UINT8                 ReadIndex;
  FAKE_MAC_MESSAGE      Message[FAKE_MAC_MESSAGE_COUNT];                 
} FAKE_MAC_NODE_ENTRY;

typedef struct {
  UINT8                 GlobalIdCount;
  FAKE_MAC_NODE_ENTRY   NodeEntry[FAKE_MAC_NODE_COUNT];
} FAKE_MAC_SHARE_MEMORY;

FAKE_MAC_SHARE_MEMORY  *mFakeMacShareMemoryPtr;
UINT8                  mMyNodeId;

#define FAKE_MAC_PROTOCOL_GUID { \
  0xb9f332e, 0xe553, 0x4683, 0xab, 0x22, 0x3f, 0x30, 0x65, 0xbb, 0x8f, 0x14 \
}

EFI_GUID  gFakeMacProtocolGuid = FAKE_MAC_PROTOCOL_GUID;

VOID
EnqueueMessage (
  IN UINT8 *SendBuffer,
  IN UINTN SendBufferSize
  )
{
  UINTN               Index;
  FAKE_MAC_NODE_ENTRY *NodeEntry;
  FAKE_MAC_MESSAGE    *Message;

  for (Index = 0; Index < mFakeMacShareMemoryPtr->GlobalIdCount; Index++) {
    if (Index + 1 == mMyNodeId) {
      continue;
    }
    NodeEntry = &mFakeMacShareMemoryPtr->NodeEntry[Index];
#if 0
    AcquireSpinLock (&NodeEntry->Lock);
#else
    if (!AcquireSpinLockOrFail (&NodeEntry->Lock)) {
      printf ("AcquireSpinLock fail\n", Index + 1);
      continue;
    }
#endif
    if ((UINT8)(NodeEntry->WriteIndex + 1) == NodeEntry->ReadIndex) {
      ReleaseSpinLock (&NodeEntry->Lock);
      // Full, discard it
      printf ("Message Full\n", Index + 1);
      continue;
    }
    Message = &NodeEntry->Message[NodeEntry->WriteIndex];
    ASSERT (sizeof(Message->Data) >= SendBufferSize);
    CopyMem (Message->Data, SendBuffer, SendBufferSize);
    Message->DataSize = SendBufferSize;
    NodeEntry->WriteIndex ++;
    ReleaseSpinLock (&NodeEntry->Lock);
  }
  return ;
}

BOOLEAN
DequeueMessage (
  IN OUT UINT8 *ReceiveBuffer,
     OUT UINTN *ReceiveBufferSize
  )
{
  FAKE_MAC_NODE_ENTRY *NodeEntry;
  FAKE_MAC_MESSAGE    *Message;

  NodeEntry = &mFakeMacShareMemoryPtr->NodeEntry[mMyNodeId - 1];
#if 0
  AcquireSpinLock (&NodeEntry->Lock);
#else
  if (!AcquireSpinLockOrFail (&NodeEntry->Lock)) {
    printf ("AcquireSpinLock fail\n");
    return FALSE;
  }
#endif
  if (NodeEntry->WriteIndex == NodeEntry->ReadIndex) {
    ReleaseSpinLock (&NodeEntry->Lock);
//    printf ("No Message\n");
    return FALSE;
  }
  Message = &NodeEntry->Message[NodeEntry->ReadIndex];
  CopyMem (ReceiveBuffer, Message->Data, Message->DataSize);
  *ReceiveBufferSize = Message->DataSize;
  NodeEntry->ReadIndex ++;
  ReleaseSpinLock (&NodeEntry->Lock);

  return TRUE;
}

/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  UINT8 *SendBuffer;
  UINTN SendBufferSize;
  const rimeaddr_t *addr_receiver;
  const rimeaddr_t *addr_sender;
  UINT8 *PacketBuf;

  printf ("uefi_mac.send_packet ()\n");

  SendBuffer = packetbuf_dataptr();
  SendBufferSize = packetbuf_datalen();

  PacketBuf = SendBuffer - PACKETBUF_HDR_SIZE;

#if DEBUG
  {
    UINTN Index;
    printf ("Head:\n");
    for (Index = 0; Index < PACKETBUF_HDR_SIZE; Index++) {
      printf ("%02x", PacketBuf[Index]);
    }
    printf ("\n");
    printf ("Data:\n");
    for (Index = 0; Index < SendBufferSize; Index++) {
      printf ("%02x", SendBuffer[Index]);
    }
    printf ("\n");
  }
#endif

  addr_receiver = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  //addr_sender = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  addr_sender = &rimeaddr_node_addr;
  printf ("  addr - ");
  uip_debug_rimeaddr_print(addr_sender);
  printf (" => ");
  uip_debug_rimeaddr_print(addr_receiver);
  printf ("\n");

  CopyMem (PacketBuf, addr_receiver, sizeof(rimeaddr_t));
  ZeroMem (PacketBuf + sizeof(rimeaddr_t), sizeof(uip_lladdr_t) - sizeof(rimeaddr_t));
  CopyMem (PacketBuf + sizeof(uip_lladdr_t), addr_sender, sizeof(rimeaddr_t));
  ZeroMem (PacketBuf + sizeof(uip_lladdr_t) + sizeof(rimeaddr_t), sizeof(uip_lladdr_t) - sizeof(rimeaddr_t));

  EnqueueMessage (PacketBuf, PACKETBUF_HDR_SIZE + SendBufferSize);
  return;
}

/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  UINTN ReceiveBufferSize;
  UINT8 *ReceiveBuffer;
  rimeaddr_t addr_receiver;
  rimeaddr_t addr_sender;
  UINT8 *PacketBuf;

#if DEBUG
  static UINTN Count = 0;

  if (Count % 0x10000000 == 0) {
    printf ("uefi_mac.input_packet () - 0x%x\n", Count);
  }
  Count ++;
#endif

  ReceiveBuffer = packetbuf_dataptr();
  PacketBuf = ReceiveBuffer - PACKETBUF_HDR_SIZE;

  //
  // This will be called periodically.
  // So we need check if there is pending packet
  //
  if (DequeueMessage (PacketBuf, &ReceiveBufferSize)) {
    printf ("uefi_mac.input_packet (XXX)\n");
#if DEBUG
    {
      UINTN Index;

      ReceiveBufferSize -= PACKETBUF_HDR_SIZE;

      printf ("Head:\n");
      for (Index = 0; Index < PACKETBUF_HDR_SIZE; Index++) {
        printf ("%02x", PacketBuf[Index]);
      }
      printf ("\n");
      printf ("Data:\n");
      for (Index = 0; Index < ReceiveBufferSize; Index++) {
        printf ("%02x", ReceiveBuffer[Index]);
      }
      printf ("\n");
    }
#endif
    CopyMem (&addr_receiver, PacketBuf, sizeof(rimeaddr_t));
    CopyMem (&addr_sender, PacketBuf + sizeof(uip_lladdr_t), sizeof(rimeaddr_t));

    printf ("  addr - ");
    uip_debug_rimeaddr_print(&addr_sender);
    printf (" => ");
    uip_debug_rimeaddr_print(&addr_receiver);
    printf ("\n");

    packetbuf_set_addr (PACKETBUF_ADDR_SENDER, &addr_sender);
    packetbuf_set_addr (PACKETBUF_ADDR_RECEIVER, &addr_receiver);
    packetbuf_set_datalen (ReceiveBufferSize);

    NETSTACK_NETWORK.input();

    return;
  }
  return;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  printf ("uefi_mac.on ()\n");
  // JYAO1: TBD
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  printf ("uefi_mac.off ()\n");
  // JYAO1: TBD
  return 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  printf ("uefi_mac.channel_check_interval ()\n");
  // JYAO1: TBD
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  printf ("uefi_mac.init ()\n");
}
/*---------------------------------------------------------------------------*/
const struct mac_driver uefi_mac_driver = {
  "UEFI_MAC",
  init,
  send_packet,
  input_packet,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/

EFI_STATUS
EFIAPI
UefiContikiMacConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  UINTN      Index;
  EFI_HANDLE Handle;
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (
                  &gFakeMacProtocolGuid,
                  NULL,
                  &mFakeMacShareMemoryPtr
                  );
  if (EFI_ERROR (Status)) {
    mFakeMacShareMemoryPtr = AllocateZeroPool (sizeof(FAKE_MAC_SHARE_MEMORY));
    ASSERT (mFakeMacShareMemoryPtr != NULL);

    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gFakeMacProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    mFakeMacShareMemoryPtr
                    );

    for (Index = 0; Index < FAKE_MAC_NODE_COUNT; Index++) {
      InitializeSpinLock (&mFakeMacShareMemoryPtr->NodeEntry[Index].Lock);
    }
  }

  ASSERT (mFakeMacShareMemoryPtr->GlobalIdCount < FAKE_MAC_NODE_COUNT);
  mFakeMacShareMemoryPtr->GlobalIdCount ++;
  mMyNodeId = mFakeMacShareMemoryPtr->GlobalIdCount;

  return EFI_SUCCESS;
}
