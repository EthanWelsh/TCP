#include "ip.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util.h"
#include "error.h"



IPAddress MyIPAddr() {
    char * ip_str = getenv("MINET_IPADDR");

    if (ip_str == NULL) {
	Die("MINET_IPADDR is not set!");
    }
    
    return IPAddress(ip_str);
}


IPAddress::IPAddress() :addr(0)
{}

IPAddress::IPAddress(const char * s)
{
    addr = ntohl(inet_addr(s));
}



IPAddress::IPAddress(const IPAddress &rhs) : addr(rhs.addr)
{}

IPAddress::IPAddress(const unsigned rhs) : addr(rhs)
{}

IPAddress & IPAddress::operator=(const IPAddress &rhs)
{
    this->addr = rhs.addr;
    return *this;
}

IPAddress & IPAddress::operator=(const unsigned rhs)
{
    this->addr = rhs;
    return *this;
}

bool IPAddress::operator==(const IPAddress &rhs) const
{
    return addr == rhs.addr;
}

IPAddress::operator unsigned() const
{
    return addr;
}

char *IPAddress::operator& () const
{
    return (char*)&addr;
}


void IPAddress::Serialize(const int fd) const
{
    if (writeall(fd, (const char *)&addr, sizeof(addr)) != sizeof(addr)) {
	throw SerializationException();
    }
}

void IPAddress::Unserialize(const int fd)
{
    if (readall(fd, (char *)&addr, sizeof(addr)) != sizeof(addr)) {
	throw SerializationException();
    }
}

std::ostream & IPAddress::Print(std::ostream &os) const
{
    struct in_addr in;
    in.s_addr = htonl(addr);
    os << "IPAddress(" << inet_ntoa(in) << ")";
    return os;
}


static unsigned short ip_id_counter = 0;


IPHeader::IPHeader() : Header(Headers::IPHeader)
{
    SetVersion(IP_HEADER_REQUIRED_VERSION);
    SetHeaderLength(IP_HEADER_BASE_LENGTH_IN_WORDS);  // 20 bytes, 5 words
    SetTOS(IP_HEADER_DEFAULT_TOS);
    SetTotalLength(IP_HEADER_BASE_LENGTH);
    SetID(ip_id_counter++);
    SetFlags(IP_HEADER_FLAG_DEFAULT);
    SetFragOffset(0);
    SetTTL(IP_HEADER_TTL_DEFAULT);
    SetProtocol(IP_PROTO_RAW);
    SetSourceIP(IPAddress(0U));
    SetDestIP(IPAddress(0U));
    RecomputeChecksum();
}

IPHeader::IPHeader(const Buffer &rhs) : Header(Headers::IPHeader, rhs)
{
}

IPHeader::IPHeader(const IPHeader &rhs) : Header(rhs)
{
}

IPHeader::IPHeader(const char * buf, unsigned len) : Header(Headers::IPHeader, buf, len)
{
}

IPHeader::~IPHeader()
{}

IPHeader & IPHeader::operator=(const IPHeader &rhs)
{
    Header::operator = (rhs);
    return *this;
}



// This is a helper to determine the header length prior to
// extracting it
// The assumption is that the ethernet header has already been stripped.
unsigned IPHeader::EstimateIPHeaderLength(Packet &p)
{
    unsigned char len;
    Buffer &b = p.GetPayload();

    b.GetData((char *)&len, 1, 0);

    len &= 0xf;

    len *= 4;

    return len;
}

// 4 bit version fields
// automatically set to
void IPHeader::GetVersion(unsigned char &version) const
{
    GetData((char *)&version, 1, 0);
    version &= 0xf0;
    version >>= 4;
}


void IPHeader::SetVersion(const unsigned char &version)
{
    unsigned char t;

    GetData((char *)&t, 1, 0);
    t = (t & 0x0f) | ((version << 4) & 0xf0);
    SetData((char *)&t, 1, 0);

    RecomputeChecksum();
}

void IPHeader::GetHeaderLength(unsigned char &hlen) const
{
    GetData((char *)&hlen, 1, 0);
    hlen &= 0x0f;
}

// note that the header length is recomputed automatically
void IPHeader::SetHeaderLength(const unsigned char &hlen)
{
    unsigned char t;

    GetData((char *)&t, 1, 0);
    t = (t & 0xf0) | (hlen & 0x0f);
    SetData((char *)&t, 1, 0);

    RecomputeChecksum();
}

void IPHeader::GetTOS(unsigned char &tos) const
{
    GetData((char *)&tos, 1, 1);
}

void IPHeader::SetTOS(const unsigned char &tos)
{
    SetData((char *)&tos, 1, 1);
    RecomputeChecksum();
}

void IPHeader::GetTotalLength(unsigned short &len) const
{
    GetData((char *)&len, 2, 2);
    len=ntohs(len);
}

// note that total length is automatically computed
void IPHeader::SetTotalLength(const unsigned short &len)
{
    unsigned short l = htons(len);

    SetData((char *)&l, 2, 2);

    RecomputeChecksum();
}


void IPHeader::GetID(unsigned short &id) const
{
    GetData((char *)&id, 2, 4);
    id = ntohs(id);
}

void IPHeader::SetID(const unsigned short &id)
{
    unsigned short i = htons(id);

    SetData((char *)&i, 2, 4);

    RecomputeChecksum();
}

void IPHeader::GetFlags(unsigned char &flags) const
{
    GetData((char *)&flags, 1, 6);
    flags = (0x7 & (flags >> 5));
}

void IPHeader::SetFlags(const unsigned char &flags)
{
    unsigned char t;

    GetData((char *)&t, 1, 6);
    t &= 0x1f;
    t |= ((0x7 & flags) << 5);
    SetData((char *)&t, 1, 6);

    RecomputeChecksum();
}

void IPHeader::GetFragOffset(unsigned short &offset) const
{
    GetData((char *)&offset, 2, 6);

    offset = ntohs(offset);
    offset &= 0x1fff;
}


void IPHeader::SetFragOffset(const unsigned short &offset)
{
    unsigned char f;
    unsigned short o = htons(offset);

    GetFlags(f);
    SetData((char *)&o, 2, 6);
    SetFlags(f);

    RecomputeChecksum();
}


void IPHeader::GetTTL(unsigned char &ttl) const
{
    GetData((char *)&ttl, 1, 8);
}

void IPHeader::SetTTL(const unsigned char &ttl)
{
    SetData((char *)&ttl, 1, 8);
    RecomputeChecksum();
}

void IPHeader::GetProtocol(unsigned char &proto) const
{
    GetData((char *)&proto, 1, 9);
}

void IPHeader::SetProtocol(const unsigned char &proto)
{
    SetData((char *)&proto, 1, 9);
    RecomputeChecksum();
}


unsigned short IPHeader::ComputeChecksum() const
{
    unsigned short buf[IP_HEADER_MAX_LENGTH / 2];
    unsigned char len;

    GetHeaderLength(len);
    len *= 4;

    GetData((char *)buf, len, 0);

    return ~(OnesComplementSum(buf, len / 2));
}


bool IPHeader::IsChecksumCorrect() const
{
    return ComputeChecksum() == 0;
}

void IPHeader::RecomputeChecksum()
{
    SetChecksum(0);
    SetChecksum(ComputeChecksum());
}

void IPHeader::GetChecksum(unsigned short &checksum) const
{
    GetData((char *)&checksum, 2, 10);
    checksum = ntohs(checksum);
}

// Note that this will be recomputed every time one of the set calls is run
void IPHeader::SetChecksum(const unsigned short &checksum)
{
    unsigned short c = htons(checksum);
    SetData((char *)&c, 2, 10);
}

void IPHeader::GetSourceIP(IPAddress &addr) const
{
    GetData(&addr, 4, 12);
    addr = ntohl(addr);
}

void IPHeader::SetSourceIP(const IPAddress &addr)
{
    unsigned a = htonl(addr);
    SetData((char *)&a, 4, 12);
    RecomputeChecksum();
}

void IPHeader::GetDestIP(IPAddress &addr) const
{
    GetData(&addr, 4, 16);
    addr = ntohl(addr);
}

void IPHeader::SetDestIP(const IPAddress &addr)
{
    unsigned a = htonl(addr);

    SetData((char *)&a, 4, 16);

    RecomputeChecksum();
}


void IPHeader::GetOptions(IPOptions &opt) const
{
    unsigned char len;

    GetHeaderLength(len);
    len = len * 4;

    if (len > IP_HEADER_BASE_LENGTH) {
	GetData(opt.data, len - IP_HEADER_BASE_LENGTH, IP_HEADER_BASE_LENGTH);
	opt.len = len - IP_HEADER_BASE_LENGTH;
    } else {
	opt.len = 0;
    }
}


void IPHeader::SetOptions(const IPOptions &opt)
{
    SetData(opt.data, opt.len, IP_HEADER_BASE_LENGTH);

    short len = (IP_HEADER_BASE_LENGTH + opt.len) / 4;
    SetHeaderLength(len);

    RecomputeChecksum();
}


std::ostream & IPHeader::Print(std::ostream &os) const
{
    bool isvalidchecksum;
    unsigned char version, hlen, tos, flags, ttl, proto;
    unsigned short len, id, fragoff, checksum, computedchecksum;
    IPAddress src, dest;

    GetVersion(version);
    GetHeaderLength(hlen);
    GetTOS(tos);
    GetTotalLength(len);
    GetID(id);
    GetFlags(flags);
    GetFragOffset(fragoff);
    GetTTL(ttl);
    GetProtocol(proto);
    GetChecksum(checksum);
    GetSourceIP(src);
    GetDestIP(dest);

    isvalidchecksum = IsChecksumCorrect();

    computedchecksum = ComputeChecksum();

    os << "IPHeader"
       << "( version=" << unsigned(version)
       << ", hlen=" << unsigned(hlen) << " (" << unsigned(hlen) * 4 << ")"
       << ", tos="  << unsigned(tos)
       << ", len="  << len
       << ", id="   << id
       << ", flags=" << unsigned(flags) 
       << "("
       << ((flags & IP_HEADER_FLAG_RESERVED) ? "RESERVED " : "")
       << ((flags & IP_HEADER_FLAG_DONTFRAG) ? "DO NOT FRAGMENT " : "OK TO FRAGMENT ")
       << ((flags & IP_HEADER_FLAG_MOREFRAG) ? "MORE FRAGMENTS" : "NO MORE FRAGMENTS") 
       << ")"
       << ", fragoff=" << fragoff
       << ", ttl=" << unsigned(ttl)
       << ", proto=" << unsigned(proto) 
       << "("
       << ((proto == IP_PROTO_UDP) ? "UDP" :
	   (proto == IP_PROTO_TCP) ? "TCP" :
	   (proto == IP_PROTO_ICMP) ? "ICMP" :
	   (proto == IP_PROTO_RAW) ? "RAW" :
	   (proto == IP_PROTO_IP) ? "IP" : 
	   "UNKNOWN") 
       << ")"
       << ", checksum=" << checksum << (isvalidchecksum ? "(valid)" : "(INVALID)")
       << ", computedchecksum=" << computedchecksum
       << ", src=" << src
       << ", dst=" << dest
       << (((hlen * 4) > IP_HEADER_BASE_LENGTH) ? "OPTIONS" : "noopts") 
       << " )";

    return os;
}






