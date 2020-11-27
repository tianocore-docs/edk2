#include "contiki.h"
#include "sys/clock.h"
#include "sys/autostart.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/clock-isr.h"
#include "dev/button-sensor.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "debug.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#ifndef WITH_UIP
#define WITH_UIP 0
#endif
#if WITH_UIP
#include "ethernet.h"
#endif /* WITH_UIP */

#ifndef WITH_UIP6
#define WITH_UIP6 0
#endif
#if WITH_UIP6
#include "net/uip.h"
#include "net/uip-ds6.h"
#define PRINT6ADDR(addr) printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#endif /* WITH_UIP6 */

/*---------------------------------------------------------------------------*/
#if STARTUP_CONF_VERBOSE
#define PUTSTRING(...) writestring(__VA_ARGS__)
#define PUTHEX(...) writehex(__VA_ARGS__)
#define PUTBIN(...) writebin(__VA_ARGS__)
#define PUTCHAR(...) writechar(__VA_ARGS__)
#else
#define PUTSTRING(...)
#define PUTHEX(...)
#define PUTBIN(...)
#define PUTCHAR(...)
#endif
/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
extern volatile uint8_t sleep_flag;
#endif
/*---------------------------------------------------------------------------*/
extern rimeaddr_t rimeaddr_node_addr;
static uint16_t len;
/*---------------------------------------------------------------------------*/

unsigned short node_id;

#if ENERGEST_CONF_ON
static unsigned long irq_energest = 0;
#define ENERGEST_IRQ_SAVE(a) do { \
    a = energest_type_time(ENERGEST_TYPE_IRQ); } while(0)
#define ENERGEST_IRQ_RESTORE(a) do { \
    energest_type_set(ENERGEST_TYPE_IRQ, a); } while(0)
#else
#define ENERGEST_IRQ_SAVE(a) do {} while(0)
#define ENERGEST_IRQ_RESTORE(a) do {} while(0)
#endif

EFI_STATUS
EFIAPI
UefiContikiMacConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

extern UINT8                  mMyNodeId;

/*---------------------------------------------------------------------------*/
static void
fade(int l)
{
  volatile int i, a;
  int k, j;
  for(k = 0; k < 400; ++k) {
    j = k > 200 ? 400 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      a = i;
    }
    leds_off(l);
    for(i = 0; i < 200 - j; ++i) {
      a = i;
    }
  }
}

VOID
UefiIvInit ();

EFI_STATUS
UefiIvSetup (
  VOID
  );

VOID
UefiIvTeardown (
  VOID
  );
/*---------------------------------------------------------------------------*/
#if WITH_UIP

#endif /* WITH_UIP */
/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  rimeaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(rimeaddr_t));
#if WITH_UIP6
  for(i = 0; i < sizeof(uip_lladdr.addr); i += 2) {
    addr.u8[i + 1] = node_id & 0xff;
    addr.u8[i + 0] = node_id >> 8;
  }
#else /* WITH_UIP6 */
  addr.u8[0] = node_id & 0xff;
  addr.u8[1] = node_id >> 8;
#endif /* WITH_UIP6 */
  rimeaddr_set_node_addr(&addr);
  printf("Rime started with address ");
  uip_debug_rimeaddr_print (&addr);
  printf("\n");
}
/*---------------------------------------------------------------------------*/
int
main(void)
{
  /* Hardware initialization */
  clock_init();
  rtimer_init();

  /* Init LEDs here */
  leds_init();
  leds_off(LEDS_ALL);
  fade(LEDS_GREEN);

  /* initialize process manager. */
  process_init();

  PUTSTRING("##########################################\n");
  writestring(CONTIKI_VERSION_STRING "\n");

  writestring("x86 platform\n");

#if STARTUP_CONF_VERBOSE
  PUTCHAR('\n');

  PUTSTRING(" Net: ");
  PUTSTRING(NETSTACK_NETWORK.name);
  PUTCHAR('\n');
  PUTSTRING(" MAC: ");
  PUTSTRING(NETSTACK_MAC.name);
  PUTCHAR('\n');
  PUTSTRING(" RDC: ");
  PUTSTRING(NETSTACK_RDC.name);
  PUTCHAR('\n');

  printf ("packetbuf_dataptr - 0x%p\n", packetbuf_dataptr());

//#define SICSLOWPAN_IP_BUF   ((struct uip_ip_hdr *)&sicslowpan_buf[UIP_LLH_LEN])
//  printf ("SICSLOWPAN_IP_BUF - 0x%p\n", SICSLOWPAN_IP_BUF);

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
  printf ("UIP_IP_BUF - 0x%p\n", UIP_IP_BUF);

  PUTSTRING("##########################################\n");
#endif

  watchdog_init();

  /* Initialise the H/W RNG engine. */
  random_init(0);

  /* start services */
  process_start(&etimer_process, NULL);
  ctimer_init();

  /* Print startup information */
  printf(CONTIKI_VERSION_STRING " started. ");
  if(node_id > 0) {
    printf("Node id is set to %d.\n", node_id);
  } else {
    printf("Node id is not set.\n");
  }


  /* initialize the netstack */
  set_rime_addr();
#if WITH_UIP6
  {
    uint8_t longaddr[8];
    uint16_t shortaddr;
    
    shortaddr = (rimeaddr_node_addr.u8[0] << 8) +
      rimeaddr_node_addr.u8[1];
    memset(longaddr, 0, sizeof(longaddr));
    rimeaddr_copy((rimeaddr_t *)&longaddr, &rimeaddr_node_addr);
    printf("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
           longaddr[0], longaddr[1], longaddr[2], longaddr[3],
           longaddr[4], longaddr[5], longaddr[6], longaddr[7]);
  }
#endif

  queuebuf_init();

  /* Initialize communication stack */
  netstack_init();
  printf("%a/%a/%a, channel check rate %d Hz\n",
	 NETSTACK_NETWORK.name, NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                         NETSTACK_RDC.channel_check_interval()));

#if WITH_UIP
  /* IPv4 CONFIGURATION */
  {
    static struct ethernet_config config = {0xAFAF, "uefi_eth"};
    uip_ipaddr_t addr;

    uip_ipaddr(&addr, 192,168,0,rimeaddr_node_addr.u8[0]);
    uip_sethostaddr(&addr);

    printf("IPv4 address: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));

    uip_ipaddr(&addr, 255,255,255,0);
    uip_setnetmask(&addr);

    uip_ipaddr(&addr, 192,168,0,1);
    uip_setdraddr(&addr);

    uip_ipaddr(&addr, 192,168,0,1);
    resolv_conf(&addr);

    process_start(&tcpip_process, NULL);
//    process_start(&ethernet_process, (void *)&config);
    
    ethernet_init(&config);
    tcpip_set_outputfunc(ethernet_output);
  }
#endif /* WITH_UIP */

#if WITH_UIP6
  /* IPv6 CONFIGURATION */
  {
    int i;
    uint8_t addr[sizeof(uip_lladdr.addr)];
    for(i = 0; i < sizeof(uip_lladdr.addr); i += 2) {
      addr[i + 1] = node_id & 0xff;
      addr[i + 0] = node_id >> 8;
    }
    rimeaddr_copy((rimeaddr_t *)addr, &rimeaddr_node_addr);
    memcpy(&uip_lladdr.addr, addr, sizeof(uip_lladdr.addr));

    process_start(&tcpip_process, NULL);

    printf("Tentative link-local IPv6 address ");
    {
      uip_ds6_addr_t *lladdr;
      int i;
      lladdr = uip_ds6_get_link_local(-1);
      for(i = 0; i < 7; ++i) {
	printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
	       lladdr->ipaddr.u8[i * 2 + 1]);
      }
      printf("%02x%02x\n", lladdr->ipaddr.u8[14],
	     lladdr->ipaddr.u8[15]);
    }

    if(1) {
      uip_ipaddr_t ipaddr;
      int i;
      uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
      uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
      uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
      printf("Tentative global IPv6 address ");
      for(i = 0; i < 7; ++i) {
        printf("%02x%02x:",
               ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
      }
      printf("%02x%02x\n",
             ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
    }
  }
#endif /* WITH_UIP6 */


  autostart_start(autostart_processes);

  watchdog_start();

  fade(LEDS_YELLOW);

  UefiIvInit ();
  UefiIvSetup ();
#if 0
  while(1) {
    uint8_t r;
    do {
      /* Reset watchdog and handle polls and events */
      watchdog_periodic();

#if CLOCK_CONF_STACK_FRIENDLY
      if(sleep_flag) {
        if(etimer_pending() &&
            (etimer_next_expiration_time() - clock_time() - 1) > MAX_TICKS) {
          etimer_request_poll();
        }
        sleep_flag = 0;
      }
#endif
      r = process_run();
    } while(r > 0);

#if 0
    len = NETSTACK_RADIO.pending_packet();
    if(len) {
      packetbuf_clear();
      len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_RDC.input();
      }
    }
#endif
  }

  UefiIvTeardown ();
#endif
}
/*---------------------------------------------------------------------------*/
VOID
PlatformIsrHandler (
  VOID
  )
{
  uint8_t r;

  do {
    /* Reset watchdog and handle polls and events */
    watchdog_periodic();
    r = process_run();
  } while(r > 0);

#ifdef UEFI_NETSTACK_MAC
    // JYAO1
#if WITH_UIP
  pollhandler ();
#endif
#if WITH_UIP6
  NETSTACK_MAC.input();
#endif
#endif
}

EFI_STATUS
EFIAPI
ContikiMainEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  UefiContikiMacConstructor (ImageHandle, SystemTable);
  node_id = mMyNodeId;

  main ();

  return EFI_SUCCESS;
}
