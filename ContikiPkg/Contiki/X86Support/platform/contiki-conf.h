#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "x86platformdef.h"
#include "sys/cc.h"
#include <string.h>

/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include "project-conf.h"
#endif /* PROJECT_CONF_H */

/*
 * Define this as 1 to poll the etimer process from within main instead of from
 * the clock ISR. This reduces the ISR's stack usage and may prevent crashes.
 */
#ifndef CLOCK_CONF_STACK_FRIENDLY
#define CLOCK_CONF_STACK_FRIENDLY 0
#endif

#ifndef STACK_CONF_DEBUGGING
#define STACK_CONF_DEBUGGING  0
#endif

/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON      0
#endif

/* Verbose Startup? Turning this off saves plenty of bytes of CODE in HOME */
#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE  1
#endif

/* More CODE space savings by turning off process names */
#define PROCESS_CONF_NO_PROCESS_NAMES 0

/* Code Shortcuts */
/*
 * When set, this directive also configures the following bypasses:
 *   - process_post_synch() in tcpip_input() (we call packet_input())
 *   - process_post_synch() in tcpip_uipcall (we call the relevant pthread)
 *   - mac_call_sent_callback() is replaced with sent() in various places
 *
 * These are good things to do, they reduce stack usage and prevent crashes
 */
#define NETSTACK_CONF_SHORTCUTS   1

/* Network Stack */
#ifndef NETSTACK_CONF_NETWORK
#if UIP_CONF_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver
#endif /* UIP_CONF_IPV6 */
#endif /* NETSTACK_CONF_NETWORK */

#ifdef UEFI_NETSTACK_MAC
#define NETSTACK_CONF_MAC     uefi_mac_driver
#define NETSTACK_CONF_RDC     uefi_rdc_driver
#define NETSTACK_CONF_RADIO   uefi_radio_driver
#endif

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NULLRDC_802154_AUTOACK 1
#define NULLRDC_802154_AUTOACK_HW 1
#endif

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#endif

#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif

#ifndef NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO   cc2530_rf_driver
#endif

/* RF Config */
#define IEEE802154_CONF_PANID 0x5449 /* TI */

#ifndef CC2530_RF_CONF_CHANNEL
#define CC2530_RF_CONF_CHANNEL    25
#endif /* CC2530_RF_CONF_CHANNEL */

#ifndef CC2530_RF_CONF_AUTOACK
#define CC2530_RF_CONF_AUTOACK 1
#endif /* CC2530_CONF_AUTOACK */

#if UIP_CONF_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define RIMEADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_LLH_LEN                     0
#define UIP_CONF_NETIF_MAX_ADDRESSES         3

/* TCP, UDP, ICMP */
#define UIP_CONF_TCP                         0
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif

#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#endif

#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS                 4 /* Handle n Neighbors */
#endif
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES               4 /* Handle n Routes */
#endif

/* uIP */
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE               240
#endif
#define UIP_CONF_IPV6_QUEUE_PKT              0
#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
//#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_IPV6
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 0 /* About 2KB of CODE if 1 */
#endif
#define SICSLOWPAN_CONF_MAXAGE               8

/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}

#define MAC_CONF_CHANNEL_CHECK_RATE          8

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    6
#endif

#else /* UIP_CONF_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1
#define UIP_CONF_BUFFER_SIZE               108
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0
#define QUEUEBUF_CONF_NUM                    8
#endif /* UIP_CONF_IPV6 */

/* Prevent SDCC compile error when UIP_CONF_ROUTER == 0 */
#if !UIP_CONF_ROUTER
#define UIP_CONF_DS6_AADDR_NBU               1
#endif



#define UIP_CONF_LOOPBACK_INTERFACE 1

#if 1

#if UIP_CONF_LL_802154
#define PACKETBUF_CONF_SIZE           128
#define PACKETBUF_CONF_HDR_SIZE       48
#else /*UIP_CONF_LL_802154*/
#if UIP_CONF_LL_80211
#define PACKETBUF_CONF_SIZE           1496
#define PACKETBUF_CONF_HDR_SIZE       14
#else /*UIP_CONF_LL_80211*/
// ether net
// #define UIP_LINK_MTU 1280
#define PACKETBUF_CONF_SIZE           1496
#define PACKETBUF_CONF_HDR_SIZE       14
#endif /*UIP_CONF_LL_80211*/
#endif /*UIP_CONF_LL_802154*/

#endif

#endif /* __CONTIKI_CONF_H__ */
