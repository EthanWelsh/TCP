#include "icmp.h"

#include <netinet/in.h>

using namespace std;

// ***** ICMPHHeader DECLARATIONS *****
ICMPHeader::ICMPHeader() : 
    Header(Headers::ICMPHeader)
{}

ICMPHeader::ICMPHeader(const ICMPHeader &rhs) : 
    Header(Headers::ICMPHeader, rhs)
{}

ICMPHeader::ICMPHeader(const Header &rhs) : 
    Header(Headers::ICMPHeader, rhs)
{}

ICMPHeader::ICMPHeader(const Buffer &rhs) : 
    Header(Headers::ICMPHeader, rhs)
{}

ICMPHeader::ICMPHeader(const char *buf, const unsigned len) : 
    Header(Headers::ICMPHeader, buf, len)
{}

ICMPHeader::~ICMPHeader()
{}


ICMPHeader & ICMPHeader::operator=(const ICMPHeader &rhs)
{
    Header::operator=(rhs);
    return *this;
}





// ***** ICMPHeader ACCESSOR FUNCTIONS *****
void ICMPHeader::GetType(unsigned char &type) const
{
    GetData((char *)&type, 1, 0);
}

void ICMPHeader::SetType(const unsigned char &type, const Packet &p)
{
    SetData((char *)&type, 1, 0);
    RecomputeChecksum(p);
}

void ICMPHeader::GetCode(unsigned char &code) const
{
    GetData((char *)&code, 1, 1);
}

void ICMPHeader::SetCode(const unsigned char &code, const Packet &p)
{
    SetData((char *)&code, 1, 1);
    RecomputeChecksum(p);
}

void ICMPHeader::GetChecksum(unsigned short &checksum) const
{
    GetData((char *)&checksum, 2, 2);
    checksum = ntohs(checksum);
}

void ICMPHeader::SetChecksum(const unsigned short &checksum)
{
    unsigned short set_checksum = htons(checksum);
    SetData((char *)&set_checksum, 2, 2);
}

void ICMPHeader::GetIdentifier(unsigned short &identifier) const
{
    GetData((char *)&identifier, 2, 4);
    //identifier = ntohs(identifier);
}

void ICMPHeader::SetIdentifier(const unsigned short &identifier, const Packet &p)
{
    //unsigned short set_identifier=htons(identifier);
    SetData((char *)&identifier, 2, 4);
    RecomputeChecksum(p);
}

void ICMPHeader::GetSequenceNumber(unsigned short &seqnumber) const
{
    GetData((char *)&seqnumber, 2, 6);
    // seqnumber = ntohs(seqnumber);
}

void ICMPHeader::SetSequenceNumber(const unsigned short &seqnumber, const Packet &p)
{
    // unsigned short set_seqnumber = htons(seqnumber);
    SetData((char *)&seqnumber, 2, 6);
    RecomputeChecksum(p);
}




// ***** ICMPHeader ACCESSOR FUNCTIONS SPECIFIC TO TYPE *****
void ICMPHeader::GetIphandIcmphEightBytes(const Packet &p, Buffer &data)
{
    data.Clear();

    IPHeader iph = p.FindHeader(Headers::IPHeader);
    ICMPHeader icmph = p.FindHeader(Headers::ICMPHeader);

    // add the ip header and the 8 bytes of the icmp header
    data.AddFront(iph);
    data.AddBack(icmph);
}

void ICMPHeader::GetIphandEightBytes(const Packet &p, Buffer &data)
{
    data.Clear();
    Packet pkt(p);

    IPHeader iph = p.FindHeader(Headers::IPHeader);

    // add the ip header and the first 8 bytes
    data.AddFront(iph);
    data.AddBack( pkt.GetPayload().ExtractFront(8));
}

void ICMPHeader::SetIphandEightBytes(Buffer &payload, Buffer &data)
{
    payload = data;
}

void ICMPHeader::GetGatewayAddress(IPAddress &gateway_address) const
{
    GetData((char *)&gateway_address, 4, 4);
    gateway_address = ntohl(gateway_address);
}
void ICMPHeader::SetGatewayAddress(const IPAddress &gateway_address, const Packet &p)
{
    unsigned long set_gateway_address = htonl(gateway_address);

    SetData((char *)&set_gateway_address, 4, 4);

    RecomputeChecksum(p);
}

void ICMPHeader::GetPointer(unsigned char &pointer) const
{
    GetData((char *)&pointer, 1, 4);
}

void ICMPHeader::SetPointer(const unsigned char &pointer, const Packet &p)
{
    SetData((char *)&pointer, 1, 4);
    RecomputeChecksum(p);
}

void ICMPHeader::GetOriginateTimestamp(const Buffer &payload, unsigned long &originate)
{
    payload.GetData((char *)&originate, 4, 0);
    originate = ntohl(originate);
}

void ICMPHeader::SetOriginateTimestamp(Buffer &payload, unsigned long &originate)
{
    unsigned long set_originate = htonl(originate);

    payload.SetData((char *)&set_originate, 4, 0);
}

void ICMPHeader::GetReceiveTimestamp(const Buffer &payload, unsigned long &receive)
{
    payload.GetData((char *)&receive, 4, 4);
    receive = ntohl(receive);
}

void ICMPHeader::SetReceiveTimestamp(Buffer &payload, unsigned long &receive)
{
    unsigned long set_receive = htonl(receive);
    payload.SetData((char *)&set_receive, 4, 4);
}

void ICMPHeader::GetTransmitTimestamp(const Buffer &payload, unsigned long &transmit)
{
    payload.GetData((char *)&transmit, 4, 8);
    transmit = ntohl(transmit);
}

void ICMPHeader::SetTransmitTimestamp(Buffer &payload, unsigned long &transmit)
{
    unsigned long set_transmit = htonl(transmit);
    payload.SetData((char *)&set_transmit, 4, 8);
}

void ICMPHeader::GetAddressMask(const Buffer &payload, IPAddress &address_mask)
{
    payload.GetData((char *)&address_mask, 4, 0);
    address_mask = ntohl(address_mask);
}

void ICMPHeader::SetAddressMask(Buffer &payload, const IPAddress &address_mask)
{
    unsigned long set_address_mask = htonl(address_mask);
    payload.SetData((char *)&set_address_mask, 4, 0);
}




// ***** ICMPHeader HELPER FUNCTIONS *****
unsigned short ICMPHeader::ComputeChecksum(const Packet &p) const
{
    size_t data_length = ((Packet&)p).GetPayload().GetSize();
    size_t total_length = ICMP_HEADER_LENGTH + data_length;
    unsigned short buffer[total_length];

    memset((char *)buffer, 0, 2 * total_length);

    GetData((char *)&(buffer[0]), ICMP_HEADER_LENGTH, 0);

    ((Packet&)p).GetPayload().GetData((char *)&(buffer[ICMP_HEADER_LENGTH]), data_length, 0);
    
    return ~(OnesComplementSum(buffer, total_length));
}

bool ICMPHeader::IsCorrectChecksum(const Packet &p) const
{
    unsigned short c;

    GetChecksum(c);

    if (c == 0) {
	return true;
    } 
	
    return (ComputeChecksum(p) == 0);
}

void ICMPHeader::RecomputeChecksum(const Packet &p)
{
    unsigned short check = 0;
    
    SetChecksum(0);
    check = ComputeChecksum(p);

    SetChecksum(check);
}

void ICMPHeader::GetCurrentTimeInMilliseconds(unsigned long &current)
{
    tm * timeptr = NULL;
    time_t time_value;

    // get time elapsed since 01/01/1970
    time_value = time(NULL);

    // convert to local time
    timeptr = localtime(&time_value);

    int conversion = 1000;
    int seconds = timeptr->tm_sec;
    int minutes = timeptr->tm_min;
    int hours   = timeptr->tm_hour;
    
    // calculate milliseconds after midnight
    current = (long)((seconds * conversion) +
		     (minutes * 60 * conversion) +
		     (hours * 60 * 60 * conversion));
}

std::ostream & ICMPHeader::Print(std::ostream &os) const
{
    unsigned char type, code;
    unsigned short checksum, identifier, seqnumber;

    GetType(type);
    GetCode(code);
    GetChecksum(checksum);
    GetIdentifier(identifier);
    GetSequenceNumber(seqnumber);
    
    os << "ICMPHeader(type=" << unsigned(type)
       << ", code=" << unsigned(code)
       << ", checksum=" << checksum
       << ", identifier=" << identifier
       << ", seqnumber=" << seqnumber
       << ")";

    return os;
}




// ***** ICMPPacket DECLARATIONS *****
ICMPPacket::ICMPPacket() : Packet()
{}

ICMPPacket::ICMPPacket(const Packet &rhs) : Packet(rhs)
{}

ICMPPacket::ICMPPacket(const Packet &original,
		       const IPAddress &source, const IPAddress &destination,
		       const unsigned char &type, const unsigned char &code,
		       const unsigned short &identifier, const unsigned short &seqnumber,
		       Buffer &data) : Packet()
{
    // create headers, modify data
    IPHeader iph;

    iph.SetTOS(0);
    iph.SetTotalLength(48);
    iph.SetProtocol(IP_PROTO_ICMP);
    iph.SetSourceIP(source);
    iph.SetDestIP(destination);

    ICMPHeader icmph;
    icmph.SetType(type, (*this));

    if (unsigned(type) == PARAMETER_PROBLEM) {
	// code is interpreted as the place where the error is

	icmph.SetPointer(code, (*this));
    } else if (unsigned(type) == TIMESTAMP_REQUEST) {
	unsigned long current_time;

	icmph.GetCurrentTimeInMilliseconds(current_time);
	icmph.SetOriginateTimestamp(data, current_time);

    } else if (unsigned(type) == ADDRESSMASK_REQUEST) {
	// set to broadcast ip
	iph.SetDestIP("IP_ADDRESS_BROADCAST");
    }

    payload = data;
    icmph.SetCode(code, (*this));
    icmph.SetIdentifier(identifier, (*this));
    icmph.SetSequenceNumber(seqnumber, (*this));
    
    PushHeader(icmph);
    PushHeader(iph);
}

//request / reply constructors
ICMPPacket::ICMPPacket(const IPAddress &destination,
		       const unsigned char &type, const unsigned char &code,
		       const unsigned short &identifier, const unsigned short &seqnumber)
{
    // create new payload
    unsigned short buffer[56];

    memset((char *)buffer, 0, 2 * 56);

    Buffer payload((char *)buffer, 56);
    
    ICMPPacket p((*this), getenv("MINET_IPADDR"), destination, type, code, identifier, seqnumber, payload);

    *this = p;
}

ICMPPacket::ICMPPacket(const IPAddress &destination,
		       const unsigned char &type, const unsigned char &code)
{
    // create new payload
    unsigned short buffer[56];

    memset((char *)buffer, 0, 2 * 56);

    Buffer payload((char *)buffer, 56);

    ICMPPacket p((*this), getenv("MINET_IPADDR"),  destination, type, code, 0, 0, payload);

    *this = p;
}

ICMPPacket::ICMPPacket(const IPAddress &destination,
		       const unsigned char &type)
{
    // create new payload
    unsigned short buffer[56];

    memset((char *)buffer, 0, 2 * 56);

    Buffer payload((char *)buffer, 56);

    ICMPPacket p((*this), getenv("MINET_IPADDR"),  destination, type, 0, 0, 0, payload);

    *this = p;
}

// error constructors
ICMPPacket::ICMPPacket(const IPAddress &destination,
		       const unsigned char &type, const unsigned char &code,
		       const unsigned short &identifier, const unsigned short &seqnumber,
		       const Packet &p)
{
    // extract necessary data
    Packet pkt(p);
    unsigned short buffer[56];

    memset((char*)buffer, 0, 2 * 56);

    Buffer data((char*)buffer, 56);
    ExtractIphandEightBytes(pkt, data);
    
    ICMPPacket error((*this), getenv("MINET_IPADDR"), destination, type, code, identifier, seqnumber, data);

    *this = error;
}

ICMPPacket::ICMPPacket(const IPAddress &destination,
			 const unsigned char &type, const unsigned char &code,
			 const Packet &p)
{
    // extract necessary data
    Packet pkt(p);
    unsigned short buffer[56];

    memset((char*)buffer, 0, 2 * 56);

    Buffer data((char*)buffer, 56);
    ExtractIphandEightBytes(pkt, data);
    
    ICMPPacket error((*this), getenv("MINET_IPADDR"),  destination, type, code, 0, 0, data);

    *this = error;
}




// ***** ICMPPacket ACCESSOR FUNCTIONS *****
void ICMPPacket::ExtractIphandIcmphEightBytes(const Packet &p, Buffer &data)
{
    data.Clear();

    IPHeader iph = p.FindHeader(Headers::IPHeader);
    ICMPHeader icmph = p.FindHeader(Headers::ICMPHeader);

    // add the ip header and the 8 bytes of the icmp header
    data.AddFront(iph);
    data.AddBack(icmph);
}


void ICMPPacket::ExtractIphandEightBytes(const Packet &p, Buffer &data)
{
    data.Clear();
    Packet pkt(p);

    IPHeader iph = p.FindHeader(Headers::IPHeader);

    // add the ip header and the first 8 bytes
    data.AddFront(iph);
    data.AddBack( pkt.GetPayload().ExtractFront(8));
}


void ICMPPacket::SetIphandEightBytes(Buffer &payload, Buffer &data)
{
    payload = data;
}




// ***** ICMPPacket HELPER FUNCTIONS *****
void ICMPPacket::respond(const Packet &p)
{
    ICMPPacket response(p);
    *this = response;
    
    // get header information
    EthernetHeader eh = PopHeader();
    IPHeader iph      = FindHeader(Headers::IPHeader);
    ICMPHeader icmph  = FindHeader(Headers::ICMPHeader);
 
    // reverse source/ destination ip addresses
    IPAddress source, destination;

    iph.GetSourceIP(source); 
    iph.GetDestIP(destination);
    iph.SetSourceIP(destination);  
    iph.SetDestIP(source);
    
    // handle response/error messaging
    unsigned char icmp_type;  
    unsigned char icmp_code;

    icmph.GetType(icmp_type); 
    icmph.GetCode(icmp_code);
    
    if ( (unsigned(icmp_type) == ECHO_REQUEST) ||
	 (unsigned(icmp_type) == TIMESTAMP_REQUEST) ||
	 (unsigned(icmp_type) == ADDRESSMASK_REQUEST) ) {
	handle_response(iph, icmph, payload, icmp_type);
	    
	// modify outgoing packet
	SetHeader(iph);
	SetHeader(icmph);

    } else if ( (unsigned(icmp_type) == ECHO_REPLY) ||
		(unsigned(icmp_type) == TIMESTAMP_REPLY) ||
		(unsigned(icmp_type) == ADDRESSMASK_REPLY) ||
		(unsigned(icmp_type) == DESTINATION_UNREACHABLE) ||
		(unsigned(icmp_type) == SOURCE_QUENCH) ||
		(unsigned(icmp_type) == REDIRECT) ||
		(unsigned(icmp_type) == TIME_EXCEEDED) ||
		(unsigned(icmp_type) == PARAMETER_PROBLEM)) {

	handle_error(iph, icmph, payload, icmp_type);
	
    } else {
	std::cout << "Invalid ICMP Packet" << std::endl;
    }
}

void ICMPPacket::respond(const RawEthernetPacket &rp)
{
    Packet p(rp);

    p.ExtractHeaderFromPayload<EthernetHeader>(ETHERNET_HEADER_LEN);
    p.ExtractHeaderFromPayload<IPHeader>(IPHeader::EstimateIPHeaderLength(p));
    p.ExtractHeaderFromPayload<ICMPHeader>(ICMP_HEADER_LENGTH);
    
    respond(p);
}

void ICMPPacket::respond_in_ip_module(const Packet &p)
{
    Packet pkt(p);

    pkt.ExtractHeaderFromPayload<ICMPHeader>(ICMP_HEADER_LENGTH);

    respond(pkt);
}

bool ICMPPacket::requires_reply()
{
    return requires_response;
}

void ICMPPacket::handle_response(IPHeader &iph, ICMPHeader &icmph,
				  Buffer &payload, unsigned char &icmp_type)
{
    if (unsigned(icmp_type) == ECHO_REQUEST) {
	// change type to ECHO_REPLY

	icmph.SetType(ECHO_REPLY, (*this));
	requires_response = true;

    } else if (unsigned(icmp_type) == TIMESTAMP_REQUEST) {
	// change type to TIMESTAMP_REPLY
	unsigned long current_time;

	icmph.SetType(TIMESTAMP_REPLY, (*this));
	icmph.GetCurrentTimeInMilliseconds(current_time);
	icmph.SetOriginateTimestamp(payload, current_time);
    } else if (unsigned(icmp_type) == ADDRESSMASK_REQUEST) {
	// change type to ADDRESSMASK_REPLY

	icmph.SetType(ADDRESSMASK_REPLY, (*this));
	icmph.SetAddressMask(payload, "255.255.0.0");
	icmph.RecomputeChecksum( (*this));
	requires_response = true;
    }
}

void ICMPPacket::handle_error(IPHeader &iph, ICMPHeader &icmph, Buffer &payload, unsigned char &icmp_type)
{
    if (unsigned(icmp_type) == REDIRECT) {
	; // modify route table
    }

    
    requires_response = false;
}




// ***** GENERAL HELPER FUNCTIONS *****
void DebugDump(Packet p)
{
    //EthernetHeader eh = p.FindHeader(Headers::EthernetHeader);
    IPHeader iph = p.FindHeader(Headers::IPHeader);
    ICMPHeader icmph = p.FindHeader(Headers::ICMPHeader);
    Buffer payload = p.GetPayload();

    //eh.Print(std::cerr);  
    cerr << std::endl;
    
    iph.Print(std::cerr);     
    std::cerr << std::endl;
    
    icmph.Print(std::cerr);   
    std::cerr<< std::endl;
    
    p.Print(std::cerr);       
    std::cerr << std::endl;
    
    payload.Print(std::cerr); 
    std::cerr << std::endl;
}
