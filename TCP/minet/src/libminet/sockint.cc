#include "sockint.h"

Connection::Connection(const IPAddress &s,
		       const IPAddress &d,
		       const unsigned short sport,
		       const unsigned short dport,
		       const unsigned char  proto) :
  src(s), dest(d), srcport(sport), destport(dport), protocol(proto)
{}

Connection::Connection(const Connection &rhs) :
  src(rhs.src), dest(rhs.dest), srcport(rhs.srcport),
  destport(rhs.destport), protocol(rhs.protocol)
{}

Connection::Connection() :
  src(0U), dest(0U), srcport(0),
  destport(0), protocol(0)
{}

Connection::~Connection()
{}

Connection & Connection::operator=(const Connection &rhs)
{
  src=rhs.src;
  dest=rhs.dest;
  srcport=rhs.srcport;
  destport=rhs.destport;
  protocol=rhs.protocol;
  return *this;
}

bool Connection::MatchesSource(const Connection &rhs) const
{
  if (src==IP_ADDRESS_ANY || rhs.src==IP_ADDRESS_ANY) {
    if (srcport==PORT_ANY || rhs.srcport==PORT_ANY) {
      return true;
    } else {
      return srcport==rhs.srcport;
    }
  } else if (src==rhs.src) {
    if (srcport==PORT_ANY || rhs.srcport==PORT_ANY) {
      return true;
    } else {
      return srcport==rhs.srcport;
    }
  } else {
    return false;
  }
}

bool Connection::MatchesDest(const Connection &rhs) const
{
  if (dest==IP_ADDRESS_ANY || rhs.dest==IP_ADDRESS_ANY) {
    if (destport==PORT_ANY || rhs.destport==PORT_ANY) {
      return true;
    } else {
      return destport==rhs.destport;
    }
  } else if (dest==rhs.dest) {
    if (destport==PORT_ANY || rhs.destport==PORT_ANY) {
      return true;
    } else {
      return destport==rhs.destport;
    }
  } else {
    return false;
  }
}

bool Connection::MatchesProtocol(const Connection &rhs) const
{
  return rhs.protocol==protocol;
}

bool Connection::Matches(const Connection &rhs) const
{
  return MatchesSource(rhs) && MatchesDest(rhs) && MatchesProtocol(rhs);
}


void Connection::Serialize(const int fd) const
{
  src.Serialize(fd);
  dest.Serialize(fd);
  if (writeall(fd,(const char*)&srcport,sizeof(srcport))!=sizeof(srcport)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&destport,sizeof(destport))!=sizeof(destport)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&protocol,sizeof(protocol))!=sizeof(protocol)) {
    throw SerializationException();
  }
}

void Connection::Unserialize(const int fd)
{
  src.Unserialize(fd);
  dest.Unserialize(fd);
  if (readall(fd,(char*)&srcport,sizeof(srcport))!=sizeof(srcport)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&destport,sizeof(destport))!=sizeof(destport)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&protocol,sizeof(protocol))!=sizeof(protocol)) {
    throw SerializationException();
  }
}

std::ostream & Connection::Print(std::ostream &rhs) const
{
  rhs << "Connection(src="<<src<<":"<<srcport
      << ", dest="<<dest<<":"<<destport
      << ", protocol="<<((int)protocol)<<")";
  return rhs;
}


SockRequestResponse::SockRequestResponse(const srrType &t,
					 const Connection &c,
					 const Buffer &d,
					 const unsigned &b,
					 const int &err) :
  type(t), connection(c), data(d), bytes(b), error(err)
{}

SockRequestResponse::SockRequestResponse()
{}

SockRequestResponse::SockRequestResponse(const
					 SockRequestResponse &rhs) :
  type(rhs.type), connection(rhs.connection), data(rhs.data),
  bytes(rhs.bytes), error(rhs.error)
{}

SockRequestResponse::~SockRequestResponse()
{}

SockRequestResponse &
SockRequestResponse::operator=(const SockRequestResponse &rhs)
{
  type=rhs.type;
  connection=rhs.connection;
  data=rhs.data;
  bytes=rhs.bytes;
  error=rhs.error;
  return *this;
}

void SockRequestResponse::Serialize(const int fd) const
{
  int t=(int) type;
  if (writeall(fd,(const char*)&t,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
  connection.Serialize(fd);
  data.Serialize(fd);
  if (writeall(fd,(const char*)&bytes,sizeof(unsigned))!=sizeof(unsigned)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&error,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
}

void SockRequestResponse::Unserialize(const int fd)
{
  int t;
  if (readall(fd,(char*)&t,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
  type=(srrType)t;
  connection.Unserialize(fd);
  data.Unserialize(fd);
  if (readall(fd,(char*)&bytes,sizeof(unsigned))!=sizeof(unsigned)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&error,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
}

std::ostream & SockRequestResponse::Print(std::ostream &rhs) const
{
  rhs << "SockRequestResponse(type=";
  rhs << (type==ACCEPT ? "ACCEPT" :
	  type==CONNECT ? "CONNECT" :
	  type==WRITE ? "WRITE" :
	  type==FORWARD ? "FORWARD" :
	  type==CLOSE ? "CLOSE" :
	  type==STATUS ? "STATUS" :
	  "UNKNOWN");
  rhs << ", connection=" << connection;
  rhs << ", data=" << data;
  rhs << ", bytes=" << bytes;
  rhs << ", error=" << error ;
  rhs << ")";
  return rhs;
}

SockLibRequestResponse::SockLibRequestResponse(const slrrType &t,
					       const Connection &c,
					       const unsigned &s,
					       const Buffer &d,
					       const unsigned &b,
					       const int &err,
					       const fd_set &r,
					       const fd_set &w,
					       const fd_set &e) :
  type(t), connection(c), sockfd(s), data(d), bytes(b), error(err),
  readfds(r), writefds(w), exceptfds(e)
{}

SockLibRequestResponse::SockLibRequestResponse(const slrrType &t,
					       const Connection &c,
					       const unsigned &s,
					       const Buffer &d,
					       const unsigned &b,
					       const int &err,
					       const int &num_minet_fds,
					       const struct pollfd &minet_fds) :
  type(t), connection(c), sockfd(s), data(d), bytes(b), error(err),
  readfds(fd_set()), writefds(fd_set()), exceptfds(fd_set())
{
  // FIX THIS
}



SockLibRequestResponse::SockLibRequestResponse(const slrrType &t,
					       const Connection &c,
					       const unsigned &s,
					       const Buffer &d,
					       const unsigned &b,
					       const int &err) :
  type(t), connection(c), sockfd(s), data(d), bytes(b), error(err),
  readfds(fd_set()), writefds(fd_set()), exceptfds(fd_set())
{}

SockLibRequestResponse::SockLibRequestResponse()
{}

SockLibRequestResponse::SockLibRequestResponse(const
					       SockLibRequestResponse
					       &rhs) :
  type(rhs.type), connection(rhs.connection), sockfd(rhs.sockfd),
  data(rhs.data), bytes(rhs.bytes), error(rhs.error),
  readfds(rhs.readfds), writefds(rhs.writefds),
  exceptfds(rhs.exceptfds)
{}

SockLibRequestResponse::~SockLibRequestResponse()
{}

SockLibRequestResponse &
SockLibRequestResponse::operator=(const SockLibRequestResponse &rhs)
{
  type=rhs.type;
  connection=rhs.connection;
  sockfd=rhs.sockfd;
  data=rhs.data;
  bytes=rhs.bytes;
  error=rhs.error;
  readfds=rhs.readfds;
  writefds=rhs.writefds;
  exceptfds=rhs.exceptfds;
  return *this;
}

void SockLibRequestResponse::Serialize(const int fd) const
{
  int t=(int) type;
  if (writeall(fd,(const char*)&t,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
  connection.Serialize(fd);
  if (writeall(fd,(const char*)&sockfd,sizeof(unsigned))!=sizeof(unsigned)) {
    throw SerializationException();
  }
  data.Serialize(fd);
  if (writeall(fd,(const char*)&bytes,sizeof(unsigned))!=sizeof(unsigned)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&error,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&readfds,sizeof(fd_set))!=sizeof(fd_set)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&writefds,sizeof(fd_set))!=sizeof(fd_set)) {
    throw SerializationException();
  }
  if (writeall(fd,(const char*)&writefds,sizeof(fd_set))!=sizeof(fd_set)) {
    throw SerializationException();
  }
}

void SockLibRequestResponse::Unserialize(const int fd)
{
  int t;
  if (readall(fd,(char*)&t,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
  type=(slrrType)t;
  connection.Unserialize(fd);
  if (readall(fd,(char*)&sockfd,sizeof(unsigned))!=sizeof(unsigned)) {
    throw SerializationException();
  }
  data.Unserialize(fd);
  if (readall(fd,(char*)&bytes,sizeof(unsigned))!=sizeof(unsigned)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&error,sizeof(int))!=sizeof(int)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&readfds,sizeof(fd_set))!=sizeof(fd_set)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&writefds,sizeof(fd_set))!=sizeof(fd_set)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&exceptfds,sizeof(fd_set))!=sizeof(fd_set)) {
    throw SerializationException();
  }
}

std::ostream & SockLibRequestResponse::Print(std::ostream &rhs) const
{
  rhs << "SockLibRequestResponse(type=";
  rhs << (type==mSOCKET ? "SOCKET" :
	  type==mBIND ? "BIND" :
	  type==mLISTEN ? "LISTEN" :
	  type==mACCEPT ? "ACCEPT" :
	  type==mCONNECT ? "CONNECT" :
	  type==mREAD ? "READ" :
	  type==mWRITE ? "WRITE" :
	  type==mRECVFROM ? "RECVFROM" :
	  type==mSENDTO ? "SENDTO" :
	  type==mCLOSE ? "CLOSE" :
	  type==mSELECT ? "SELECT" :
	  type==mPOLL ? "POLL" :
	  type==mSET_BLOCKING ? "SET_BLOCKING" :
	  type==mSET_NONBLOCKING ? "SET_NONBLOCKING" :
	  type==mCAN_WRITE_NOW ? "CAN_WRITE_NOW" :
	  type==mCAN_READ_NOW ? "CAN_READ_NOW" :
	  type==mSTATUS ? "STATUS" :
	  "UNKNOWN");
  rhs << ", connection=" << connection;
  rhs << ", sockfd=" << sockfd;
  rhs << ", data=" << data;
  rhs << ", bytes=" << bytes;
  rhs << ", error=" << error ;
  rhs << ")";
  return rhs;
}
