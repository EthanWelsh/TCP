#include <unistd.h>
#include <cstdio>
extern "C" {
#include <libnet.h>
}
#include <sys/poll.h>
#include "config.h"
#include "util.h"
#include "raw_ethernet_packet.h"
#include "error.h"
#include "ethernet.h"

//
// Set to 1 if libnet 1.1 is being used
//
#define LIBNET11 1

/*
   Accept incoming packets from stdin, write them to the wire and
   then signal back the parent that the write is done
*/

using std::cout;
using std::cerr;
using std::endl;

char errbuf[LIBNET_ERRORBUF_SIZE];

#if LIBNET11
libnet_t *interface;
#else
struct libnet_link_int *interface;
#endif

void usage()
{
  fprintf(stderr, "writer device buffersize\n");
}

char *device;

std::deque<RawEthernetPacket> buffer;

int main(int argc, char *argv[])
{
  RawEthernetPacket p;
  int nin,nout;
  int one;
  int fail=ETHERNET_SERVICE_DMA_FAIL;
  int ok = ETHERNET_SERVICE_DMA_DONE;

  one=1;

  if (argc!=3) {
    usage();
    return -1;
  }

  device=argv[1];

#if LIBNET11
  interface=libnet_init(LIBNET_LINK_ADV,device,errbuf);
#else
  interface=libnet_open_link_interface(device,errbuf);
#endif

  if (interface==NULL) {
    fprintf(stderr,"Can't open interface - %s\n",errbuf);
    exit(-1);
  }

  if (ioctl(fileno(stdin),FIONBIO, &one)) {
    fprintf(stderr,"Can't set nonblocking I/O\n");
    exit(-1);
  }

  nin=nout=0;

  while (1) {
#if 1
    if (buffer.empty()) {
      WaitForRead(fileno(stdin));
    }
    if (CanReadNow(fileno(stdin))) {
      RawEthernetPacket p;
      p.Unserialize(fileno(stdin));
      cerr << "writer: newpacket "<<nin<<": " << p << "\n";
      buffer.push_back(p);
      nin++;
    } else {
      if (buffer.empty()) {
	continue;
      }
    }

    p = buffer.front();
    buffer.pop_front();

    memset(&(p.data[p.size]),0,ETHERNET_PACKET_LEN-p.size);
    p.size=MAX(p.size,(size_t)(ETHERNET_HEADER_LEN+ETHERNET_DATA_MIN));

    Packet cp(p);
    cp.ExtractHeaderFromPayload<EthernetHeader>(ETHERNET_HEADER_LEN);
    EthernetHeader eh=cp.FindHeader(Headers::EthernetHeader);
    EthernetAddr ea;
    EthernetProtocol ep;
    eh.GetSrcAddr(ea);
    eh.GetProtocolType(ep);


    if (ea!=MyEthernetAddr() || (ep!=PROTO_ARP && ep!=PROTO_IP)) {
      cerr << "writer: discard packet with header=";
      cerr << eh;
      cerr <<" and length="<<p.size<<endl;
      continue;
    }

    cerr << "writer: launch packet "<<nout<< " with header "<<eh<<" and length="<<p.size<<endl;

#if LIBNET11
  if (libnet_adv_write_link(interface,
			    (u_char *)(p.data),
			    p.size)<0) {
#else
  if (libnet_write_link_layer(interface,
			      (const char *)device,
			      (u_char *)(p.data),
			      p.size)<0) {
#endif
      cerr << "Can't write output packet "<<nout<<" to link\n";
      if (writeall(fileno(stdout),(char*)&fail,sizeof(fail),0,1)!=sizeof(fail)) {
	PERROR();
	return -1;
      }
      kill(ETHERNET_SIGNAL,getppid());
    } else {
      if (writeall(fileno(stdout),(char*)&ok,sizeof(ok),0,1)!=sizeof(ok)) {
	PERROR();
	return -1;
      }
      kill(ETHERNET_SIGNAL,getppid());
      nout++;
    }


#else
    if ((len=readall(fileno(stdin),(char*)&(p.size),sizeof(p.size),1,0))<0) {
      if (errno==EWOULDBLOCK) {
	if (buffer.empty()) {
	  /* Nothing to do.  poll on stdin until something happens */
	  fprintf(stderr,"Nothing to do, waiting for input...");
	  poll(&pfd,1,-1);
	  fprintf(stderr,"OK\n");
	  continue;
	} else {
	  RawEthernetPacket p = buffer.front();
	  buffer.pop_front();
	  cerr << p;
#if LIBNET11
	  if (libnet_adv_write_link(interface,
				    (u_char *)(p.data),
				    p.size)<0) {
#else
          if (libnet_write_link_layer(interface,
				      (const char *)device,
				      (u_char *)(p.data),
				      p.size)<0) {
#endif
	    fprintf(stderr,"Can't write output packet %d to link\n",nout);
	    if (writeall(fileno(stdout),(char*)&fail,sizeof(fail),0,1)!=sizeof(fail)) {
	      PERROR();
	      return -1;
	    }
	    kill(ETHERNET_SIGNAL,getppid());
	  }
	  if (writeall(fileno(stdout),(char*)&ok,sizeof(ok),0,1)!=sizeof(ok)) {
	    PERROR();
	    return -1;
	  }
	  kill(ETHERNET_SIGNAL,getppid());
	  nout++;
	}
      } else {
	perror("Can't read packet length");
	exit(-1);
      }
    } else {
      assert(len==sizeof(int));
      len=readall(fileno(stdin),p.data,p.size,0,1);
      assert(len==p.size);
      fprintf(stderr,"writer: have input\n");
      buffer.push_back(p);
      nin++;
    }
#endif

  }

#if LIBNET11
  libnet_destroy(interface);
#else
  libnet_close_link_interface(interface);
#endif

}







