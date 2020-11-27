/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * Author: jiewen yao <jiewen.yao@intel.com>
 *
 */

#include "contiki-net.h"
#include "ethernet-drv.h"
#include "ethernet.h"

#include "net/uip_arp.h"
#include "net/uip-debug.h"

#include <Library/DebugLib.h>

#define DEBUG 1

extern UINT8                  mMyNodeId;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

VOID
EnqueueMessage (
  IN UINT8 *SendBuffer,
  IN UINTN SendBufferSize
  );

BOOLEAN
DequeueMessage (
  IN OUT UINT8 *ReceiveBuffer,
     OUT UINTN *ReceiveBufferSize
  );

/*---------------------------------------------------------------------------*/
void
ethernet_init(struct ethernet_config *config)
{
  struct uip_eth_addr ethernet_address = {0x00};
  int i;

  printf ("ethernet_init () - %a\n", config->name);

  for(i = 0; i < sizeof(ethernet_address.addr); i += 2) {
    ethernet_address.addr[i + 1] = mMyNodeId;
    ethernet_address.addr[i + 0] = 0;
  }

  printf("ethernet: %02x:%02x:%02x:%02x:%02x:%02x\n",
    ethernet_address.addr[0],
    ethernet_address.addr[1],
    ethernet_address.addr[2],
    ethernet_address.addr[3],
    ethernet_address.addr[4],
    ethernet_address.addr[5]);

  // JYAO1: TBD
  uip_setethaddr(ethernet_address);
}
/*---------------------------------------------------------------------------*/
uint16_t
ethernet_poll(void)
{
  UINTN ReceiveBufferSize;
  UINT8 *ReceiveBuffer;

#if DEBUG
  static UINTN Count = 0;

  if (Count % 0x10000000 == 0) {
    printf ("ethernet_poll () - 0x%x\n", Count);
  }
  Count ++;
#endif

  ReceiveBuffer = uip_buf;
  
  //
  // This will be called periodically.
  // So we need check if there is pending packet
  //
  if (DequeueMessage (ReceiveBuffer, &ReceiveBufferSize)) {
    printf ("ethernet_poll (XXX)\n");
    ASSERT (ReceiveBufferSize >= sizeof(struct uip_eth_hdr));
#if DEBUG
    {
      UINTN Index;

      printf ("Head:\n");
      for (Index = 0; Index < sizeof(struct uip_eth_hdr); Index++) {
        printf ("%02x", ReceiveBuffer[Index]);
      }
      printf ("\n");
      printf ("Data:\n");
      for (Index = sizeof(struct uip_eth_hdr); Index < ReceiveBufferSize; Index++) {
        printf ("%02x", ReceiveBuffer[Index]);
      }
      printf ("\n");
    }
#endif

    printf ("  addr - ");
    uip_debug_lladdr_print(&BUF->src);
    printf (" => ");
    uip_debug_lladdr_print(&BUF->dest);
    printf ("\n");

    return (uint16_t)(ReceiveBufferSize);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
ethernet_send(void)
{
  UINT8 *SendBuffer;
  UINTN SendBufferSize;

  printf ("ethernet_send ()\n");

  SendBuffer = uip_buf;
  SendBufferSize = uip_len;

#if DEBUG
  {
    UINTN Index;
    printf ("Head:\n");
    for (Index = 0; Index < sizeof(struct uip_eth_hdr); Index++) {
      printf ("%02x", SendBuffer[Index]);
    }
    printf ("\n");
    printf ("Data:\n");
    for (Index = sizeof(struct uip_eth_hdr); Index < SendBufferSize; Index++) {
      printf ("%02x", SendBuffer[Index]);
    }
    printf ("\n");
  }

  printf ("  addr - ");
  uip_debug_lladdr_print(&BUF->src);
  printf (" => ");
  uip_debug_lladdr_print(&BUF->dest);
  printf ("\n");

#endif

  EnqueueMessage (SendBuffer, SendBufferSize);
  return ;
}
/*---------------------------------------------------------------------------*/
void
ethernet_exit(void)
{
  printf ("ethernet_exit ()\n");
}
/*---------------------------------------------------------------------------*/
