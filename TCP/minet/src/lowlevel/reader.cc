#include <unistd.h>

extern "C" {
#include <pcap.h>
}

#include <assert.h>
#include <string.h>
#include "config.h"
#include "raw_ethernet_packet.h"

#include "debug.h"

#define HOSTNAMELEN 1024
#define PROGLEN 1024

/*
   reader device ipaddr

   print stream of raw packets bound for this machine to stdout in the
   format:  packetsize packetcontents 
*/


void usage() {
    fprintf(stderr, "usage: promisc device host\n");
}
	  
void PacketHandler(u_char * junk,  const struct pcap_pkthdr * header,  const u_char * packet) {
    static int n = 0;

    assert(header->caplen == header->len);
  
    RawEthernetPacket p((char *)packet, (unsigned)(header->len));
    
    p.Serialize(1);
    
    DEBUGPRINTF(5, "wrote packet %d\n",n++);
    
    kill(getppid(), ETHERNET_SIGNAL);
    
    DEBUGPRINTF(5, "reader: sent ETHERNET_SIGNAL to parent\n");

}
  

int main(int argc, char *argv[]) {
    char * dev = NULL;
    char * host = NULL;

    char prog[PROGLEN];
    char ebuf[PCAP_ERRBUF_SIZE];
    pcap_t * pcap = NULL;
    bpf_u_int32 net;
    bpf_u_int32 mask;
    struct bpf_program filter;
    int count = 0;

    char * ip = getenv("MINET_IPADDR");  

    if (argc != 3) { 
	usage();
	return -1;
    }

    if (ip == 0) { 
	fprintf(stderr, "SET MINET_IPADDR");
	return -1;
    }

    dev = argv[1];
    host = argv[2];

    DEBUGPRINTF(10,"Using device %s\n",dev);

    if (pcap_lookupnet(dev, &net, &mask, ebuf)) {
	fprintf(stderr, "Can't get net and mask for %s: %s\n", dev, ebuf);
	return -1;
    }

    DEBUGPRINTF(10,"Net=%x, mask=%x\n",net,mask);
  
    if ((pcap = pcap_open_live(dev, 1518, 1, 0, ebuf)) == NULL) { 
	fprintf(stderr, "Can't open %s: %s\n", dev, ebuf);
	return -1;
    }

    prog[0] = 0;
    //strcpy(prog,"net 10.10/16");


    sprintf(prog, "host %s or broadcast",ip);
  
    DEBUGPRINTF(10, "Filter: '%s'\n", prog);

    if (pcap_compile(pcap, &filter, prog, 0, mask)) {
	fprintf(stderr, "Can't compile filter\n");
	return -1;
    }
    
    if (pcap_setfilter(pcap, &filter)) { 
	fprintf(stderr, "Can't set filter\n");
	return -1;
    }
    
    DEBUGPRINTF(10, "Starting...\n");
    
    while (1) { 
	count += pcap_loop(pcap, -1, &PacketHandler, 0);
	fprintf(stderr, "Error in pcap_loop\n");
    }
    
    DEBUGPRINTF(10, "Handled %d packets\n", count);
    
    return 0;

}
