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

using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
  MinetHandle mux;
  MinetHandle sock;

  MinetInit(MINET_IP_OTHER_MODULE);

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

  cerr << "ipother_module handling non-UDP, non-TCP, non-ICMP traffic......."<<endl;

  MinetSendToMonitor(MinetMonitoringEvent("ipother_module handling non-UDP, non-TCP, non-ICMP traffic........"));

  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {
      if (event.handle==mux) {
	Packet p;
	MinetReceive(mux,p);
      }
      if (event.handle==sock) {
	SockRequestResponse req;
	MinetReceive(sock,req);
	switch (req.type) {
	case STATUS:
	  // ignored, no response needed
	  break;
	case CONNECT:
	case ACCEPT:
	case WRITE:
	case FORWARD:
	case CLOSE:
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
