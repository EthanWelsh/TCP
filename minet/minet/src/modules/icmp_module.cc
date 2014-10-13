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

#define DEBUG_SEND 0
#define DEBUG_RECV 0

using std::cout;
using std::cerr;
using std::endl;

struct ICMPState {
  std::ostream & Print(std::ostream &os) const { os <<"ICMPState()"; return os;}

  friend std::ostream &operator<<(std::ostream &os, const ICMPState& L) {
    return L.Print(os);
  }
};

int main(int argc, char *argv[])
{

  MinetHandle ipmux;
  MinetHandle sock;

  ConnectionList<ICMPState> clist;

  MinetInit(MINET_ICMP_MODULE);

  ipmux=MinetIsModuleInConfig(MINET_IP_MUX) ? MinetConnect(MINET_IP_MUX) : MINET_NOHANDLE;
  sock=MinetIsModuleInConfig(MINET_SOCK_MODULE) ? MinetAccept(MINET_SOCK_MODULE) : MINET_NOHANDLE;

  if (ipmux==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_IP_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ipmux"));
    return -1;
  }
  if (sock==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_SOCK_MODULE)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to sock_module"));
    return -1;
  }


  cerr << "icmp_module handling icmp traffic\n";

  MinetSendToMonitor(MinetMonitoringEvent("icmp_module handling icmp traffic"));



  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {

      if (event.handle==ipmux) {
	Packet p;
	MinetReceive(ipmux, p);
	p.ExtractHeaderFromPayload<ICMPHeader>(ICMP_HEADER_LENGTH);

	// received packet information
#if DEBUG_RECV
	cerr << "Received ICMP/Packet: " << endl;
	DebugDump(p);
#endif

	// respond to packet
	ICMPPacket response;
	response.respond(p);

	if (response.requires_reply())
	  {
#if DEBUG_RECV
	    cerr << "Sent Packet: " << endl;
	    DebugDump(response);  cerr << endl;
#endif
	    MinetSend(ipmux, response);
	  }
	else
	  {
	    IPHeader iph = response.FindHeader(Headers::IPHeader);
	    ICMPHeader icmph = response.FindHeader(Headers::ICMPHeader);

	    // make new connection
	    Connection c;
	    iph.GetDestIP(c.dest);
	    iph.GetSourceIP(c.src);
	    iph.GetProtocol(c.protocol);
	    c.srcport = PORT_NONE;
	    c.destport = PORT_NONE;

	    // ConnectionList<ICMPState>::iterator cs = clist.FindMatching(c);
	    // if (cs!=clist.end()) {}

	    Buffer data;  icmph.GetIphandIcmphEightBytes(response, data);
	    data.AddBack(response.GetPayload());
	    SockRequestResponse write(WRITE,
				      c, //(*cs).connection,
				      data,
				      data.GetSize(),
				      EOK);

#if DEBUG_RECV
	    cerr << "Forwarding ICMP Packet to Sock" << endl;
#endif
	    // not sure why this is happening -PAD
	    MinetSend(sock,write);
	  }
      }

      if (event.handle==sock) {
	SockRequestResponse req;
	MinetReceive(sock,req);

	switch (req.type) {
	case CONNECT:
	  break;
	case ACCEPT:
	  {
	    // ignored, send OK response
	    SockRequestResponse repl;
	    // repl.type=SockRequestResponse::STATUS;
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
	case WRITE:
	  {
	    // Not really clear what this is/was trying to do... -PAD
	    //
	    // FIXTHIS
	    /*
	      unsigned bytes = MIN(UDP_MAX_DATA, req.data.GetSize());
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
	    */
	  }
	  break;
	case FORWARD:
	  {
	    cout << "forward" << endl;
	    ConnectionToStateMapping<ICMPState> m;
	    m.connection=req.connection;
	    // remove any old forward that might be there.
	    ConnectionList<ICMPState>::iterator cs = clist.FindMatching(req.connection);
	    if (cs!=clist.end()) {
	      clist.erase(cs);
	    }
	    clist.push_back(m);
	    SockRequestResponse repl;
	    repl.type=STATUS;
	    repl.connection=req.connection;
	    repl.error=EOK;
	    repl.bytes=0;
	    MinetSend(sock,repl);
	  }
	  break;
	case CLOSE:
	  {
	    cout << "close" << endl;
	    ConnectionList<ICMPState>::iterator cs = clist.FindMatching(req.connection);
	    SockRequestResponse repl;
	    repl.connection=req.connection;
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
	    cout << "default" << endl;
	    SockRequestResponse repl;
	    repl.type=STATUS;
	    repl.error=EWHAT;
	    MinetSend(sock,repl);
	  }
	}
      }
    }
  }

  MinetDeinit();
  return 0;
}

