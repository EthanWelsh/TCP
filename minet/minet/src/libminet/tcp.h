#ifndef _tcp
#define _tcp

#include <iostream>
#include <cstring>

#include "config.h"
#include "packet.h"
#include "ip.h"
#include "headertrailer.h"


const unsigned TCP_HEADER_BASE_LENGTH=20;
const unsigned TCP_HEADER_OPTION_MAX_LENGTH=40;
const unsigned TCP_HEADER_MAX_LENGTH=TCP_HEADER_BASE_LENGTH+TCP_HEADER_OPTION_MAX_LENGTH;
const unsigned TCP_HEADER_OPTION_KIND_END=0;
const unsigned TCP_HEADER_OPTION_KIND_END_LEN=1;
const unsigned TCP_HEADER_OPTION_KIND_NOP=1;
const unsigned TCP_HEADER_OPTION_KIND_NOP_LEN=1;
const unsigned TCP_HEADER_OPTION_KIND_MSS=2;
const unsigned TCP_HEADER_OPTION_KIND_MSS_LEN=4;
const unsigned TCP_HEADER_OPTION_KIND_WSF=3;
const unsigned TCP_HEADER_OPTION_KIND_WSF_LEN=3;
const unsigned TCP_HEADER_OPTION_KIND_TS=8;
const unsigned TCP_HEADER_OPTION_KIND_TS_LEN=10;

const unsigned short TCP_PORT_NONE=0;
const unsigned short TCP_PORT_ANY=0;



struct TCPOptions {
public:
  unsigned len;
  char     data[TCP_HEADER_OPTION_MAX_LENGTH];
};


class TCPHeader : public Header {
public:
  TCPHeader();
  TCPHeader(const TCPHeader &rhs);
  TCPHeader(const Header &rhs);
  TCPHeader(const Buffer &rhs);
  TCPHeader(const char *buf, const unsigned len);
  virtual ~TCPHeader();

  TCPHeader & operator=(const TCPHeader &rhs);

  // This is a helper to determine the header length prior to
  // extracting it
  static unsigned EstimateTCPHeaderLength(Packet &p);


  void GetSourcePort(unsigned short &port) const;
  void SetSourcePort(const unsigned short &port, const Packet &p);

  void GetDestPort(unsigned short &port) const;
  void SetDestPort(const unsigned short &port, const Packet &p);

  void GetSeqNum(unsigned int &n) const;
  void SetSeqNum(const unsigned int &n, const Packet &p);

  void GetAckNum(unsigned int &n) const;
  void SetAckNum(const unsigned int &n, const Packet &p);

  void GetHeaderLen(unsigned char &l) const;
  void SetHeaderLen(const unsigned char &l, const Packet &p);

  void GetFlags(unsigned char &f) const;
  void SetFlags(const unsigned char &f, const Packet &p);

  void GetWinSize(unsigned short &w) const;
  void SetWinSize(const unsigned short &w, const Packet &p);

  unsigned short ComputeChecksum(const Packet &p) const;
  bool IsCorrectChecksum(const Packet &p) const;
  void RecomputeChecksum(const Packet &p);

  void GetChecksum(unsigned short &checksum) const;
  void SetChecksum(const unsigned short &checksum);

  void GetUrgentPtr(unsigned short &up) const;
  void SetUrgentPtr(const unsigned short &up, const Packet &p);

  void GetOptions(struct TCPOptions &o) const;
  void SetOptions(const struct TCPOptions &o);


  std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const TCPHeader& L) {
    return L.Print(os);
  }
};

inline bool IS_URG(const unsigned char &f) { return f&32; };
inline bool IS_ACK(const unsigned char &f) { return f&16; };
inline bool IS_PSH(const unsigned char &f) { return f&8; };
inline bool IS_RST(const unsigned char &f) { return f&4; };
inline bool IS_SYN(const unsigned char &f) { return f&2; };
inline bool IS_FIN(const unsigned char &f) { return f&1; };

inline void SET_URG(unsigned char &f) { f|=32; };
inline void SET_ACK(unsigned char &f) { f|=16; };
inline void SET_PSH(unsigned char &f) { f|=8; };
inline void SET_RST(unsigned char &f) { f|=4; };
inline void SET_SYN(unsigned char &f) { f|=2; };
inline void SET_FIN(unsigned char &f) { f|=1; };

inline void CLR_URG(unsigned char &f) { f&=255-32; };
inline void CLR_ACK(unsigned char &f) { f&=255-16; };
inline void CLR_PSH(unsigned char &f) { f&=255-8; };
inline void CLR_RST(unsigned char &f) { f&=255-4; };
inline void CLR_SYN(unsigned char &f) { f&=255-2; };
inline void CLR_FIN(unsigned char &f) { f&=255-1; };

#endif
