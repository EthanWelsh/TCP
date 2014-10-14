#include "tcp.h"

#include <netinet/in.h>

//
// Note - original version lost, this is a reconstruction
//

TCPHeader::TCPHeader() : Header(Headers::TCPHeader)
{}

TCPHeader::TCPHeader(const TCPHeader &rhs) : Header(Headers::TCPHeader,rhs)
{}

TCPHeader::TCPHeader(const Header &rhs) : Header(Headers::TCPHeader,rhs)
{}

TCPHeader::TCPHeader(const Buffer &rhs) : Header(Headers::TCPHeader,rhs)
{}


TCPHeader::TCPHeader(const char *buf, const unsigned len) : Header(Headers::TCPHeader,buf,len)
{}

TCPHeader::~TCPHeader()
{}


TCPHeader & TCPHeader::operator=(const TCPHeader &rhs)
{
    Header::operator = (rhs);
    return *this;
}


unsigned TCPHeader::EstimateTCPHeaderLength(Packet &p)
{
    unsigned char len;
    Buffer &b = p.GetPayload();

    b.GetData((char *)&len, 1, 12);

    len = (len >> 4) & 0xf;
    len *= 4;

    return len;

}

void TCPHeader::GetSourcePort(unsigned short &port) const
{
    GetData((char *)&port, 2, 0);
    port = ntohs(port);
}

void TCPHeader::SetSourcePort(const unsigned short &port, const Packet &p)
{
    unsigned short pt = htons(port);

    SetData((char *)&pt, 2, 0);

    RecomputeChecksum(p);
}

void TCPHeader::GetDestPort(unsigned short &port) const
{
    GetData((char *)&port, 2, 2);
    port = ntohs(port);
}

void TCPHeader::SetDestPort(const unsigned short &port, const Packet &p)
{
    unsigned short pt = htons(port);

    SetData((char *)&pt, 2, 2);

    RecomputeChecksum(p);
}

void TCPHeader::GetSeqNum(unsigned int &n) const
{
    GetData((char *)&n, 4, 4);
    n = ntohl(n);
}

void TCPHeader::SetSeqNum(const unsigned int &n, const Packet &p)
{
    unsigned int nt = htonl(n);

    SetData((char*)&nt, 4, 4);

    RecomputeChecksum(p);
}

void TCPHeader::GetAckNum(unsigned int &n) const
{
    GetData((char*)&n, 4, 8);
    n = ntohl(n);
}

void TCPHeader::SetAckNum(const unsigned int &n, const Packet &p)
{
    unsigned int nt=htonl(n);
    
    SetData((char*)&nt,4,8);
    
    RecomputeChecksum(p);
}

void TCPHeader::GetHeaderLen(unsigned char &len) const
{
    GetData((char *)&len, 1, 12);
    len = (len & 0xf0) >> 2;

}

void TCPHeader::SetHeaderLen(const unsigned char &new_len, const Packet &p)
{
  unsigned char tmp_len;

  GetData((char *)&tmp_len, 1, 12);
  tmp_len = (tmp_len & 0x0f) | ((new_len << 2) & 0xf0);
  SetData((char *)&tmp_len, 1, 12);

  RecomputeChecksum(p);
}

void TCPHeader::GetFlags(unsigned char &flags) const
{
    GetData((char *)&flags, 1, 13);
    flags &= 0x3f;
}

void TCPHeader::SetFlags(const unsigned char &new_flags, const Packet &p)
{
  unsigned char ft;

  GetData((char*)&ft,1,13);
  ft&=64+128;
  ft|=(new_flags&(1+2+4+8+16+32));
  SetData((char*)&ft,1,13);

  RecomputeChecksum(p);
}

void TCPHeader::GetWinSize(unsigned short &w) const
{
  GetData((char*)&w,2,14);
  w=ntohs(w);
}

void TCPHeader::SetWinSize(const unsigned short &w, const Packet &p)
{
  unsigned short wt=htons(w);
  SetData((char*)&wt,2,14);
  RecomputeChecksum(p);
}

unsigned short TCPHeader::ComputeChecksum(const Packet &p) const
{
  // assumes we DO have an IP header in the packet already
  IPHeader iph= (IPHeader &) p.FindHeader(Headers::IPHeader);
  IPAddress srcip, destip;
  unsigned char proto;

  iph.GetSourceIP(srcip); srcip=htonl(srcip);
  iph.GetDestIP(destip); destip=htonl(destip);
  iph.GetProtocol(proto);

  unsigned short len, buflen;
  unsigned char iphlen;
  unsigned char tcphlen;

  GetHeaderLen(tcphlen);

  iph.GetTotalLength(len);
  iph.GetHeaderLength(iphlen);

  len-=iphlen*4;

  buflen=((12+len)+(len%2?1:0))/2;

  unsigned short buf[buflen];

  memset((char*)buf,0,2*buflen);

  memcpy((char*)buf,&srcip,4);
  memcpy((char*)(buf+2),&destip,4);
  buf[4]=htons((unsigned short)proto);
  buf[5]=htons(len);

  GetData((char*)&(buf[6]),tcphlen*4,0);

  //YUCK FIX
  ((Packet&)p).GetPayload().GetData((char*)(buf+6+tcphlen*2),len-tcphlen*4,0);

  return ~(OnesComplementSum(buf,buflen));

}

bool TCPHeader::IsCorrectChecksum(const Packet &p) const
{
  unsigned short c;
  GetChecksum(c);
  if (c==0) {
    return true;
  } else {
    return ComputeChecksum(p)==0;
  }
}

void TCPHeader::RecomputeChecksum(const Packet &p)
{
  if (GetSize()<TCP_HEADER_BASE_LENGTH) {
    SetUrgentPtr(0,p);
  }
  SetChecksum(0);
  unsigned short ck=ComputeChecksum(p);
  SetChecksum(ck);
}



void TCPHeader::GetChecksum(unsigned short &checksum) const
{
  GetData((char*)&checksum,2,16);
  checksum=ntohs(checksum);
}

void TCPHeader::SetChecksum(const unsigned short &checksum)
{
  unsigned short ct=htons(checksum);
  SetData((char*)&ct,2,16);
}


void TCPHeader::GetUrgentPtr(unsigned short &up) const
{
    GetData((char *)&up, 2, 18);
    up = ntohs(up);
}

void TCPHeader::SetUrgentPtr(const unsigned short &up, const Packet &p)
{
    unsigned short upt = htons(up);

    SetData((char *)&upt, 2, 18);

    RecomputeChecksum(p);
}


void TCPHeader::GetOptions(struct TCPOptions &o) const
{
    unsigned char hlen;
    
    GetHeaderLen(hlen);
    o.len = (hlen * 4) - TCP_HEADER_BASE_LENGTH;
    
    if (o.len > 0) {
	GetData(o.data, o.len, TCP_HEADER_BASE_LENGTH);
    }
}

void TCPHeader::SetOptions(const struct TCPOptions &o)
{
    SetData(o.data, o.len, TCP_HEADER_BASE_LENGTH);
}


std::ostream & TCPHeader::Print(std::ostream &os) const 
{
    unsigned short sport, dport, winsize, checksum, uptr;
    unsigned int seqno, ackno;
    unsigned char hlen, flags;
    
    GetSourcePort(sport);
    GetDestPort(dport);
    GetSeqNum(seqno);
    GetAckNum(ackno);
    GetHeaderLen(hlen);
    GetFlags(flags);
    GetWinSize(winsize);
    GetChecksum(checksum);
    GetUrgentPtr(uptr);
    

    os << "TCPHeader"
       << "(srcport="     << sport
       << ", destport="   << dport
       << ", seqnum="     << seqno
       << ", acknum="     << ackno
       << ", hlen="       << ((unsigned)hlen) << "(" << (hlen * 4) << " bytes)"
       << ", flags="      << ((unsigned)flags) 
       << "("
       << (IS_URG(flags) ? " URG " : "")
       << (IS_ACK(flags) ? " ACK " : "")
       << (IS_PSH(flags) ? " PSH " : "")
       << (IS_RST(flags) ? " RST " : "")
       << (IS_SYN(flags) ? " SYN " : "")
       << (IS_FIN(flags) ? " FIN " : "")
       << ")"
       << ", winsize="    << winsize
       << ", checksum="   << checksum
       << ", urgentptr="  << uptr
       << ((hlen > 5) ? " HAS OPTIONS" : "")
       << ")";

    return os;    
}
