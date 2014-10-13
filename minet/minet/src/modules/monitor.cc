#include <stdlib.h>
#include <iomanip>
#include "Minet.h"
#include "Monitor.h"

using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
  MinetHandle
    reader,
    writer,
    device,
    ethermux,
    ip,
    arp,
    other,
    ipmux,
    ipother,
    icmp,
    udp,
    tcp,
    sock,
    socklib,
    app;

  MinetInit(MINET_MONITOR);

  reader = MinetIsModuleMonitored(MINET_READER) ? MinetAccept(MINET_READER) : MINET_NOHANDLE;
  writer = MinetIsModuleMonitored(MINET_WRITER) ? MinetAccept(MINET_WRITER) : MINET_NOHANDLE;
  device = MinetIsModuleMonitored(MINET_DEVICE_DRIVER) ? MinetAccept(MINET_DEVICE_DRIVER) : MINET_NOHANDLE;
  ethermux = MinetIsModuleMonitored(MINET_ETHERNET_MUX) ? MinetAccept(MINET_ETHERNET_MUX) : MINET_NOHANDLE;
  ip= MinetIsModuleMonitored(MINET_IP_MODULE) ? MinetAccept(MINET_IP_MODULE) : MINET_NOHANDLE;
  arp= MinetIsModuleMonitored(MINET_ARP_MODULE) ? MinetAccept(MINET_ARP_MODULE) : MINET_NOHANDLE;
  other= MinetIsModuleMonitored(MINET_OTHER_MODULE) ? MinetAccept(MINET_OTHER_MODULE) : MINET_NOHANDLE;
  ipmux= MinetIsModuleMonitored(MINET_IP_MUX) ? MinetAccept(MINET_IP_MUX) : MINET_NOHANDLE;
  ipother=MinetIsModuleMonitored(MINET_IP_OTHER_MODULE) ? MinetAccept(MINET_IP_OTHER_MODULE) : MINET_NOHANDLE;
  icmp=MinetIsModuleMonitored(MINET_ICMP_MODULE) ? MinetAccept(MINET_ICMP_MODULE) : MINET_NOHANDLE;
  udp=MinetIsModuleMonitored(MINET_UDP_MODULE) ? MinetAccept(MINET_UDP_MODULE) : MINET_NOHANDLE;
  tcp=MinetIsModuleMonitored(MINET_TCP_MODULE) ? MinetAccept(MINET_TCP_MODULE) : MINET_NOHANDLE;
  sock=MinetIsModuleMonitored(MINET_SOCK_MODULE) ? MinetAccept(MINET_SOCK_MODULE) : MINET_NOHANDLE;
  socklib=MinetIsModuleMonitored(MINET_SOCKLIB_MODULE) ? MinetAccept(MINET_SOCKLIB_MODULE) : MINET_NOHANDLE;
  app=MinetIsModuleMonitored(MINET_APP) ? MinetAccept(MINET_APP) : MINET_NOHANDLE;

  MinetEvent myevent;
  MinetMonitoringEventDescription desc;

  MinetEvent event;
  MinetMonitoringEvent monevent;
  RawEthernetPacket rawpacket;
  Packet packet;
  SockRequestResponse srr;
  SockLibRequestResponse slrr;
  ARPRequestResponse arr;


  cerr << "monitor running\n";

  while (MinetGetNextEvent(myevent)==0) {
    if (myevent.eventtype!=MinetEvent::Dataflow || myevent.direction!=MinetEvent::IN) {
      cerr << "Ignoring this event: "<<myevent<<endl;
    } else {
      MinetReceive(myevent.handle,desc);
      cerr << std::setprecision(20) << desc << " : ";
      switch (desc.datatype) {
      case MINET_EVENT:
	MinetReceive(myevent.handle,event);
	cerr << event << endl;
	break;
      case MINET_MONITORINGEVENT:
	MinetReceive(myevent.handle,monevent);
	cerr << monevent << endl;
	break;
      case MINET_RAWETHERNETPACKET:
	MinetReceive(myevent.handle,rawpacket);
	cerr << rawpacket << endl;
	break;
      case MINET_PACKET:
	MinetReceive(myevent.handle,packet);
	cerr << packet << endl;
	break;
      case MINET_ARPREQUESTRESPONSE:
	MinetReceive(myevent.handle,arr);
	cerr << arr << endl;
	break;
      case MINET_SOCKREQUESTRESPONSE:
	MinetReceive(myevent.handle,srr);
	cerr << srr << endl;
	break;
      case MINET_SOCKLIBREQUESTRESPONSE:
	MinetReceive(myevent.handle,slrr);
	cerr << slrr << endl;
	break;
      case MINET_NONE:
      default:
	break;
      }
    }
  }
  MinetDeinit();
}
