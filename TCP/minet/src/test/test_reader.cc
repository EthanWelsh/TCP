#include <cstdio>
#include "raw_ethernet_packet.h"
#include "util.h"


int main()
{
  RawEthernetPacket p;

  while (1) {
    p.Unserialize(fileno(stdin));
    p.Print(fileno(stdout));
  }
}


