#include <unistd.h>
#include <cstdio>
extern "C" {
#include <libnet.h>
#include <pcap.h>
}
#include <sys/poll.h>
#include "netinet/if_ether.h"
#include "config.h"
#include "util.h"
#include "raw_ethernet_packet.h"
#include "error.h"
#include "ethernet.h"

#define LIBNET11 1

/*
   Bridge code for Virtuoso project
*/


void usage()
{
  cerr<<"usage: bridge device local|remote remotemacaddr otheraddrs*\n\n"
      <<"In local mode, packets sent to any of the ethernet addresses\n"
      <<"  and not originating from remotemacaddr\n"
      <<"  are serialized as RawEthernetPackets to stdout and any\n"
      <<"  RawEthernetPackets seen on stdin are written to the network\n"
      <<"In remote mode, packets received from any of the ethernet\n"
      <<"  addresses are serialized to stdout and any RawEthernetPackets\n"
      <<"  received from stdin are written to the network\n\n";
}


// bridge config
char *device;
deque<EthernetAddr> addresses;
enum {LOCAL, REMOTE} config;


// libnet stuff
char net_errbuf[LIBNET_ERRORBUF_SIZE];
#if LIBNET11
libnet_t *net_interface;
#else
struct libnet_link_int *net_interface;
#endif

// libpcap stuff
const int pcap_proglen=10240;
char pcap_program[pcap_proglen];
pcap_t *pcap_interface;
char pcap_errbuf[PCAP_ERRBUF_SIZE];
struct bpf_program pcap_filter;
bpf_u_int32 pcap_net, pcap_mask;
int pcap_fd;




void Init()
{
  // establish libnet session
#if LIBNET11
  net_interface=libnet_init(LIBNET_LINK_ADV,device,net_errbuf);
#else
  net_interface=libnet_open_link_interface(device,net_errbuf);
#endif
  if (net_interface==NULL) {
    cerr<<"Can't open interface: "<<net_errbuf<<endl;
    exit(-1);
  }

  // establish pcap filter
  if (pcap_lookupnet(device,&pcap_net,&pcap_mask,pcap_errbuf)) {
    cerr<<"Can't get net and mask for "<<device<<": "<<pcap_errbuf<<endl;
    exit(-1);
  }
  cerr << "pcap_net="<<pcap_net<<", pcap_mask="<<pcap_mask<<endl;
  if ((pcap_interface=pcap_open_live(device,1518,1,0,pcap_errbuf))==NULL) {
    cerr<< "Can't open "<<device<<":"<<pcap_errbuf<<endl;
    exit(-1);
  }
  pcap_program[0]=0;
  char dir[5];
  EthernetAddrString addr;
  if (config==LOCAL) {
    strcpy(dir,"dst ");
  } else {
    strcpy(dir,"src ");
  }
  if (config==LOCAL) {
    (*(addresses.begin())).GetAsString(addr);
    sprintf(pcap_program,"(not ether src %s) and (",addr);
  }
  for (deque<EthernetAddr>::const_iterator i=addresses.begin();
       i!=addresses.end();
       ++i) {
    if (i!=addresses.begin()) {
      strcat(pcap_program," or ");
    }
    (*i).GetAsString(addr);
    strcat(pcap_program,"ether ");
    strcat(pcap_program,dir);
    strcat(pcap_program,addr);
  }
  if (config==LOCAL) {
    strcat(pcap_program,")");
  }

  cerr <<"pcap_program='"<<pcap_program<<"'"<<endl;
  if (pcap_compile(pcap_interface,&pcap_filter,pcap_program,0,pcap_mask)) {
    cerr<<"Can't compile filter\n";
    exit(-1);
  }
  if (pcap_setfilter(pcap_interface,&pcap_filter)) {
    cerr<<"Can't set filter\n";
    exit(-1);
  }
  pcap_fd=pcap_fileno(pcap_interface);
}

void ProcessPcap()
{
  struct pcap_pkthdr header;
  const u_char *packet;

  packet=pcap_next(pcap_interface,&header);

  if (packet==NULL) {
    cerr <<"pcap_next returned a null pointer\n";
    exit(-1);
  }

  RawEthernetPacket p((const char *)packet,(unsigned)(header.len));

  cerr << (config==LOCAL ? "Local: " : "Remote: ") <<"Forwarding "<<p<<endl;

  p.Serialize(fileno(stdout));

}

void ProcessStdin()
{
  RawEthernetPacket p;

  p.Unserialize(fileno(stdin));


  cerr << (config==LOCAL ? "Local: " : "Remote: ") << "Emitting "<<p<<endl;

#if LIBNET11
  if (libnet_adv_write_link(net_interface,
			      (u_char *)(p.data),
			      p.size)<0) {
#else
  if (libnet_write_link_layer(net_interface,
			      (const char *)device,
			      (u_char *)(p.data),
			      p.size)<0) {
#endif
      cerr << "Can't write output packet to link\n";
      exit(-1);
  }
}


void Run()
{
  fd_set fds;

  while (1) {
    FD_ZERO(&fds);
    FD_SET(pcap_fd,&fds);
    FD_SET(0,&fds);
    int rc=select(pcap_fd+1,&fds,0,0,0);
    if (rc<0) {
      if (errno==EINTR) {
	// shouldn't happen on linux, but can happen on solaris
	continue;
      }
    } else if (rc==0) {
      // huh? didn't ask for timeouts so just repeat
      continue;
    } else {
      if (FD_ISSET(0,&fds)) {
	ProcessStdin();
      }
      if (FD_ISSET(pcap_fd,&fds)) {
	ProcessPcap();
      }
    }
  }
}





int main(int argc, char *argv[])
{

  if (argc<4) {
    usage();
    return -1;
  }

  device=argv[1];

  if (toupper(argv[2][0])=='L') {
    config=LOCAL;
  } else if (toupper(argv[2][0])=='R') {
    config=REMOTE;
  } else {
    usage();
    return -1;
  }

  cerr << "Configured as a "<<(config==LOCAL ? "local" : "remote")
       <<" bridge on "<<device<<" serving:";

  for (int i=3;i<argc;i++) {
    addresses.push_back(EthernetAddr(argv[i]));
    cerr <<" "<<EthernetAddr(argv[i]);
  }

  cerr << endl;

  Init();
  Run();
}







