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
#include "bitsource.h"

#define DIFFUSION_HACK         0
#define DIFFUSION_RES_TOS      0
#define DIFFUSION_ID_DF        0
#define DIFFUSION_RES_FLAGS    0
#define DIFFUSION_DF_FRAGOFF   0
#define DIFFUSION_CHECKSUM     0


#define FORCE_ROUTE_THROUGH_GATEWAY 0

using std::cout;
using std::cerr;
using std::endl;

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

#if DIFFUSION_HACK
  InitBits();
#endif


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

	cerr << "Received Packet: " << endl;
	iph.Print(cerr);  cerr << endl;  p.Print(cerr);  cerr << endl;

#if DIFFUSION_HACK
	cerr << "IP Diffusion Enabled.  Recovered bits follow..." << endl;
	if (DIFFUSION_RES_TOS) {
	  unsigned char tos;
	  iph.GetTOS(tos);
	  tos&=0x3;
	  tos<<=6;
	  cerr<< "   tos reserved bits (2) : "; PrintBits(cerr,&tos,2,0); cerr<<endl;
	}
	if (DIFFUSION_ID_DF) {
	  unsigned char flags;
	  iph.GetFlags(flags);
	  if (flags&IP_HEADER_FLAG_DONTFRAG) {
	    unsigned short id;
	    iph.GetID(id);
	    cerr << "   id bits (16) : "; PrintBits(cerr,(unsigned char*)&id,16,0); cerr <<endl;
	  } else {
	    cerr << "   id bits (16) : NONE (DF not set)"<<endl;
	  }
	}
	if (DIFFUSION_RES_FLAGS) {
	  unsigned char flags;
	  iph.GetFlags(flags);
	  unsigned char bit = ((flags&IP_HEADER_FLAG_RESERVED)>>2)<<7;
	  cerr << "   Reserved flag bit (1) : "; PrintBits(cerr,&bit,1,0);cerr <<endl;
	}
	if (DIFFUSION_DF_FRAGOFF) {
	  unsigned char flags;
	  iph.GetFlags(flags);
	  if ((flags&IP_HEADER_FLAG_DONTFRAG) && !(flags&IP_HEADER_FLAG_MOREFRAG)) {
	    unsigned short off;
	    iph.GetFragOffset(off);
	    off<<=3;
	    cerr << "   Fragoff bits (13) : "; PrintBits(cerr,(unsigned char*)&off,13,0); cerr <<endl;
	  } else {
	    cerr << "   Fragoff bits (13) : none (DF not set)"<<endl;
	  }
	}
	if (DIFFUSION_CHECKSUM) {
	  cerr << "   Checksum bits (?) : none (don't know how to do this yet)"<<endl;
	}
	cerr << "End of IP Diffusion stuff."<<endl;
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

	    // add ethernet header
	    IPHeader error_iph = error.FindHeader(Headers::IPHeader);
	    IPAddress ipaddr;
	    error_iph.GetDestIP(ipaddr);
	    ARPRequestResponse req(ipaddr,
				   EthernetAddr(ETHERNET_BLANK_ADDR),
				   ARPRequestResponse::REQUEST);
	    ARPRequestResponse resp;

	    MinetSend(arp,req);
	    MinetReceive(arp,resp);

	    EthernetHeader error_eh;
	    error_eh.SetSrcAddr(MyEthernetAddr());
	    error_eh.SetDestAddr(resp.ethernetaddr);
	    error_eh.SetProtocolType(PROTO_IP);
	    error.PushHeader(error_eh);

	    RawEthernetPacket e(error);
	    MinetSend(ethermux, e);
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
	    cerr << "NOTE: NO ICMP PACKET WAS SENT BACK\n";

	    continue;
	  }
	  MinetSend(ipmux,p);
	} else {
	  ; // discarded due to different target address
	}

	/*
	// EXAMPLE: RESPONDING TO ICMP REQUESTS FROM IP_MODULE (RAW)
	// respond to packet
	ICMPPacket response;
	response.respond(raw);

	// add ethernet header
	IPHeader respond_iph = response.FindHeader(Headers::IPHeader);
	IPAddress ipaddr;
	respond_iph.GetDestIP(ipaddr);
	ARPRequestResponse req(ipaddr,
			       EthernetAddr(ETHERNET_BLANK_ADDR),
			       ARPRequestResponse::REQUEST);
	ARPRequestResponse resp;

	MinetSend(arp,req);
	MinetReceive(arp,resp);

	EthernetHeader respond_eh;
	respond_eh.SetSrcAddr(MyEthernetAddr());
	respond_eh.SetDestAddr(resp.ethernetaddr);
	respond_eh.SetProtocolType(PROTO_IP);
	response.PushHeader(respond_eh);

	if (response.requires_reply())
	{
	  cerr << "Sent Packet: " << endl;
	  DebugDump(response);  cerr << endl;
	  RawEthernetPacket e(response);

	  MinetSend(ethermux, e);
	}
	// END RESPONDING TO ICMP REQUESTS FROM IP_MODULE
	*/

	/*
	// EXAMPLE: RESPONDING TO ICMP REQUESTS FROM IP_MODULE (PACKET)
	// respond to packet
	ICMPPacket response;
	response.respond_in_ip_module(p);

	// add ethernet header
	IPHeader respond_iph = response.FindHeader(Headers::IPHeader);
	IPAddress ipaddr;
	respond_iph.GetDestIP(ipaddr);
	ARPRequestResponse req(ipaddr,
			       EthernetAddr(ETHERNET_BLANK_ADDR),
			       ARPRequestResponse::REQUEST);
	ARPRequestResponse resp;

	MinetSend(arp,req);
	MinetReceive(arp,resp);

	EthernetHeader respond_eh;
	respond_eh.SetSrcAddr(MyEthernetAddr());
	respond_eh.SetDestAddr(resp.ethernetaddr);
	respond_eh.SetProtocolType(PROTO_IP);
	response.PushHeader(respond_eh);

	if (response.requires_reply())
	{
	  cerr << "Sent Packet: " << endl;
	  DebugDump(response);  cerr << endl;
	  RawEthernetPacket e(response);

	  MinetSend(ethermux, e);
	}
	// END RESPONDING TO ICMP REQUESTS FROM IP_MODULE
	*/

	/*
	// EXAMPLE: SENDING ICMP ERRORS FROM IP_MODULE
	ICMPPacket error("129.105.100.9", DESTINATION_UNREACHABLE, PROTOCOL_UNREACHABLE, p, IP_PROTO_IP);

	// add ethernet header
	IPHeader error_iph = response.FindHeader(Headers::IPHeader);
	IPAddress ipaddr;
        error_iph.GetDestIP(ipaddr);
	ARPRequestResponse req(ipaddr,
			       EthernetAddr(ETHERNET_BLANK_ADDR),
			       ARPRequestResponse::REQUEST);
	ARPRequestResponse resp;

	EthernetHeader error_eh;
	error_eh.SetSrcAddr(MyEthernetAddr());
	error_eh.SetDestAddr(resp.ethernetaddr);
	error_eh.SetProtocolType(PROTO_IP);
	error.PushHeader(error_eh);

	cerr << "Received Packet: " << endl;
	p.ExtractHeaderFromPayload<ICMPHeader>(ICMP_HEADER_LENGTH); DebugDump(p);  cerr << endl;
	cerr << "Sent Packet: " << endl;
	DebugDump(error);  cerr << endl;
	RawEthernetPacket e(error);

	MinetSend(ethermux, e);
	*/


      }
      if (event.handle==ipmux) {
	Packet p;
	MinetReceive(ipmux,p);
	IPHeader iph = p.FindHeader(Headers::IPHeader);

#if DIFFUSION_HACK
	cerr << "IP Diffusion Enabled.  Inserted  bits follow..." << endl;
	if (DIFFUSION_RES_TOS) {
	  unsigned char tos;
	  iph.GetTOS(tos);
	  if ((tos&0x3)==0) {
	    unsigned char bits,bits2;
	    GetNextBits(&bits,2,0);
	    bits2=(bits>>6)&0x3;
	    tos|=bits2;
	    iph.SetTOS(tos);
	    cerr << "   tos reserved bits (2) : "; PrintBits(cerr,&bits,2,0); cerr << endl;
	  } else {
	    cerr << "   tos reserved bits (2) : NONE (already set!)"<<endl;
	  }
	}
	if (DIFFUSION_ID_DF) {
	  unsigned char flags;
	  iph.GetFlags(flags);
	  if (flags&IP_HEADER_FLAG_DONTFRAG) {
	    unsigned short id;
	    GetNextBits((unsigned char*)&id,16,0);
	    iph.SetID(id);
	    cerr << "   id bits (16) : "; PrintBits(cerr,(unsigned char*)&id,16,0); cerr<<endl;
	  } else {
	    cerr << "   id bits (16) : NONE (DF not set)"<<endl;
	  }
	}
	if (DIFFUSION_RES_FLAGS) {
	  unsigned char flags;
	  iph.GetFlags(flags);
	  unsigned char bit = (flags&IP_HEADER_FLAG_RESERVED)>>2;
	  if (!bit) {
	    unsigned char bit2;
	    GetNextBits(&bit,1,0);
	    bit2=bit>>(7-2);
	    flags|=bit2;
	    iph.SetFlags(flags);
	    cerr << "   Reserved flag bit (1) : "; PrintBits(cerr,&bit,1,0); cerr <<endl;
	  } else {
	    cerr << "   Reserved flag bit (1) : NONE (already set!)"<<endl;
	  }
	}
	if (DIFFUSION_DF_FRAGOFF) {
	  unsigned char flags;
	  iph.GetFlags(flags);
	  if (flags&IP_HEADER_FLAG_DONTFRAG) {
	    unsigned short off;
	    unsigned short bits;
	    GetNextBits((unsigned char*)&bits,13,0);
	    off=bits>>3;
	    iph.SetFragOffset(off);
	    flags&=~IP_HEADER_FLAG_MOREFRAG;
	    iph.SetFlags(flags);
	    cerr << "   Fragoff bits (13) : "; PrintBits(cerr,(unsigned char*)&bits,13,0); cerr <<endl;
	  } else {
	    cerr << "   Fragoff bits (13) : none (DF not set)"<<endl;
	  }
	}
	if (DIFFUSION_CHECKSUM) {
	  cerr << "   Checksum bits (?) : none (don't know how to do this yet)"<<endl;
	}
	// Now reinsert the header
	// first toss off the old one
	p.PopFrontHeader();
	// now push our modified one back on
	p.PushFrontHeader(iph);
	// and then copy it back out again for the rest
	iph=p.FindHeader(Headers::IPHeader);

	cerr << "End of IP Diffusion stuff."<<endl;

#endif



	// ROUTE



	// force route through gateway




	IPAddress ipaddr;

#if FORCE_ROUTE_THROUGH_GATEWAY
	if (getenv("MINET_GATEWAY")) {
	  ipaddr = IPAddress(getenv("MINET_GATEWAY"));
	} else {
	  cerr << "Can't route to gateway as MINET_GATEWAY is not set=> Sending Direct.\n";
	  iph.GetDestIP(ipaddr);
	}
#else
	iph.GetDestIP(ipaddr);
#endif

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

	  cout << "ABOUT TO SEND OUT: " << endl;
	  Packet check(e);
	  check.ExtractHeaderFromPayload<EthernetHeader>(ETHERNET_HEADER_LEN);
	  check.ExtractHeaderFromPayload<IPHeader>(IPHeader::EstimateIPHeaderLength(check));
	  // check.ExtractHeaderFromPayload<ICMPHeader>(ICMP_HEADER_LENGTH);
	  EthernetHeader eh = check.FindHeader(Headers::EthernetHeader);
	  IPHeader iph = check.FindHeader(Headers::IPHeader);
	  // ICMPHeader icmph = check.FindHeader(Headers::ICMPHeader);

	  eh.Print(cerr);  cerr << endl;
	  iph.Print(cerr); cerr << endl;
	  // icmph.Print(cerr);  cerr << endl;
	  cout << "END OF PACKET" << endl << endl;


	  MinetSend(ethermux,e);
	} else {
	  MinetSendToMonitor(MinetMonitoringEvent("Discarding packet because there is no arp entry"));
	  cerr << "Discarded IP packet because there is no arp entry\n";
	}
      }
    }
  }
  MinetDeinit();
  return 0;
}





