/*
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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include <string.h>
#include <stdio.h>

#include "dev/button-sensor.h"
#include "debug.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8

#define PING4_NB 5
#define PING4_DATALEN 16

#define UIP_ICMP4_ECHO_REQUEST_LEN        4

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])

static struct etimer ping4_periodic_timer;
static uint8_t count = 0;
static uip_ipaddr_t dest_addr;

PROCESS(ping4_process, "PING4 process");
/*---------------------------------------------------------------------------*/
static void
ping4handler()
{
  if(count < PING4_NB) {
    PRINTF("ping4handler\n");
    UIP_IP_BUF->vhl = 0x45;
    UIP_IP_BUF->tos = 0;
    UIP_IP_BUF->ipid[0] = 0;
    UIP_IP_BUF->ipid[1] = 0;
    UIP_IP_BUF->ipoffset[0] = 0x40;
    UIP_IP_BUF->ipoffset[1] = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP;
    UIP_IP_BUF->ttl = 2; // uip_ds6_if.cur_hop_limit;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);
    uip_gethostaddr (&UIP_IP_BUF->srcipaddr);

    UIP_ICMP_BUF->type = ICMP_ECHO;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP4_ECHO_REQUEST_LEN,
        count, PING4_DATALEN);


    uip_len = UIP_ICMPH_LEN + UIP_ICMP4_ECHO_REQUEST_LEN + UIP_IPH_LEN
        + PING4_DATALEN;
    UIP_IP_BUF->len[0] = (uint8_t)(uip_len >> 8);
    UIP_IP_BUF->len[1] = (uint8_t)(uip_len & 0x00FF);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmpchksum();


    PRINTF("Echo Request to ");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF(" from ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");
    UIP_STAT(++uip_stat.icmp.sent);

    tcpip_output();

    count++;
    etimer_set(&ping4_periodic_timer, 3 * CLOCK_SECOND);
  } else {
    count = 0;
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ping4_process, ev, data)
{

  PROCESS_BEGIN();
  PRINTF("ping4 running.\n");
  PRINTF("Button 1: 5 pings 16 byte payload.\n");

  uip_ipaddr(&dest_addr, 192, 168, 0, 1);
  count = 0;

  etimer_set(&ping4_periodic_timer, 3*CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&ping4_periodic_timer)) {
      ping4handler();
    }
    if(*(uint8_t *)data == ICMP_ECHO_REPLY) {
      PRINTF("Echo Reply\n");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
