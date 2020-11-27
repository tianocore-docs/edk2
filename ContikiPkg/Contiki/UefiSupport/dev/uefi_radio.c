/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 */

/**
 * \file
 *         Implementation of the radio driver in UEFI
 *
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */
#include "contiki.h"
#include "dev/radio.h"

#include "net/packetbuf.h"
#include "net/netstack.h"

/*---------------------------------------------------------------------------*/
static int
init(void)
{
  printf ("uefi_radio_driver.init ()\n");
  // JYAO1 - TBD  
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  printf ("uefi_radio_driver.prepare ()\n");
  // JYAO1 - TBD  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  printf ("uefi_radio_driver.transmit ()\n");
  // JYAO1 - TBD  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  printf ("uefi_radio_driver.send ()\n");
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
read(void *buf, unsigned short bufsize)
{
  printf ("uefi_radio_driver.read ()\n");
  // JYAO1 - TBD  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  printf ("uefi_radio_driver.channel_clear ()\n");
  // JYAO1 - TBD  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  printf ("uefi_radio_driver.receiving_packet ()\n");
  // JYAO1 - TBD  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  printf ("uefi_radio_driver.pending_packet ()\n");
  // JYAO1 - TBD  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  printf ("uefi_radio_driver.on ()\n");
  // JYAO1 - TBD  
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  printf ("uefi_radio_driver.off ()\n");
  // JYAO1 - TBD  
  return 1;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver uefi_radio_driver = {
  init,
  prepare,
  transmit,
  send,
  read,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
};
/*---------------------------------------------------------------------------*/
