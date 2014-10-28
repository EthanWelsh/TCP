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

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
  MinetHandle mux;

  MinetInit(MINET_OTHER_MODULE);

  mux=MinetIsModuleInConfig(MINET_ETHERNET_MUX) ? MinetConnect(MINET_ETHERNET_MUX) : MINET_NOHANDLE;

  if (mux==MINET_NOHANDLE && MinetIsModuleInConfig(MINET_ETHERNET_MUX)) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ethermux"));
    return -1;
  }

  cerr << "other_module: handling non-IP, non-ARP traffic\n";
  MinetSendToMonitor(MinetMonitoringEvent("other_module: handling non-IP, non-ARP traffic"));

  MinetEvent event;

  while (MinetGetNextEvent(event)==0) {
    if (event.eventtype!=MinetEvent::Dataflow
	|| event.direction!=MinetEvent::IN) {
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    } else {
      if (event.handle==mux) {
	RawEthernetPacket raw;
	MinetReceive(mux,raw);
	cerr << raw << endl;
      }
    }
  }
  return 0;
}
