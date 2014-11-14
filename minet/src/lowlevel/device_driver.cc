#include <cstdio>
#include <cassert>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <libnet.h>
}

#include "config.h"
#include "error.h"
#include "ethernet.h"
#include "raw_ethernet_packet_buffer.h"
#include "debug.h"


#define DEBUG_SEND 0
#define DEBUG_RECV 0

using std::cerr;

int MyISR(int device, int service);

EthernetConfig myconfig={0,0,MyISR};
RawEthernetPacketBuffer *input_queue, *output_queue;

void usage()
{
  fprintf(stderr,"usage: device_driver input_queue_size output_queue_size\n");
}

int ReceiveNextPacket()
{
  RawEthernetPacket packet;
  int rc;

  DEBUGPRINTF(5,"ReceiveNextPacket\n");
  if (EthernetGetNextPacket(&myconfig,&packet)<0) {
    fprintf(stderr,"Can't get next packet!\n");
    return -1;
  }
#if DEBUG_RECV
  cerr << "device: received packet :" << packet<<"\n";
#endif
  rc=input_queue->PushPacket(&packet);
  switch (rc) {
  case PACKETBUFFER_OK:
    DEBUGPRINTF(5,"Pushed packet into input queue\n");
    break;
  case PACKETBUFFER_FULL:
    DEBUGPRINTF(5,"Packet discarded - input queue is full\n");
    break;
  default:
    DEBUGPRINTF(5,"Unknown packet_buffer error\n");
    break;
  }
  return 0;
}

int SendNextPacket()
{
  RawEthernetPacket packet;
  int rc;

  DEBUGPRINTF(5,"SendNextPacket\n");
  rc=output_queue->PullPacket(&packet);
  switch (rc) {
  case PACKETBUFFER_OK:
    DEBUGPRINTF(5,"Initsending next outgoing packet\n");
#if DEBUG_SEND
    cerr << "device: sending packet :" << packet <<"\n";
#endif
    if (EthernetInitiateSend(&myconfig,&packet)!=0) {
      DEBUGPRINTF(5,"Initsend failed: packet dropped\n");
      cerr << "device_driver: send failed\n";
    }
    break;
  case PACKETBUFFER_EMPTY:
    DEBUGPRINTF(5,"No outgoing packets remain\n");
      break;
  default:
    DEBUGPRINTF(5,"Unknown packet_buffer error\n");
    break;
  }
  return 0;
}


int MyISR(int device, int service)
{
  assert(device==myconfig.device);

  switch (service) {
  case ETHERNET_SERVICE_PACKET_READY:
    DEBUGPRINTF(5,"New packet available\n");
    return ReceiveNextPacket();
    break;
  case ETHERNET_SERVICE_OUTPUT_BUFFER_FULL:
    DEBUGPRINTF(5,"Ethernet output buffer full, packet dropped\n");
    return 0;
    break;
  case ETHERNET_SERVICE_DMA_DONE:
    DEBUGPRINTF(5,"Outgoing DMA done\n");
    return SendNextPacket();
    break;
  case ETHERNET_SERVICE_DMA_FAIL:
    DEBUGPRINTF(5,"Outgoing DMA FAILED - packet dropped\n");
    return SendNextPacket();
    break;
  default:
    DEBUGPRINTF(5,"Unknown service request\n");
    return -1;
    break;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  int rc;
  int input_queue_size, output_queue_size;

  int muxin, muxout;

  fd_set input, output;
  sigset_t sigset;

  RawEthernetPacket packet;

  if (argc!=3) {
    usage();
    exit(-1);
  }

  input_queue_size=atoi(argv[1]);
  output_queue_size=atoi(argv[2]);

  input_queue=new RawEthernetPacketBuffer(input_queue_size);
  output_queue= new RawEthernetPacketBuffer(output_queue_size);

  if (input_queue==NULL || output_queue==NULL) {
    DEBUGPRINTF(5,"ISR: Can't allocate memory for queues\n");
    exit(-1);
  }

  muxout = open(ether2mux_fifo_name,O_WRONLY);
  muxin = open(mux2ether_fifo_name,O_RDONLY);

  EthernetStartup(&myconfig);


  cerr << "device_driver operating\n";

  int maxfd=0;

  while (1) {
    FD_ZERO(&input);
    FD_ZERO(&output);
    FD_SET(muxin,&input); maxfd=muxin;
    if (input_queue->Numitems()>0) {
      FD_SET(muxout,&output); maxfd=MAX(maxfd,muxout);
    }
    rc = select(maxfd+1,&input,&output,0,0);

    DEBUGPRINTF(5,"select done rc=%d\n",rc);

    if (rc<0) {
      if (errno==EINTR) {
	continue;
      } else {
	DEBUGPRINTF(5,"Unexpected error in select\n");
	exit(-1);
      }
    } else if (rc==0) {
      DEBUGPRINTF(5, "Unexpected timeout in select\n");
      exit(-1);
    } else if (rc>0) {
      DEBUGPRINTF(5,"some fds are available...\n",rc);
      sigemptyset(&sigset);
      sigaddset(&sigset,ETHERNET_SIGNAL);
      sigprocmask(SIG_BLOCK,&sigset,0);
      /* OK, now we are in critical section */
      if (FD_ISSET(muxout,&output)) {
	DEBUGPRINTF(5,"ISR about to output next packet to mux\n");
	/* we can output the next packet now, if there is one */
	if (input_queue->PullPacket(&packet)==PACKETBUFFER_OK) {
	  packet.Serialize(muxout);
#if DEBUG_RECV
	  cerr << "New input packet: "<<packet<<"\n";
#endif
	} else {
	  DEBUGPRINTF(5,"No packets available, so ISR did not output one\n");
	}
      }
      if (FD_ISSET(muxin,&input)) {
	DEBUGPRINTF(3,"ISR about to input next packet from mux\n");
	/* we can input the next packet now */
	packet.Unserialize(muxin);
#if DEBUG_SEND
	cerr << "New output packet: "<<packet<<"\n";
#endif
	if (output_queue->PushPacket(&packet)!=PACKETBUFFER_OK) {
	  DEBUGPRINTF(3,"Ouput queue full, packet dropped\n");
	}
	/* Now see if we have to start up the packet send */
	if (output_queue->Numitems()>0) {
	  SendNextPacket();
	}
      }
      /* finish critical section */
      sigprocmask(SIG_UNBLOCK,&sigset,0);
    }
  }
  return 0;
}
