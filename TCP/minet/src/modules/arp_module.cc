#include <iostream>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "Minet.h"

using std::cerr;
using std::endl;

void usage()
{
  cerr<<"arp_module myipadx myethernetadx\n";
}


int main(int argc, char *argv[])
{
  ARPCache cache;

  IPAddress ipaddr(MyIPAddr());
  EthernetAddr ethernetaddr(MyEthernetAddr());


  MinetInit(MINET_ARP_MODULE);

  MinetHandle mux = MinetIsModuleInConfig(MINET_ETHERNET_MUX) ? MinetConnect(MINET_ETHERNET_MUX) : MINET_NOHANDLE;
  MinetHandle ip = MinetIsModuleInConfig(MINET_IP_MODULE) ? MinetAccept(MINET_IP_MODULE) : MINET_NOHANDLE;

  if (mux==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_ETHERNET_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ethernet mux"));
    return -1;
  }
  if (ip==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_IP_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from ip_module"));
    return -1;
  }


  cache.Update(ARPRequestResponse(ipaddr,ethernetaddr,ARPRequestResponse::RESPONSE_OK));



  cerr << "arp_module: answering ARPs for " << ipaddr
       << " with " << ethernetaddr <<"\n";

  MinetSendToMonitor(MinetMonitoringEvent("arp_module operational."));

  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {
      if (event.handle==ip) {
	ARPRequestResponse r;
	MinetReceive(ip,r);
	cerr << "Local Request:  "<<r<<"\n";
	cache.Lookup(r);
	cerr << "Local Response: "<<r<<"\n";
	MinetSend(ip,r);
	if (r.flag==ARPRequestResponse::RESPONSE_UNKNOWN) {
	  ARPPacket request(ARPPacket::Request,
			    ethernetaddr,
			    ipaddr,
			    ETHERNET_BLANK_ADDR,
			    r.ipaddr);
	  EthernetHeader h;
	  h.SetSrcAddr(ethernetaddr);
	  h.SetDestAddr(ETHERNET_BROADCAST_ADDR);
	  h.SetProtocolType(PROTO_ARP);
	  request.PushHeader(h);

	  RawEthernetPacket rawout(request);
	  MinetSend(mux,rawout);
	}
      }
      if (event.handle==mux) {
	RawEthernetPacket rawpacket;
	MinetReceive(mux,rawpacket);
	ARPPacket arp(rawpacket);

	if (arp.IsIPToEthernet()) {
	  IPAddress sourceip;
	  EthernetAddr sourcehw;

	  arp.GetSenderIPAddr(sourceip);
	  arp.GetSenderEthernetAddr(sourcehw);

	  ARPRequestResponse r(sourceip,sourcehw,ARPRequestResponse::RESPONSE_OK);
	  cache.Update(r);

	  //cerr << cache << "\n";

	  if (arp.IsIPToEthernetRequest()) {
	    IPAddress targetip;
	    arp.GetTargetIPAddr(targetip);
	    if (targetip==ipaddr) {
	      // reply

	      ARPPacket repl(ARPPacket::Reply,
			     ethernetaddr,
			     ipaddr,
			     sourcehw,
			     sourceip);

	      EthernetHeader h;
	      h.SetSrcAddr(ethernetaddr);
	      h.SetDestAddr(sourcehw);
	      h.SetProtocolType(PROTO_ARP);
	      repl.PushHeader(h);

	      RawEthernetPacket rawout(repl);
	      MinetSend(mux,rawout);
	      cerr << "Remote Request:  " << arp <<"\n";
	      cerr << "Remote Response: " << repl <<"\n";
	    }
	  }
	}
      }
    }
  }
}





