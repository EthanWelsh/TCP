#ifndef _Monitor
#define _Monitor

#include <iostream>
#include <string>
#include "Minet.h"


enum MinetOpType {
  MINET_INIT,
  MINET_DEINIT,
  MINET_SEND,
  MINET_RECEIVE,
  MINET_GETNEXTEVENT,
  MINET_CONNECT,
  MINET_ACCEPT,
  MINET_CLOSE,
  MINET_SENDTOMONITOR,
  MINET_NOP,
};

std::ostream & operator<<(std::ostream &os, const MinetOpType &op);



struct MinetMonitoringEventDescription {
  double         timestamp;
  MinetModule    source;
  MinetModule    from;
  MinetModule    to;
  MinetDatatype  datatype;
  MinetOpType    optype;

  MinetMonitoringEventDescription();
  MinetMonitoringEventDescription(const MinetMonitoringEventDescription &rhs);
  virtual ~MinetMonitoringEventDescription();

  virtual const MinetMonitoringEventDescription & operator= (const MinetMonitoringEventDescription &rhs);

  virtual void Serialize(const int fd) const;
  virtual void Unserialize(const int fd);

  virtual std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const MinetMonitoringEventDescription& L) {
    return L.Print(os);
  }
};


struct MinetMonitoringEvent : public std::string {
  MinetMonitoringEvent();
  MinetMonitoringEvent(const char *s);
  MinetMonitoringEvent(const std::string &s);
  MinetMonitoringEvent(const MinetMonitoringEvent &rhs);
  virtual ~MinetMonitoringEvent();

  virtual const MinetMonitoringEvent & operator= (const MinetMonitoringEvent &rhs);

  virtual void Serialize(const int fd) const;
  virtual void Unserialize(const int fd);

  virtual std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const MinetMonitoringEvent& L) {
    return L.Print(os);
  }
};


#endif




