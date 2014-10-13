#include <sys/time.h>
#include <sys/types.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>

#include <iostream>

#include "Minet.h"


#define DEBUG_ICMP 0

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char * argv[])
{
  MinetHandle ip, udp, tcp, icmp, other;

  MinetInit(MINET_IP_MUX);

  ip=MinetIsModuleInConfig(MINET_IP_MODULE) ? MinetConnect(MINET_IP_MODULE) : MINET_NOHANDLE;
  udp=MinetIsModuleInConfig(MINET_UDP_MODULE) ? MinetAccept(MINET_UDP_MODULE) : MINET_NOHANDLE;
  tcp=MinetIsModuleInConfig(MINET_TCP_MODULE) ? MinetAccept(MINET_TCP_MODULE) : MINET_NOHANDLE;
  icmp=MinetIsModuleInConfig(MINET_ICMP_MODULE) ? MinetAccept(MINET_ICMP_MODULE) : MINET_NOHANDLE;
  other=MinetIsModuleInConfig(MINET_IP_OTHER_MODULE) ? MinetAccept(MINET_IP_OTHER_MODULE) : MINET_NOHANDLE;

  if (ip==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_IP_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ip_module"));
    return -1;
  }
  if (udp==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_UDP_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from udp_module"));
    return -1;
  }
  if (tcp==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_TCP_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from tcp_module"));
    return -1;
  }
  if (icmp==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_ICMP_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from icmp_module"));
    return -1;
  }
  if (other==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_IP_OTHER_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from ipother_module"));
    return -1;
  }


  cerr << "ip_mux operating!\n";

  MinetSendToMonitor(MinetMonitoringEvent("ip_mux operating!"));

  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {
      if (event.handle==ip) {
	Packet p;
	unsigned char proto;
	MinetReceive(ip,p);
	IPHeader iph=p.FindHeader(Headers::IPHeader);
	iph.GetProtocol(proto);
	switch (proto) {
	case IP_PROTO_UDP:
	  if (udp!=MINET_NOHANDLE) {
	    MinetSend(udp,p);
	  }
	  break;
	case IP_PROTO_TCP:
	  if (tcp!=MINET_NOHANDLE) {
	    MinetSend(tcp,p);
	  }
	  break;
	case IP_PROTO_ICMP:
	  if (icmp!=MINET_NOHANDLE) {
	    MinetSend(icmp,p);
	  }
	  break;
	default:
	  if (other!=MINET_NOHANDLE) {
	    MinetSend(other,p);
	  } else {
	    MinetSendToMonitor(MinetMonitoringEvent("Discarding incoming IP Packet of unknown protocol"));
	    IPAddress source;  iph.GetSourceIP(source);
	    ICMPPacket error(source, DESTINATION_UNREACHABLE, PROTOCOL_UNREACHABLE, p);
	    MinetSendToMonitor(MinetMonitoringEvent("ICMP error message has been sent to host"));
	    MinetSend(ip, error);
	  }
	  break;
	}
      }
    }
    if (event.handle==udp) {
      Packet p;
      MinetReceive(udp,p);
      MinetSend(ip,p);
    }
    if (event.handle==tcp) {
      Packet p;
      MinetReceive(tcp,p);
      MinetSend(ip,p);
    }
    if (event.handle==icmp) {
      Packet p;
      MinetReceive(icmp,p);

#if DEBUG_ICMP
      cout << "ABOUT TO SEND OUT: " << endl;
      Packet check(p);
      check.ExtractHeaderFromPayload<IPHeader>(IPHeader::EstimateIPHeaderLength(check));
      check.ExtractHeaderFromPayload<ICMPHeader>(ICMP_HEADER_LENGTH);
      IPHeader iph = check.FindHeader(Headers::IPHeader);
      ICMPHeader icmph = check.FindHeader(Headers::ICMPHeader);

      iph.Print(cerr); cerr << endl;
      icmph.Print(cerr);  cerr << endl;
      cout << "END OF PACKET" << endl << endl;
#endif

      MinetSend(ip,p);
    }
    if (event.handle==other) {
      Packet p;
      MinetReceive(other,p);
      MinetSend(ip,p);
    }
  }
}





