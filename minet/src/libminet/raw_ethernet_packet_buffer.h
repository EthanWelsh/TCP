#ifndef _raw_ethernet_packet_buffer
#define _raw_ethernet_packet_buffer

#include "raw_ethernet_packet.h"

struct RawEthernetPacketBuffer {
  int     bufsize;
  RawEthernetPacket *packet;
  int     curin;
  int     curout;

  RawEthernetPacketBuffer(int size);
  virtual ~RawEthernetPacketBuffer();

  bool IsEmpty() const ;
  bool IsFull() const ;
  size_t Numitems() const;

  int PushPacket(RawEthernetPacket *packet);
  int PullPacket(RawEthernetPacket *packet);
};


#define PACKETBUFFER_OK    0
#define PACKETBUFFER_FULL  1
#define PACKETBUFFER_EMPTY 2


#endif
