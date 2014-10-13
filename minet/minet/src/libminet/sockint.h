#ifndef _sockint
#define _sockint


//This file documents the interface between the sock module and the
//adjacent modules, both the lower-level modules (TCP and UDP) and
//the minet stubs.
//
//the file "sockint.semantics" gives more information on the interface
//to lower-level modules.

#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "sock.h"
#include "config.h"
#include "buffer.h"
#include "ip.h"
#include "udp.h"
#include "tcp.h"

const unsigned NUM_SOCK_TYPES=6;
enum srrType {CONNECT=0, ACCEPT=1, WRITE=2, FORWARD=3, CLOSE=4, STATUS=5};
enum slrrType {mSOCKET, mBIND, mLISTEN, mACCEPT, mCONNECT, mREAD, mWRITE,
	       mRECVFROM, mSENDTO, mCLOSE, mSELECT, mPOLL, mSET_BLOCKING,
	       mSET_NONBLOCKING, mCAN_WRITE_NOW, mCAN_READ_NOW, mSTATUS};

const unsigned short PORT_NONE=0x0000;
const unsigned short PORT_ANY=PORT_NONE;

struct Connection {
  IPAddress src, dest;
  unsigned short srcport, destport;
  unsigned char  protocol;

  Connection(const IPAddress &s,
	     const IPAddress &d,
  	     const unsigned short sport,
	     const unsigned short destport,
	     const unsigned char  proto);
  Connection(const Connection &rhs);
  Connection();
  virtual ~Connection();
  Connection & operator=(const Connection &rhs);

  void Serialize(const int fd) const;
  void Unserialize(const int fd);

  bool MatchesSource(const Connection &rhs) const ;
  bool MatchesDest(const Connection &rhs) const ;
  bool MatchesProtocol(const Connection &rhs) const;
  bool Matches(const Connection &rhs) const;

  std::ostream & Print(std::ostream &rhs) const;

  friend std::ostream &operator<<(std::ostream &os, const Connection& L) {
    return L.Print(os);
  }
};

struct SockRequestResponse {
  srrType    type;
  Connection connection;
  Buffer     data;
  unsigned   bytes;
  int        error;
  SockRequestResponse(const srrType &t,
		      const Connection &c,
		      const Buffer &d,
		      const unsigned &b,
		      const int &err);
  SockRequestResponse(const SockRequestResponse &rhs);
  SockRequestResponse();
  virtual ~SockRequestResponse();
  SockRequestResponse & operator=(const SockRequestResponse &rhs);

  void Serialize(const int fd) const;
  void Unserialize(const int fd);

  std::ostream & Print(std::ostream &rhs) const;

  friend std::ostream &operator<<(std::ostream &os, const SockRequestResponse& L) {
    return L.Print(os);
  }
};

struct SockLibRequestResponse {
  slrrType   type;
  Connection connection;
  unsigned   sockfd;
  Buffer     data;
  unsigned   bytes;
  int        error;
  fd_set     readfds;
  fd_set     writefds;
  fd_set     exceptfds;

  SockLibRequestResponse(const slrrType &t,
			 const Connection &c,
			 const unsigned &s,
			 const Buffer &d,
			 const unsigned &b,
			 const int &err,
			 const int &num_minet_fds,
			 const struct pollfd &minet_fds);

  SockLibRequestResponse(const slrrType &t,
			 const Connection &c,
			 const unsigned &s,
			 const Buffer &d,
			 const unsigned &b,
			 const int &err,
			 const fd_set &r,
			 const fd_set &w,
			 const fd_set &e);

  SockLibRequestResponse(const slrrType &t,
			 const Connection &c,
			 const unsigned &s,
			 const Buffer &d,
			 const unsigned &b,
			 const int &err);

  SockLibRequestResponse(const SockLibRequestResponse &rhs);
  SockLibRequestResponse();
  virtual ~SockLibRequestResponse();
  SockLibRequestResponse & operator=(const SockLibRequestResponse
				     &rhs);

  void Serialize(const int fd) const;
  void Unserialize(const int fd);

  std::ostream & Print(std::ostream &rhs) const;

  friend std::ostream &operator<<(std::ostream &os, const SockLibRequestResponse& L) {
    return L.Print(os);
  }
};

#define EOK                       0
// #define EREQUEST_TOKEN   1
// #define ESUPPLY_TOKEN    2
#define EBASE                     4
#define ENOMATCH                  EBASE+1
#define EWHAT                     EBASE+2
#define EBUF_SPACE                EBASE+3
#define ESET_SELECT               EBASE+4
#define ECLEAR_SELECT             EBASE+5
// other errors are EBASE + positive int
#define EUNKNOWN                  EBASE+6
#define ERESOURCE_UNAVAIL         EBASE+7
#define EINVALID_OP               EBASE+8
#define ENOT_IMPLEMENTED          EBASE+9
#define ENOT_SUPPORTED            EBASE+10
#define EWOULD_BLOCK              EBASE+11
#define ECONN_FAILED              EBASE+12

#endif
