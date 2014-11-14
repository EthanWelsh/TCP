#ifndef _ip
#define _ip

#include <iostream>
#include "headertrailer.h"
#include "packet.h"

struct IPAddress {
  unsigned addr;

  IPAddress();
  IPAddress(const char *s);
  IPAddress(const IPAddress &rhs);
  IPAddress(const unsigned rhs);
  IPAddress & operator=(const IPAddress &rhs);
  IPAddress & operator=(const unsigned rhs);
  std::ostream & Print(std::ostream &rhs) const;
  char     *operator&() const;
  bool operator==(const IPAddress &rhs) const;
  operator unsigned() const;
  void Serialize(const int fd) const;
  void Unserialize(const int fd);

  friend std::ostream &operator<<(std::ostream &os, const IPAddress& L) {
    return L.Print(os);
  }
};

char * IPAddressToString(const IPAddress &addr);

extern IPAddress MyIPAddr();

const IPAddress          IP_ADDRESS_BROADCAST(0xffffffffU);
const IPAddress          IP_ADDRESS_NONE(0x00000000U);
const IPAddress          IP_ADDRESS_ANY(IP_ADDRESS_NONE);
const IPAddress          IP_ADDRESS_LO("127.0.0.0");

const unsigned char      IP_HEADER_BASE_LENGTH_IN_WORDS=5;
const unsigned char      IP_HEADER_BASE_LENGTH=IP_HEADER_BASE_LENGTH_IN_WORDS*4;
const unsigned char      IP_HEADER_OPTION_MAX_LENGTH_IN_WORDS=10;
const unsigned char      IP_HEADER_OPTION_MAX_LENGTH=IP_HEADER_OPTION_MAX_LENGTH_IN_WORDS*4;
const unsigned char      IP_HEADER_MAX_LENGTH=IP_HEADER_BASE_LENGTH+IP_HEADER_OPTION_MAX_LENGTH;

const unsigned char      IP_HEADER_REQUIRED_VERSION=0x4;
const unsigned char      IP_HEADER_DEFAULT_TOS=0x0;
const unsigned short     IP_PACKET_MAX_LENGTH=65535;

const unsigned char      IP_HEADER_FLAG_RESERVED=0x4;
const unsigned char      IP_HEADER_FLAG_DONTFRAG=0x2;
const unsigned char      IP_HEADER_FLAG_MOREFRAG=0x1;
const unsigned char      IP_HEADER_FLAG_DEFAULT=IP_HEADER_FLAG_DONTFRAG;

const unsigned char      IP_HEADER_TTL_DEFAULT=64;

const unsigned char      IP_PROTO_IP=0;
const unsigned char      IP_PROTO_UDP=17;
const unsigned char      IP_PROTO_TCP=6;
const unsigned char      IP_PROTO_ICMP=1;
const unsigned char      IP_PROTO_RAW=255;



struct IPOptions {
  char data[IP_HEADER_OPTION_MAX_LENGTH];
  int len;
};


class IPHeader : public Header {

 public:
  IPHeader();
  IPHeader(const IPHeader &rhs);
  IPHeader(const Buffer &rhs);
  IPHeader(const char *buf, unsigned len);
  virtual ~IPHeader();
  IPHeader & operator=(const IPHeader &rhs);

  // This is a helper to determine the header length prior to
  // extracting it
  static unsigned EstimateIPHeaderLength(Packet &p);

  // 4 bit version fields
  // automatically set to
  void GetVersion(unsigned char &version) const;
  void SetVersion(const unsigned char &version);

  void GetHeaderLength(unsigned char &hlen) const;
  // note that the header length is recomputed automatically
  void SetHeaderLength(const unsigned char &hlen);

  void GetTOS(unsigned char &tos) const;
  void SetTOS(const unsigned char &tos);

  void GetTotalLength(unsigned short &len) const;
  // note that total length is automatically computed
  void SetTotalLength(const unsigned short &len);


  void GetID(unsigned short &id) const;
  void SetID(const unsigned short &id);

  void GetFlags(unsigned char &flags) const;
  void SetFlags(const unsigned char &flags);

  void GetFragOffset(unsigned short &offset) const;
  void SetFragOffset(const unsigned short &offset);

  void GetTTL(unsigned char &ttl) const;
  void SetTTL(const unsigned char &ttl);

  void GetProtocol(unsigned char &proto) const;
  void SetProtocol(const unsigned char &proto);

  unsigned short ComputeChecksum() const;
  bool IsChecksumCorrect() const;
  void RecomputeChecksum();

  void GetChecksum(unsigned short &checksum) const;
  // Note that this will be recomputed every time one of the set calls is run
  void SetChecksum(const unsigned short &checksum);

  void GetSourceIP(IPAddress &addr) const;
  void SetSourceIP(const IPAddress &addr);

  void GetDestIP(IPAddress &addr) const;
  void SetDestIP(const IPAddress &addr);


  void GetOptions(IPOptions &opt) const;
  void SetOptions(const IPOptions &opt);

  // use default serialize and unserialize

  std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const IPHeader& L) {
    return L.Print(os);
  }
};






#endif
