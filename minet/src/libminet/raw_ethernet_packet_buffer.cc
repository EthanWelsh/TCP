#include "debug.h"
#include "malloc.h"
#include "raw_ethernet_packet_buffer.h"

RawEthernetPacketBuffer::RawEthernetPacketBuffer(int size)
{
  bufsize=size;
  packet = new RawEthernetPacket [size];
  curin=0;
  curout=0;
}

RawEthernetPacketBuffer::~RawEthernetPacketBuffer()
{
  delete [] packet;
}

int RawEthernetPacketBuffer::PushPacket(RawEthernetPacket *packet)
{
  DEBUGPRINTF(8,"RawEthernetPacketBuffer::PushPacket(size=%u, data=%x, curin=%d, curout=%d)\n",packet->size,packet->data, curin,curout);
  if (IsFull()) {
    return PACKETBUFFER_FULL;
  } else {
    this->packet[curin] = *packet;
    curin=(curin+1)%bufsize;
    return PACKETBUFFER_OK;
  }
}

int RawEthernetPacketBuffer::PullPacket(RawEthernetPacket *packet)
{
  if (IsEmpty()) {
    return PACKETBUFFER_EMPTY;
  } else {
    *packet = this->packet[curout];
    curout=(curout+1)%bufsize;
    return PACKETBUFFER_OK;
  }
}


bool RawEthernetPacketBuffer::IsFull() const
{
  return (curin+1)%bufsize == curout ;
}

bool RawEthernetPacketBuffer::IsEmpty() const
{
  return  curout==curin;
}


size_t RawEthernetPacketBuffer::Numitems() const
{
  return curin>=curout ? curin - curout : bufsize - (curout - curin) - 1;
}

