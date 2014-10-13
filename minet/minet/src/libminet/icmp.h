#ifndef _icmp
#define _icmp

#include <iostream>
#include <time.h>
#include "Minet.h"

const unsigned short ICMP_HEADER_LENGTH = 8;
const unsigned short DEFAULT            = 0;

// types
const unsigned char ECHO_REPLY                 = 0;  // RESPONSE
const unsigned char DESTINATION_UNREACHABLE    = 3;  // ERROR
const unsigned char SOURCE_QUENCH              = 4;  // ERROR
const unsigned char REDIRECT                   = 5;  // ERROR
const unsigned char ECHO_REQUEST               = 8;  // RESPONSE
const unsigned char TIME_EXCEEDED              = 11; // ERROR
const unsigned char PARAMETER_PROBLEM          = 12; // ERROR
const unsigned char TIMESTAMP_REQUEST          = 13; // RESPONSE
const unsigned char TIMESTAMP_REPLY            = 14; // RESPONSE
const unsigned char ADDRESSMASK_REQUEST        = 17; // RESPONSE
const unsigned char ADDRESSMASK_REPLY          = 18; // RESPONSE

// codes
const unsigned char NETWORK_UNREACHABLE        = 0;
const unsigned char HOST_UNREACHABLE           = 1;
const unsigned char PROTOCOL_UNREACHABLE       = 2;
const unsigned char PORT_UNREACHABLE           = 3;
const unsigned char FRAGMENTATION_NEEDED       = 4;
const unsigned char SOURCE_ROUTE_FAILED        = 5;

const unsigned char REDIRECT_FOR_NETWORK       = 0;
const unsigned char REDIRECT_FOR_HOST          = 1;
const unsigned char REDIRECT_FOR_TOS_NETWORK   = 2;
const unsigned char REDIRECT_FOR_TOS_HOST      = 3;

const unsigned char TTL_EQUALS_ZERO_DURING_TRANSIT    = 0;
const unsigned char TTL_EQUALS_ZERO_DURING_REASSEMBLY = 1;

const unsigned char IP_HEADER_BAD           = 0;
const unsigned char REQUIRED_OPTION_MISSING = 1;


// helper functions
void DebugDump(Packet p);

class ICMPHeader : public Header {
 public:
    ICMPHeader();
    ICMPHeader(const ICMPHeader &rhs);
    ICMPHeader(const Header &rhs);
    ICMPHeader(const Buffer &rhs);
    ICMPHeader(const char *buf, const unsigned len);
    virtual ~ICMPHeader();
    ICMPHeader & operator=(const ICMPHeader &rhs);

    // accessor functions
    void GetType(unsigned char &type) const;
    void SetType(const unsigned char &type, const Packet &p);
    void GetCode(unsigned char &code) const;
    void SetCode(const unsigned char &code, const Packet &p);
    void GetChecksum(unsigned short &checksum) const;
    void SetChecksum(const unsigned short &checksum);
    void GetIdentifier(unsigned short &identifier) const;
    void SetIdentifier(const unsigned short &identifier, const Packet &p);
    void GetSequenceNumber(unsigned short &seqnumber) const;
    void SetSequenceNumber(const unsigned short &seqnumber, const Packet &p);
    
    // accessor functions specific to type
    // types 3, 4, 5, 11, 12 (all error types)
    void GetIphandIcmphEightBytes(const Packet &p, Buffer &data);
    void GetIphandEightBytes(const Packet &p, Buffer &data);
    void SetIphandEightBytes(Buffer &payload, Buffer &data);

    // type 5: redirect
    void GetGatewayAddress(IPAddress &gateway_address) const;
    void SetGatewayAddress(const IPAddress &gateway_address, const Packet &p);

    // type 12: parameter problem
    void GetPointer(unsigned char &pointer) const;
    void SetPointer(const unsigned char &pointer, const Packet &p);

    // type 13, 14: timestamp
    void GetOriginateTimestamp(const Buffer &payload, unsigned long &originate);
    void SetOriginateTimestamp(Buffer &payload, unsigned long &originate);
    void GetReceiveTimestamp(const Buffer &payload, unsigned long &receive);
    void SetReceiveTimestamp(Buffer &payload, unsigned long &receive);
    void GetTransmitTimestamp(const Buffer &payload, unsigned long &transmit);
    void SetTransmitTimestamp(Buffer &payload, unsigned long &transmit);

    // type 17, 18: address mask
    void GetAddressMask(const Buffer &payload, IPAddress &address_mask);
    void SetAddressMask(Buffer &payload, const IPAddress &address_mask);
    
    // helper functions
    unsigned short ComputeChecksum(const Packet &p) const;
    bool IsCorrectChecksum(const Packet &p) const;
    void RecomputeChecksum(const Packet &p);
    void GetCurrentTimeInMilliseconds(unsigned long &current);
    
    // debug functions
    std::ostream & Print(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, const ICMPHeader& L) {
	return L.Print(os);
    }
};



class ICMPPacket : public Packet {
 public:
    ICMPPacket();
    ICMPPacket(const Packet &rhs);
    ICMPPacket(const RawEthernetPacket &rhs);
    
    ICMPPacket(const Packet &original,
	       const IPAddress &source,
	       const IPAddress &destination,
	       const unsigned char &type,
	       const unsigned char &code,
	       const unsigned short &identifier,
	       const unsigned short &seqnumber,
	       Buffer &payload);
    
    // request / reply constructors
    ICMPPacket(const IPAddress &destination,
	       const unsigned char &type, const unsigned char &code,
	       const unsigned short &identifier, const unsigned short &seqnumber);
    ICMPPacket(const IPAddress &destination,
	       const unsigned char &type, const unsigned char &code);
    ICMPPacket(const IPAddress &destination,
	       const unsigned char &type);
    
    // error constructors
    ICMPPacket(const IPAddress &destination,
	       const unsigned char &type, const unsigned char &code,
	       const unsigned short &identifier, const unsigned short &seqnumber,
	       const Packet &p);
    ICMPPacket(const IPAddress &destination,
	       const unsigned char &type, const unsigned char &code,
	       const Packet &p);

    // accessor functions
    void ExtractIphandIcmphEightBytes(const Packet &p, Buffer &data);
    void ExtractIphandEightBytes(const Packet &p, Buffer &data);
    void SetIphandEightBytes(Buffer &payload, Buffer &data);
    
    // helper functions
    void respond(const Packet &p);
    void respond(const RawEthernetPacket &rp);
    void respond_in_ip_module(const Packet &p);
    bool requires_reply();
    void handle_response(IPHeader &iph, ICMPHeader &icmph,
			 Buffer &payload, unsigned char &icmp_type);
    void handle_error(IPHeader &iph, ICMPHeader &icmph,
		      Buffer &payload, unsigned char &icmp_type);
    
 private:
    bool requires_response;
};

#endif


