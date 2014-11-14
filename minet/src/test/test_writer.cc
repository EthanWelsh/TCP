#include <iostream>

#include "ethernet.h"
#include "arp.h"
#include "util.h"


IPAddress myip("10.10.10.10"), reqip("10.10.10.55");
EthernetAddr myeth("00:e0:98:83:91:8c");

int main(int argc, char *argv[])
{


  while (1) {
    ARPPacket request(ARPPacket::Request,
		      myeth,
		      myip,
		      ETHERNET_BLANK_ADDR,
		      reqip);

    EthernetHeader h;
    h.SetSrcAddr(myeth);
    h.SetDestAddr(ETHERNET_BROADCAST_ADDR);
    h.SetProtocolType(PROTO_ARP);
    request.PushHeader(h);

    RawEthernetPacket rawout(request);

    rawout.Serialize(fileno(stdout));

    sleep(1);
  }
}



