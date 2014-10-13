#include "raw_ethernet_packet_buffer.h"


char data[1024];

int main(int argc, char *argv[])
{
  RawEthernetPacket p;
  RawEthernetPacketBuffer b(10);

  while (1) {
    p=RawEthernetPacket(data,90);
    b.PushPacket(&p);
  }
}
