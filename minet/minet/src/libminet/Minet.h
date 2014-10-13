#ifndef _Minet
#define _Minet

#include <iostream>
#include <string>
#include <cassert>

#include "config.h"

#include "buffer.h"
#include "debug.h"
#include "error.h"
#include "util.h"

#include "raw_ethernet_packet.h"
#include "raw_ethernet_packet_buffer.h"
#include "ethernet.h"

#include "arp.h"

#include "headertrailer.h"
#include "packet.h"

#include "ip.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"

#include "sock.h"
#include "sockint.h"
#include "sock_mod_structs.h"
#include "constate.h"



typedef int MinetHandle;

int MinetHandleToInputOutputFDs(const MinetHandle &h, int *inputfd, int *outputfd);

const MinetHandle MINET_NOHANDLE=-1;

enum MinetModule {
    MINET_MONITOR,
    MINET_READER,
    MINET_WRITER,
    MINET_DEVICE_DRIVER,
    MINET_ETHERNET_MUX,
    MINET_IP_MODULE,
    MINET_ARP_MODULE,
    MINET_OTHER_MODULE,
    MINET_IP_MUX,
    MINET_IP_OTHER_MODULE,
    MINET_ICMP_MODULE,
    MINET_UDP_MODULE,
    MINET_TCP_MODULE,
    MINET_SOCK_MODULE,
    MINET_SOCKLIB_MODULE,
    MINET_APP,
    MINET_EXTERNAL,
    MINET_DEFAULT,
};



enum MinetDatatype {
    MINET_NONE,
    MINET_EVENT,
    MINET_MONITORINGEVENT,
    MINET_MONITORINGEVENTDESC,
    MINET_RAWETHERNETPACKET,
    MINET_PACKET,
    MINET_ARPREQUESTRESPONSE,
    MINET_SOCKREQUESTRESPONSE,
    MINET_SOCKLIBREQUESTRESPONSE,
};

std::ostream & operator<<(std::ostream &os, const MinetModule &mon);
std::ostream & operator<<(std::ostream &os, const MinetDatatype &t);


struct MinetEvent {
    enum {Dataflow, Exception, Timeout, Error }          eventtype;
    enum {IN, OUT, INOUT, NONE}                          direction;
    MinetHandle                                          handle;
    int                                                  error;
    double                                               overtime;
    
    MinetEvent();
    MinetEvent(const MinetEvent &rhs);
    virtual ~MinetEvent();
    
    virtual const MinetEvent & operator= (const MinetEvent &rhs);
    
    virtual void Serialize(const int fd) const;
    virtual void Unserialize(const int fd);
    
    virtual std::ostream & Print(std::ostream &os) const;
    
    friend std::ostream &operator<<(std::ostream &os, const MinetEvent& L) {
	return L.Print(os);
    }
};


class MinetException : public std::string {
 private:
    MinetException() {}
    
 public:
    MinetException(const MinetException &rhs) : std::string(rhs) {}
    MinetException(const std::string &rhs) : std::string(rhs) {}
    MinetException(const char *rhs) : std::string(rhs) {}
    
    virtual ~MinetException() {}
    
    const MinetException &operator=(const MinetException &rhs) {
	((std::string *)this)->operator = (rhs); 
	return *this;
    }
    
    virtual std::ostream & Print(std::ostream &os) const { 
	os << "MinetException(" << ((const std::string &)(*this)) << ")"; 
	return os;
    }

    friend std::ostream &operator<<(std::ostream &os, const MinetException& L) {
	return L.Print(os);
    }
};


int         MinetInit(const MinetModule &mod);
int         MinetDeinit();

bool        MinetIsModuleInConfig(const MinetModule &mod);
bool        MinetIsModuleMonitored(const MinetModule &mod);

MinetHandle MinetConnect(const MinetModule &mod);
MinetHandle MinetAccept(const MinetModule &mod);
MinetHandle MinetAddExternalConnection(const int inputfd, const int outputfd);
int         MinetClose(const MinetHandle &mh);

int         MinetGetNextEvent(MinetEvent &event, double timeout = -1);


#define MINET_DECL(TYPE)					        \
int MinetSend(const MinetHandle &handle, const TYPE &object);	\
int MinetReceive(const MinetHandle &handle, TYPE &object);		\
int MinetMonitorSend(const MinetHandle &handle, const TYPE &object); \
int MinetMonitorReceive(const MinetHandle &handle, TYPE &object);	\

MINET_DECL(MinetEvent)
MINET_DECL(RawEthernetPacket)
MINET_DECL(Packet)
MINET_DECL(ARPRequestResponse)
MINET_DECL(SockRequestResponse)
MINET_DECL(SockLibRequestResponse)

#include "Monitor.h"

int MinetSendToMonitor(const MinetMonitoringEvent &object);
int MinetSendToMonitor(const MinetMonitoringEventDescription &desc, const MinetMonitoringEvent &object=MinetMonitoringEvent("no further data"));

MINET_DECL(MinetMonitoringEvent)
MINET_DECL(MinetMonitoringEventDescription)


#endif
