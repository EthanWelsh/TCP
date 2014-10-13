#ifndef _udp
#define _udp

#include <iostream>
#include <cstring>

#include "packet.h"
#include "ip.h"
#include "headertrailer.h"

const unsigned short UDP_SOURCE_PORT_NONE=0;


const unsigned short UDP_PORT_NONE=0;
const unsigned short UDP_PORT_ANY=0;

const unsigned int UDP_HEADER_LENGTH=8;
const unsigned int UDP_MAX_DATA = IP_PACKET_MAX_LENGTH-IP_HEADER_BASE_LENGTH-UDP_HEADER_LENGTH;

class UDPHeader : public Header {
 public:
  UDPHeader();
  UDPHeader(const UDPHeader &rhs);
  UDPHeader(const Header &rhs);
  UDPHeader(const Buffer &rhs);
  UDPHeader(const char *buf, const unsigned len);
  virtual ~UDPHeader();

  UDPHeader & operator=(const UDPHeader &rhs);


  void GetSourcePort(unsigned short &port) const;
  void SetSourcePort(const unsigned short &port, const Packet &p);

  void GetDestPort(unsigned short &port) const;
  void SetDestPort(const unsigned short &port, const Packet &p);

  void GetLength(unsigned short &len) const;
  void SetLength(const unsigned short &len, const Packet &p);

  unsigned short ComputeChecksum(const Packet &p) const;
  bool IsCorrectChecksum(const Packet &p) const;
  void RecomputeChecksum(const Packet &p);

  void GetChecksum(unsigned short &checksum) const;
  void SetChecksum(const unsigned short &checksum);

  std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const UDPHeader& L) {
    return L.Print(os);
  }
};


#endif
