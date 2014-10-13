#include "arp.h"

#include <netinet/in.h>

ARPPacket::ARPPacket() : Packet()
{}

ARPPacket::ARPPacket(const ARPPacket &rhs) : Packet(rhs)
{}

ARPPacket::ARPPacket(const RawEthernetPacket &rhs) : Packet(rhs) {
    ExtractHeaderFromPayload<EthernetHeader>(14);
}

ARPPacket::ARPPacket(const Packet &rhs) : Packet(rhs)
{}

ARPPacket::ARPPacket(const OpcodeType      opcode,
                     const EthernetAddr   &sender_hw_addr,
                     const IPAddress      &sender_ip_addr,
                     const EthernetAddr   &target_hw_addr,
                     const IPAddress      &target_ip_addr) : Packet()
{
    SetHWAddressSpace(ETHERNET_HW_ADXSPACE);
    SetProtocolAddressSpace(IP_PROTO_ADXSPACE);
    SetHWAddressLen(6);
    SetProtoAddressLen(4);
    SetOpcode(opcode);
    
    SetSenderEthernetAddr(sender_hw_addr);
    SetSenderIPAddr(sender_ip_addr);
    SetTargetEthernetAddr(target_hw_addr);
    SetTargetIPAddr(target_ip_addr);
}


const ARPPacket & ARPPacket::operator=(const ARPPacket &rhs)
{
    Packet::operator = (rhs);
    return *this;
}

ARPPacket::~ARPPacket()
{}


bool ARPPacket::IsIPToEthernet() const
{
    ARPHWAddressSpaceType hw, proto;
    EthernetProtocol eproto;

    EthernetHeader header = FindHeader(Headers::EthernetHeader);

    header.GetProtocolType(eproto);

    if (eproto != PROTO_ARP) {
	return false;
    }

    GetHWAddressSpace(hw);
    GetProtocolAddressSpace(proto);
    
    return ((hw == ETHERNET_HW_ADXSPACE) && (proto == IP_PROTO_ADXSPACE));
}

bool ARPPacket::IsIPToEthernetRequest() const
{
    OpcodeType oc;

    GetOpcode(oc);

    return (IsIPToEthernet() && (oc == Request)) ;
}

bool ARPPacket::IsIPToEthernetReply() const
{
    OpcodeType oc;

    GetOpcode(oc);

    return (IsIPToEthernet() && (oc == Reply)) ;
}

void ARPPacket::GetHWAddressSpace(ARPHWAddressSpaceType &haddr) const
{
    payload.GetData((char *)&haddr, 2, 0);
    haddr = ntohs(haddr);
}

void ARPPacket::SetHWAddressSpace(const ARPHWAddressSpaceType &haddr)
{
    ARPHWAddressSpaceType haddr2 = htons(haddr);
    payload.SetData((const char *)&haddr2, 2, 0);
}


void ARPPacket::GetProtocolAddressSpace(ARPProtoAddressSpaceType &paddr) const
{
    payload.GetData((char *)&paddr, 2, 2);
    paddr = ntohs(paddr);
}

void ARPPacket::SetProtocolAddressSpace(const ARPProtoAddressSpaceType &paddr)
{
    ARPProtoAddressSpaceType paddr2 = htons(paddr);
    payload.SetData((const char *)&paddr2, 2, 2);
}


void ARPPacket::GetHWAddressLen(ARPHWAddressLenType &hlen) const
{
    payload.GetData((char *)&hlen, 1, 4);
}

void ARPPacket::SetHWAddressLen(const ARPHWAddressLenType &hlen)
{
    payload.SetData((const char *)&hlen, 1, 4);
}

void ARPPacket::GetProtoAddressLen(ARPProtoAddressLenType &hlen) const
{
    payload.GetData((char *)&hlen, 1, 5);
}

void ARPPacket::SetProtoAddressLen(const ARPProtoAddressLenType &hlen)
{
    payload.SetData((const char *)&hlen, 1, 5);
}

void ARPPacket::GetOpcode(OpcodeType &opcode) const
{
    ARPOpcodeType oc;

    payload.GetData((char *)&oc, 2, 6);
    oc = ntohs(oc);
    opcode = (oc == ARP_REQUEST_OPCODE) ? Request : Reply;
}

void ARPPacket::SetOpcode(const OpcodeType &opcode)
{
    ARPOpcodeType oc = (opcode==Request) ? ARP_REQUEST_OPCODE : ARP_REPLY_OPCODE;
    oc = htons(oc);
    payload.SetData((const char *)&oc, 2, 6);
}


void ARPPacket::SetSenderEthernetAddr(const EthernetAddr &sender_hw_addr)
{
    payload.SetData((const char *)(sender_hw_addr.addr), 6, 8);
}

void ARPPacket::GetSenderEthernetAddr(EthernetAddr &sender_hw_addr) const
{
    payload.GetData((char *)(sender_hw_addr.addr), 6, 8);
}

void ARPPacket::SetSenderIPAddr(const IPAddress &sender_ip_addr)
{
#if 1
    IPAddress temp = htonl(sender_ip_addr);
#else
    IPAddress temp = sender_ip_addr;
#endif

    payload.SetData((const char *)&temp, 4, 14);
}

void ARPPacket::GetSenderIPAddr(IPAddress &sender_ip_addr) const
{
    payload.GetData((char *)&sender_ip_addr, 4, 14);
#if 1
    sender_ip_addr = ntohl(sender_ip_addr);
#endif
}

void ARPPacket::SetTargetEthernetAddr(const EthernetAddr &target_hw_addr)
{
    payload.SetData((const char *)(target_hw_addr.addr), 6, 18);
}

void ARPPacket::GetTargetEthernetAddr(EthernetAddr &target_hw_addr) const
{
    payload.GetData((char *)(target_hw_addr.addr), 6, 18);
}

void ARPPacket::SetTargetIPAddr(const IPAddress &target_ip_addr)
{
#if 1
    IPAddress temp = htonl(target_ip_addr);
#else
    IPAddress temp = target_ip_addr;
#endif
    payload.SetData((const char *)&temp, 4, 24);
}

void ARPPacket::GetTargetIPAddr(IPAddress &target_ip_addr) const
{
    payload.GetData((char *)&target_ip_addr, 4, 24);
#if 1
    target_ip_addr = ntohl(target_ip_addr);
#endif
}

std::ostream &ARPPacket::Print(std::ostream &os) const
{
    ARPHWAddressSpaceType    hwadxspace;
    ARPProtoAddressSpaceType protoadxspace;
    ARPHWAddressLenType      hwlen;
    ARPProtoAddressLenType   protolen;
    EthernetAddr             senderhw, targethw;
    IPAddress                senderip, targetip;
    OpcodeType               opcode;

    GetHWAddressSpace(hwadxspace);
    GetProtocolAddressSpace(protoadxspace);
    GetHWAddressLen(hwlen);
    GetProtoAddressLen(protolen);
    GetSenderEthernetAddr(senderhw);
    GetSenderIPAddr(senderip);
    GetTargetEthernetAddr(targethw);
    GetTargetIPAddr(targetip);
    GetOpcode(opcode);

    os << "ARP(hwaddrspace="   << hwadxspace 
       << ", protoaddrspace="  << protoadxspace
       << ", hwlen="           << (int)hwlen 
       << ", protolen="        << (int)protolen
       << ", opcode="          << ((opcode == Request) ? "Request" : "Reply")
       << ", senderhw="        << senderhw 
       << ", senderip="        << senderip
       << ", targethw="        << targethw 
       << ", targetip="        << targetip
       << ")";
    
  return os;
}


ARPRequestResponse::ARPRequestResponse() : ipaddr(0U), 
					   ethernetaddr("00:00:00:00:00:00"), 
					   flag(RESPONSE_UNKNOWN)
{}

ARPRequestResponse::ARPRequestResponse(const ARPRequestResponse &rhs) : ipaddr(rhs.ipaddr), 
									ethernetaddr(rhs.ethernetaddr), 
									flag(rhs.flag)
{}

ARPRequestResponse::ARPRequestResponse(const IPAddress &ip, const EthernetAddr &hw, const Flag f) : ipaddr(ip), 
												    ethernetaddr(hw), 
												    flag(f)
{}

ARPRequestResponse & ARPRequestResponse::operator=(const ARPRequestResponse &rhs)
{
    ipaddr       = rhs.ipaddr;
    ethernetaddr = rhs.ethernetaddr;
    flag         = rhs.flag;
    
    return *this;
}

ARPRequestResponse::~ARPRequestResponse()
{}


void ARPRequestResponse::Serialize(const int fd) const
{
    if (writeall(fd, (const char *)&ipaddr, sizeof(ipaddr)) != sizeof(ipaddr)) {
	throw SerializationException();
    }

    ethernetaddr.Serialize(fd);

    if (writeall(fd, (const char *)&flag, sizeof(flag)) != sizeof(flag)) {
	throw SerializationException();
    }
}


void ARPRequestResponse::Unserialize(const int fd)
{
    if (readall(fd, (char *)&ipaddr, sizeof(ipaddr)) != sizeof(ipaddr)) {
	throw SerializationException();
    }

    ethernetaddr.Unserialize(fd);

    if (readall(fd, (char *)&flag, sizeof(flag)) != sizeof(flag)) {
	throw SerializationException();
    }
}

std::ostream & ARPRequestResponse::Print(std::ostream &os) const
{
    os << "ARPRequestResponse"
       << "( ipaddr="        << ipaddr 
       << ", ethernetaddr="  << ethernetaddr
       << ", flag="          << ((int)flag)
       << " "
       << ((flag == REQUEST)          ? "REQUEST" :
	   (flag == RESPONSE_OK)      ? "RESPONSE_OK" :
	   (flag == RESPONSE_UNKNOWN) ? "RESPONSE_UNKNOWN" :
	   "UNKNOWN_FLAG") 
       << ")";
    
  return os;
}


void ARPCache::Update(const ARPRequestResponse &x)
{
    data[x.ipaddr] = x;
}

void ARPCache::Lookup(ARPRequestResponse &x) const
{
    DataType::const_iterator i = data.find(x.ipaddr);
    
    if (i == data.end()) {
	x.flag = ARPRequestResponse::RESPONSE_UNKNOWN;
    } else {
	x = (*i).second;
    }
}

void ARPCache::Delete(const IPAddress &ipaddr)
{
    data.erase(data.find(ipaddr));
}

std::ostream & operator<< (std::ostream &os, std::pair<IPAddress,ARPRequestResponse> &pair)
{
    os << (pair.second) << " " ;
    return os;
}

std::ostream & ARPCache::Print(std::ostream &os) const
{
    os << "ARPCache" << "( size=" << data.size();

    os << " contents={";    
    for_each(data.begin(), data.end() ,PrintFunc<std::pair<IPAddress,ARPRequestResponse> >(os));
    os << "}";

    os << ")";

    return os;
}
