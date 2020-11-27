This is to run uIPv6 stack above X86. (6LoWPAN)

Porting:

Category  Module     UEFI
==========================
Clock     timer      Timer AP
Clock     rtimer     8054
Platform  watchdog   WatchDog AP
Platform  Led        N/A
Platform  Debug      Serial
Network   Radio      N/A
Network   RDC/MAC    UEFI version

Size:
Contiki
  40,064 ContikiUdpIp6Client.efi
  38,976 ContikiUdpIp6Server.efi
UEFI network
  26624 Udp6Dxe.efi
  57344 Ip6Dxe.efi
  28672 MnpDxe.efi
  Total: 112640
Conclusion – Contiki is 1/3 of UEFI network
