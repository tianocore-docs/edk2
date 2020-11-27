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
 *         A RDC stack protocol implemented in UEFI
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

#include "net/mac/uefi_rdc.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  printf ("uefi_rdc.send_packet ()\n");
  // JYAO1: TBD
  return ;
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  printf ("uefi_rdc.send_list ()\n");
  // JYAO1: TBD
  return ;
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
  printf ("uefi_rdc.packet_input ()\n");
  // JYAO1: TBD
  return ;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  printf ("uefi_rdc.on ()\n");
  // JYAO1: TBD
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  printf ("uefi_rdc.off ()\n");
  // JYAO1: TBD
  return 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  printf ("uefi_rdc.channel_check_interval ()\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  printf ("uefi_rdc.init ()\n");
  // JYAO1: TBD
  return;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver uefi_rdc_driver = {
  "nullrdc",
  init,
  send_packet,
  send_list,
  packet_input,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
