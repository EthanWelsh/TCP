#include <cstring>
#include <malloc.h>
#include <algorithm>
#include "raw_ethernet_packet.h"
#include "util.h"
#include "debug.h"

RawEthernetPacket::RawEthernetPacket()
{}


RawEthernetPacket::RawEthernetPacket(const RawEthernetPacket &rhs)
{
  size=rhs.size;
  memcpy(data,rhs.data,size);
}

RawEthernetPacket::RawEthernetPacket(const Packet &rhs)
{
  size=rhs.GetRawSize();
  if (size>ETHERNET_PACKET_LEN) {
    throw;
  }
  rhs.DupeRaw(data,size);
}

RawEthernetPacket::RawEthernetPacket(const char *data, const size_t size)
{
  this->size=size;
  memcpy(this->data,data,size);
}


const RawEthernetPacket & RawEthernetPacket::operator= (const RawEthernetPacket &rhs)
{
  DEBUGPRINTF(10,"RawEthernetPacket::operator=(size=%u, data=%x)\n",rhs.size,rhs.data);
  size=rhs.size;
  memcpy(data,rhs.data,size);
  return *this;
}

const RawEthernetPacket & RawEthernetPacket::operator= (const Packet &rhs)
{
  RawEthernetPacket temp(rhs);
  *this=temp;
  return *this;
}

RawEthernetPacket::~RawEthernetPacket()
{}


Packet & RawEthernetPacket::ConvertToPacket() const
{
  Packet *p = new Packet(data,size);
  return *p;
}


void RawEthernetPacket::Serialize(const int fd) const
{
  if (writeall(fd,(char*)&size,sizeof(size))!=sizeof(size)) {
    throw SerializationException();
  }
  if (writeall(fd,data,size)!=(int)size) {
    throw SerializationException();
  }
}

void RawEthernetPacket::Unserialize(const int fd)
{
  if (readall(fd,(char*)&size,sizeof(size))!=sizeof(size)) {
    throw SerializationException();
  }
  if (readall(fd,data,size)!=(int)size) {
    throw SerializationException();
  }
}

// MIGHT REGRET: Commenting this out here and putting it in util.h instead
//#define MIN(x,y) ((x)<(y) ? (x) : (y))

void RawEthernetPacket::Print(unsigned size, FILE *out) const
{
  fprintf(out,"raw_ethernet_packet: size %-4u first %u bytes: ", this->size, MIN_MACRO(this->size,size));
  printhexbuffer(out,data,MIN_MACRO(this->size,size));
  fprintf(out,"\n");
}

std::ostream & RawEthernetPacket::Print(std::ostream &os) const
{
  char buf[10240];
  unsigned n;
  unsigned i;

  snprintf(buf,2048,"RawEthernetPacket(size=%u, bytes=", size);
  n=strlen(buf);
  for (i=0;i<size;i++) {
    bytetohexbyte(data[i],&(buf[n+2*i]));
  }
  buf[n+2*i]=0;
  os<<(char*)buf;
  os<<", text=\"";
  for (i=0;i<size;i++) {
    char c=data[i];
    if (c>=32 && c<=126) {
      os<<c;
    } else {
      os<<'.';
    }
  }
  os << "\")";

  return os;
}

