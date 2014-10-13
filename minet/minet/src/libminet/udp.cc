#include "udp.h"

#include <netinet/in.h>


UDPHeader::UDPHeader() : Header(Headers::UDPHeader)
{
}

UDPHeader::UDPHeader(const UDPHeader &rhs) : Header(Headers::UDPHeader,rhs)
{}

UDPHeader::UDPHeader(const Header &rhs) : Header(Headers::UDPHeader,rhs)
{}

UDPHeader::UDPHeader(const Buffer &rhs) : Header(Headers::UDPHeader,rhs)
{}

UDPHeader::UDPHeader(const char *buf, const unsigned len) : Header(Headers::UDPHeader,buf,len)
{}

UDPHeader::~UDPHeader()
{}


UDPHeader & UDPHeader::operator=(const UDPHeader &rhs)
{
  Header::operator=(rhs);
  return *this;
}


void UDPHeader::GetSourcePort(unsigned short &port) const
{
  GetData((char*)&port,2,0);
  port=ntohs(port);
}

void UDPHeader::SetSourcePort(const unsigned short &port, const Packet &p)
{
  unsigned short pt=htons(port);
  SetData((char*)&pt,2,0);
  RecomputeChecksum(p);
}

void UDPHeader::GetDestPort(unsigned short &port) const
{
  GetData((char*)&port,2,2);
  port=ntohs(port);
}

void UDPHeader::SetDestPort(const unsigned short &port, const Packet &p)
{
  unsigned short pt=htons(port);
  SetData((char*)&pt,2,2);
  RecomputeChecksum(p);
}


void UDPHeader::GetLength(unsigned short &len) const
{
  GetData((char*)&len,2,4);
  len=ntohs(len);
}

void UDPHeader::SetLength(const unsigned short &len, const Packet &p)
{
  unsigned short l=htons(len);
  SetData((char*)&l,2,4);
  RecomputeChecksum(p);
}

unsigned short UDPHeader::ComputeChecksum(const Packet &p) const
{
  // assumes we DO have an IP header in the packet already
  IPHeader iph= (IPHeader &) p.FindHeader(Headers::IPHeader);
  IPAddress srcip, destip;
  unsigned char proto;

  iph.GetSourceIP(srcip); srcip=htonl(srcip);
  iph.GetDestIP(destip); destip=htonl(destip);
  iph.GetProtocol(proto);

  unsigned short len, buflen;

  GetLength(len);

  buflen=((12+len)+(len%2?1:0))/2;

  unsigned short buf[buflen];

  memset((char*)buf,0,2*buflen);

  memcpy((char*)buf,&srcip,4);
  memcpy((char*)(buf+2),&destip,4);
  buf[4]= htons((unsigned short)proto);
  buf[5]= htons(len);

  GetData((char*)&(buf[6]),UDP_HEADER_LENGTH,0);

  //YUCK
  ((Packet&)p).GetPayload().GetData((char*)(buf+10),len-UDP_HEADER_LENGTH,0);

  return ~(OnesComplementSum(buf,buflen));
}



bool UDPHeader::IsCorrectChecksum(const Packet &p) const
{
  unsigned short c;
  GetChecksum(c);
  if (c==0) {
    return true;
  } else {
    return ComputeChecksum(p)==0;
  }
}


void UDPHeader::RecomputeChecksum(const Packet &p)
{
  unsigned short len;
  SetChecksum(0);
  GetLength(len);
  if (len>=UDP_HEADER_LENGTH) {
    unsigned short ck=ComputeChecksum(p);
    SetChecksum(ck);
  }
}


void UDPHeader::GetChecksum(unsigned short &checksum) const
{
  GetData((char*)&checksum,2,6);
  checksum=ntohs(checksum);
}

void UDPHeader::SetChecksum(const unsigned short &checksum)
{
  unsigned short c=htons(checksum);
  SetData((char*)&c,2,6);
}

std::ostream & UDPHeader::Print(std::ostream &os) const
{
  unsigned short srcport, destport, len, checksum;

  GetSourcePort(srcport);
  GetDestPort(destport);
  GetLength(len);
  GetChecksum(checksum);

  os << "UDPHeader(srcport="<<srcport
     << ", destport="<<destport
     << ", len="<<len
     << ", checksum="<<checksum
     << ")";

  return os;
}

