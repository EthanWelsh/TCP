#ifndef _raw_ethernet_packet
#define _raw_ethernet_packet
#include <iostream>
#include "config.h"
#include "packet.h"

class Packet;

struct RawEthernetPacket {
  size_t   size;
  char     data[ETHERNET_PACKET_LEN];

  RawEthernetPacket();
  RawEthernetPacket(const RawEthernetPacket &rhs);
  RawEthernetPacket(const Packet &rhs);
  RawEthernetPacket(const char *data, const size_t size);
  const RawEthernetPacket & operator= (const RawEthernetPacket &rhs);
  const RawEthernetPacket & operator= (const Packet &rhs);
  virtual ~RawEthernetPacket();

  Packet & ConvertToPacket() const;

  void Serialize(const int fd) const;
  void Unserialize(const int fd);

  void Print(unsigned size=ETHERNET_PACKET_LEN, FILE *out=stdout) const;
  std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const RawEthernetPacket& L) {
    return L.Print(os);
  }
};
#endif
