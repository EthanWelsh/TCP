#include "Minet.h"
#include "Monitor.h"


std::ostream & operator<<(std::ostream &os, const MinetOpType &op)
{
  os << (op==MINET_INIT ? "MINET_INIT" :
         op==MINET_DEINIT ? "MINET_DEINIT" :
         op==MINET_SEND ? "MINET_SEND" :
         op==MINET_RECEIVE ? "MINET_RECEIVE" :
	 op==MINET_GETNEXTEVENT ? "MINET_GETNEXTEVENT" :
         op==MINET_CONNECT ? "MINET_CONNECT" :
         op==MINET_ACCEPT ? "MINET_ACCEPT" :
         op==MINET_SENDTOMONITOR ? "MINET_SENDTOMONITOR" :
         op==MINET_NOP ? "MINET_NOP" : "UNKNOWN");
  return os;
}


MinetMonitoringEventDescription::MinetMonitoringEventDescription() :
  timestamp(0.0),
  source(MINET_DEFAULT),
  from(MINET_DEFAULT),
  to(MINET_DEFAULT),
  datatype(MINET_NONE),
  optype(MINET_NOP)
{}

MinetMonitoringEventDescription::MinetMonitoringEventDescription(const MinetMonitoringEventDescription &rhs)
{
  *this = rhs;
}

const MinetMonitoringEventDescription &MinetMonitoringEventDescription::operator= (const MinetMonitoringEventDescription &rhs)
{
  timestamp=rhs.timestamp;
  source=rhs.source;
  from=rhs.from;
  to=rhs.to;
  datatype=rhs.datatype;
  optype=rhs.optype;
  return *this;
}


MinetMonitoringEventDescription::~MinetMonitoringEventDescription()
{}

void MinetMonitoringEventDescription::Serialize(const int fd) const
{
  if (writeall(fd,(char*)&timestamp,sizeof(timestamp))!=sizeof(timestamp)) {
    throw SerializationException();
  }
  if (writeall(fd,(char*)&source,sizeof(source))!=sizeof(source)) {
    throw SerializationException();
  }
  if (writeall(fd,(char*)&from,sizeof(from))!=sizeof(from)) {
    throw SerializationException();
  }
  if (writeall(fd,(char*)&to,sizeof(to))!=sizeof(to)) {
    throw SerializationException();
  }
  if (writeall(fd,(char*)&datatype,sizeof(datatype))!=sizeof(datatype)) {
    throw SerializationException();
  }
  if (writeall(fd,(char*)&optype,sizeof(optype))!=sizeof(optype)) {
    throw SerializationException();
  }
}

void MinetMonitoringEventDescription::Unserialize(const int fd)
{
  if (readall(fd,(char*)&timestamp,sizeof(timestamp))!=sizeof(timestamp)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&source,sizeof(source))!=sizeof(source)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&from,sizeof(from))!=sizeof(from)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&to,sizeof(to))!=sizeof(to)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&datatype,sizeof(datatype))!=sizeof(datatype)) {
    throw SerializationException();
  }
  if (readall(fd,(char*)&optype,sizeof(optype))!=sizeof(optype)) {
    throw SerializationException();
  }
}


std::ostream & MinetMonitoringEventDescription::Print(std::ostream &os) const
{
  os << "MinetMonitoringEventDescription(timestamp="<<timestamp
     << ", source="<<source<<", from="<<from<<", to="<<to<<", optype="<<optype<< ", datatype="<<datatype<<")";
  return os;
}


MinetMonitoringEvent::MinetMonitoringEvent() : std::string("")
{}


MinetMonitoringEvent::MinetMonitoringEvent(const std::string &s) : std::string(s)
{}


MinetMonitoringEvent::MinetMonitoringEvent(const char *s) : std::string(s)
{}



MinetMonitoringEvent::MinetMonitoringEvent(const MinetMonitoringEvent &rhs)
{
  *this=rhs;
}

const MinetMonitoringEvent &MinetMonitoringEvent::operator=(const MinetMonitoringEvent &rhs)
{
  ((std::string*)this)->operator=(rhs);
  return *this;
}

MinetMonitoringEvent::~MinetMonitoringEvent()
{}

void MinetMonitoringEvent::Serialize(const int fd) const
{
  const char *buf=this->c_str();
  int len=strlen(buf);
  if (writeall(fd,(char*)&len,sizeof(len))!=sizeof(len)) {
    throw SerializationException();
  }
  if (writeall(fd,buf,len)!=len) {
    throw SerializationException();
  }
}

void MinetMonitoringEvent::Unserialize(const int fd)
{
  int len;
  if (readall(fd,(char*)&len,sizeof(len))!=sizeof(len)) {
    throw SerializationException();
  }
  char buf[len+1];
  if (readall(fd,buf,len)!=len) {
    throw SerializationException();
  }
  buf[len]=0;
  this->operator=(MinetMonitoringEvent(buf));
}


std::ostream & MinetMonitoringEvent::Print(std::ostream &os) const
{
  os << "MinetMonitoringEvent("<<(*((std::string *)this))<<")";
  return os;
}


