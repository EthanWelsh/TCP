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


#include <iostream>

#include "Minet.h"

#define DEBUG_SEND 1
#define DEBUG_RECV 1

using std::cout;
using std::cerr;
using std::endl;

int SendPacket(MinetHandle &ethermux, MinetHandle &arp, Packet &p)
{

  IPHeader iph = p.FindHeader(Headers::IPHeader);

  IPAddress ipaddr;

  iph.GetDestIP(ipaddr);

  ARPRequestResponse req(ipaddr,
			 EthernetAddr(ETHERNET_BLANK_ADDR),
			 ARPRequestResponse::REQUEST);
  ARPRequestResponse resp;

  MinetSend(arp,req);
  MinetReceive(arp,resp);

  if (resp.ethernetaddr!=ETHERNET_BLANK_ADDR) {
    resp.flag=ARPRequestResponse::RESPONSE_OK;
  }

  if ((resp.flag==ARPRequestResponse::RESPONSE_OK) ||
      (ipaddr == "255.255.255.255")) {
    // set src and dest addrs in header
    // set protocol ip

    EthernetHeader h;
    h.SetSrcAddr(MyEthernetAddr());
    h.SetDestAddr(resp.ethernetaddr);
    h.SetProtocolType(PROTO_IP);
    p.PushHeader(h);

    RawEthernetPacket e(p);

#if DEBUG_SEND
    cout << "ABOUT TO SEND OUT: " << endl;
    Packet check(e);
    check.ExtractHeaderFromPayload<EthernetHeader>(ETHERNET_HEADER_LEN);
    check.ExtractHeaderFromPayload<IPHeader>(IPHeader::EstimateIPHeaderLength(check));
    EthernetHeader eh = check.FindHeader(Headers::EthernetHeader);
    IPHeader iph = check.FindHeader(Headers::IPHeader);

    eh.Print(cerr);  cerr << endl;
    iph.Print(cerr); cerr << endl;
    cout << "END OF PACKET" << endl << endl;
#endif

    MinetSend(ethermux,e);
    return 0;
  } else {
    MinetSendToMonitor(MinetMonitoringEvent("Discarding packet because there is no arp entry"));
    cerr << "Discarded IP packet because there is no arp entry\n";
    return -1;
  }
}

int main(int argc, char *argv[])
{
  MinetHandle ethermux, ipmux, arp;

  MinetInit(MINET_IP_MODULE);

  ethermux=MinetIsModuleInConfig(MINET_ETHERNET_MUX) ? 				\
			MinetConnect(MINET_ETHERNET_MUX) : MINET_NOHANDLE;
  arp=MinetIsModuleInConfig(MINET_ARP_MODULE) ? 				\
			MinetConnect(MINET_ARP_MODULE) : MINET_NOHANDLE;
  ipmux=MinetIsModuleInConfig(MINET_IP_MUX) ?					\
			MinetAccept(MINET_IP_MUX) : MINET_NOHANDLE;

  if (ethermux==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_ETHERNET_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ethermux"));
    return -1;
  }
  if (ipmux==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_ETHERNET_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from ipmux"));
    return -1;
  }
  if (arp==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_ETHERNET_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to arp_module"));
    return -1;
  }

  MinetSendToMonitor(MinetMonitoringEvent("ip_module handling IP traffic........"));

  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {
      if (event.handle==ethermux) {
	RawEthernetPacket raw;
	MinetReceive(ethermux,raw);

	Packet p(raw);
	p.ExtractHeaderFromPayload<EthernetHeader>(ETHERNET_HEADER_LEN);
	p.ExtractHeaderFromPayload<IPHeader>(IPHeader::EstimateIPHeaderLength(p));
	IPHeader iph;
	iph=p.FindHeader(Headers::IPHeader);

#if DEBUG_RECV
	cerr << "Received Packet: " << endl;
	iph.Print(cerr);  cerr << endl;  p.Print(cerr);  cerr << endl;
#endif

	IPAddress toip;
	iph.GetDestIP(toip);

	if (toip==MyIPAddr() || toip==IPAddress(IP_ADDRESS_BROADCAST)) {
	  if (!(iph.IsChecksumCorrect())) {
	    // discard the packet
	    MinetSendToMonitor(MinetMonitoringEvent				\
			("Discarding packet because header checksum is wrong."));
	    cerr << "Discarding following packet because header checksum is wrong: "<<p<<"\n";

	    IPAddress src;  iph.GetSourceIP(src);
	    // "2" specifies the octet that is wrong (in this case, the checksum)

	    ICMPPacket error(src, PARAMETER_PROBLEM, 2, p);

	    MinetSendToMonitor(MinetMonitoringEvent("ICMP error message has been sent to host"));

	    SendPacket(ethermux,arp,error);

	    continue;
	  }
	  unsigned char ttl;
	  iph.GetTTL(ttl);
	  if (ttl==0) {
	    // discard the packet
	    MinetSendToMonitor(MinetMonitoringEvent				\
			("Discarding packet because TTL is zero."));
	    cerr << "Discarding following packet because TTL is zero: "<<p<<"\n";

	    IPAddress src;  iph.GetSourceIP(src);
	    ICMPPacket error(src, TIME_EXCEEDED,TTL_EQUALS_ZERO_DURING_TRANSIT, p);

	    MinetSendToMonitor(MinetMonitoringEvent("ICMP error message has been sent to host"));

	    SendPacket(ethermux,arp,error);

	    continue;
	  }
	  unsigned short fragoff;
	  unsigned char flags;
	  iph.GetFlags(flags);
	  iph.GetFragOffset(fragoff);
	  if ((flags&IP_HEADER_FLAG_MOREFRAG) || (fragoff!=0)) {
	    MinetSendToMonitor(MinetMonitoringEvent				\
			("Discarding packet because it is a fragment"));
	    cerr << "Discarding following packet because it is a fragment: "<<p<<"\n";

	    IPAddress src;  iph.GetSourceIP(src);
	    ICMPPacket error(src, DESTINATION_UNREACHABLE ,FRAGMENTATION_NEEDED, p);

	    MinetSendToMonitor(MinetMonitoringEvent("ICMP error message has been sent to host"));

	    SendPacket(ethermux,arp,error);

	    continue;
	  }

	  // route here - would send icmp dest unreachable if fails

	  // unfragment here

	  MinetSend(ipmux,p);
	} else {
	  ; // discarded due to different target address
	}

      }
      if (event.handle==ipmux) {
	Packet p;
	MinetReceive(ipmux,p);

	// Route Packet Here - would send icmp dest unreachable if fails

	// Fragment Packet Here

	SendPacket(ethermux,arp,p);

      }
    }
  }
  MinetDeinit();
  return 0;
}



