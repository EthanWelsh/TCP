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

#include <string.h>
#include <iostream>

#include "Minet.h"

using std::cout;
using std::cerr;
using std::endl;


struct UDPState {
  std::ostream & Print(std::ostream &os) const { os <<"UDPState()"; return os;}

  friend std::ostream &operator<<(std::ostream &os, const UDPState& L) {
    return L.Print(os);
  }
};


int main(int argc, char *argv[])
{
  MinetHandle mux;
  MinetHandle sock;

  ConnectionList<UDPState> clist;

  MinetInit(MINET_UDP_MODULE);

  mux=MinetIsModuleInConfig(MINET_IP_MUX) ? MinetConnect(MINET_IP_MUX) : MINET_NOHANDLE;
  sock=MinetIsModuleInConfig(MINET_SOCK_MODULE) ? MinetAccept(MINET_SOCK_MODULE) : MINET_NOHANDLE;

  if (mux==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_IP_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ip_mux"));
    return -1;
  }
  if (sock==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_SOCK_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from sock_module"));
    return -1;
  }

  cerr << "udp_module handling udp traffic.......\n";

  MinetSendToMonitor(MinetMonitoringEvent("udp_module handling udp traffic........"));

  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {
      if (event.handle==mux) {
	Packet p;
	unsigned short len;
	bool checksumok;
	MinetReceive(mux,p);
	p.ExtractHeaderFromPayload<UDPHeader>(8);
	UDPHeader udph;
	udph=p.FindHeader(Headers::UDPHeader);
	checksumok=udph.IsCorrectChecksum(p);
	IPHeader iph;
	iph=p.FindHeader(Headers::IPHeader);
	Connection c;
	// note that this is flipped around because
	// "source" is interepreted as "this machine"
	iph.GetDestIP(c.src);
	iph.GetSourceIP(c.dest);
	iph.GetProtocol(c.protocol);
	udph.GetDestPort(c.srcport);
	udph.GetSourcePort(c.destport);
	ConnectionList<UDPState>::iterator cs = clist.FindMatching(c);
	if (cs!=clist.end()) {
	  udph.GetLength(len);
	  len-=UDP_HEADER_LENGTH;
	  Buffer &data = p.GetPayload().ExtractFront(len);
	  SockRequestResponse write(WRITE,
				    (*cs).connection,
				    data,
				    len,
				    EOK);
	  if (!checksumok) {
	    MinetSendToMonitor(MinetMonitoringEvent("forwarding packet to sock even though checksum failed"));
	  }
	  MinetSend(sock,write);
	} else {
	  MinetSendToMonitor(MinetMonitoringEvent("Unknown port, sending ICMP error message"));
	  IPAddress source; iph.GetSourceIP(source);
	  ICMPPacket error(source,DESTINATION_UNREACHABLE,PORT_UNREACHABLE,p);
	  MinetSendToMonitor(MinetMonitoringEvent("ICMP error message has been sent to host"));
	  MinetSend(mux, error);
	}
      }
      if (event.handle==sock) {
	SockRequestResponse req;
	MinetReceive(sock,req);
	switch (req.type) {
	case CONNECT:
	case ACCEPT:
	  { // ignored, send OK response
	    SockRequestResponse repl;
	    repl.type=STATUS;
	    repl.connection=req.connection;
	    // buffer is zero bytes
	    repl.bytes=0;
	    repl.error=EOK;
	    MinetSend(sock,repl);
	  }
	  break;
	case STATUS:
	  // ignored, no response needed
	  break;
	  // case SockRequestResponse::WRITE:
	case WRITE:
	  {
	    unsigned bytes = MIN_MACRO(UDP_MAX_DATA, req.data.GetSize());
	    // create the payload of the packet
	    Packet p(req.data.ExtractFront(bytes));
	    // Make the IP header first since we need it to do the udp checksum
	    IPHeader ih;
	    ih.SetProtocol(IP_PROTO_UDP);
	    ih.SetSourceIP(req.connection.src);
	    ih.SetDestIP(req.connection.dest);
	    ih.SetTotalLength(bytes+UDP_HEADER_LENGTH+IP_HEADER_BASE_LENGTH);
	    // push it onto the packet
	    p.PushFrontHeader(ih);
	    // Now build the UDP header
	    // notice that we pass along the packet so that the udpheader can find
	    // the ip header because it will include some of its fields in the checksum
	    UDPHeader uh;
	    uh.SetSourcePort(req.connection.srcport,p);
	    uh.SetDestPort(req.connection.destport,p);
	    uh.SetLength(UDP_HEADER_LENGTH+bytes,p);
	    // Now we want to have the udp header BEHIND the IP header
	    p.PushBackHeader(uh);
	    MinetSend(mux,p);
	    SockRequestResponse repl;
	    // repl.type=SockRequestResponse::STATUS;
	    repl.type=STATUS;
	    repl.connection=req.connection;
	    repl.bytes=bytes;
	    repl.error=EOK;
	    MinetSend(sock,repl);
	  }
	  break;
	  // case SockRequestResponse::FORWARD:
	case FORWARD:
	  {
	    ConnectionToStateMapping<UDPState> m;
	    m.connection=req.connection;
	    // remove any old forward that might be there.
	    ConnectionList<UDPState>::iterator cs = clist.FindMatching(req.connection);
	    if (cs!=clist.end()) {
	      clist.erase(cs);
	    }
	    clist.push_back(m);
	    SockRequestResponse repl;
	    // repl.type=SockRequestResponse::STATUS;
	    repl.type=STATUS;
	    repl.connection=req.connection;
	    repl.error=EOK;
	    repl.bytes=0;
	    MinetSend(sock,repl);
	  }
	  break;
	  // case SockRequestResponse::CLOSE:
	case CLOSE:
	  {
	    ConnectionList<UDPState>::iterator cs = clist.FindMatching(req.connection);
	    SockRequestResponse repl;
	    repl.connection=req.connection;
	    // repl.type=SockRequestResponse::STATUS;
	    repl.type=STATUS;
	    if (cs==clist.end()) {
	      repl.error=ENOMATCH;
	    } else {
	      repl.error=EOK;
	      clist.erase(cs);
	    }
	    MinetSend(sock,repl);
	  }
	  break;
	default:
	  {
	    SockRequestResponse repl;
	    // repl.type=SockRequestResponse::STATUS;
	    repl.type=STATUS;
	    repl.error=EWHAT;
	    MinetSend(sock,repl);
	  }
	}
      }
    }
  }
  return 0;
}
