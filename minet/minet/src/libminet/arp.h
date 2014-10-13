#ifndef _arp
#define _arp

#include <functional>
#include <unordered_map>
// Note: Looks like "hash_map" is deprecated and we're supposed to use "unordered_map" now.
// However, unordered_map is not yet part of standard C++, so that's not ideal, either.-------
//
// TODO: Maybe consider using the Boost library for a hash map implementation?
// (How about waiting to see if the Boost library can be helpful in other parts of the project first.
// Until then, it might not be worth complicating the project by having additional dependencies.)
//
// -------------------------------------------------------------------------------------------
//
// I did some research on how to obtain a usable hash map implementation in C++.  Here is a
// summary of my findings:
//
// The C++ Standard Library does not have a hash map implementation yet, but the upcoming
// C++0x standard will define an unordered_map class.  At the time of this writing [June 2010],
// C++0x is expected to be published by the end of 2011.
//
// For now, there are a couple options for obtaining a usable hash map implementation.  We
// can:
//
// Use the older SGI STL extensions, or
// Use the unofficial implementations of unordered_map, or
// use the Boost library (or something similar).
//
//
// Option 1: Use the older SGI STL extensions
// --------------------------------------
//
// In newer GCC versions:
// #include <ext/hash_map> and use __gnu_cxx::hash_map
//
// In older GCC versions:
// #include <hash_map> and use hash_map.
//
// In MSVC:
// #include <hash_map> and use stdext::hash_map.
//
// Note that there may be minor differences between these.  For this reason, this is not the
// recommended approach for getting a usable hash map implementation.
//
//
// Option 2: Use the unofficial implementations of unordered_map
// -------------------------------------------------------------
//
// #include <unordered_map>, then use std::unordered_map.
//
// In GCC, you must enable one of the following compiler options:
// -std=c++0x or -std=gnu++0x
//
// Haven't tested in MSVC! Based on this:
// http://msdn.microsoft.com/en-us/library/bb982522.aspx
// it should just work, but I haven't tried it.  Note that you might have to use
// std::tr1::unordered_map instead.
//
// The C++ Technical Report 1 (TR1), published in 2005, proposes the unordered_map header and
// data type.  The newer versions of some compilers (including GCC and MSVC) ship with
// implementations of these proposed types, so it is possible to use unordered_map already.
// (Technically, since TR1 has not yet been integrated into C++0x, doing so means you are
// using non-standard code, but in theory, no changes should be necessary once it is standardized.)
// However, compilers need not include TR1 to be conforming, so these implementations may be
// missing in some toolchains.
//
// When compiling code that includes <ext/hash_map> (as per Option 1 above) in GCC,
// a message generated from backward_warning.h recommends switching to <unordered_map>.
//
//
// Option 3: Use the Boost library
// -------------------------------
//
// Download boost, #include <boost/unordered_map.hpp>, and use boost::unordered_map.
//
// The advantage is that we will not have to use the dated SGI STL extensions (which isn't
// recommended), nor the non-standard TR1 implementations that some compilers may or may not ship
// with.  Furthermore, the Boost library provides a large number of other useful data types.
//
// The disadvantage is that having an additional dependency can complicate Minet unnecessarily,
// especially if we use it for just a single data type.
//

#include "raw_ethernet_packet.h"
#include "ethernet.h"
#include "packet.h"
#include "ip.h"

typedef unsigned short ARPHWAddressSpaceType;
typedef unsigned short ARPProtoAddressSpaceType;
typedef unsigned char  ARPHWAddressLenType;
typedef unsigned char  ARPProtoAddressLenType;
typedef unsigned short ARPOpcodeType;


const short ARP_REQUEST_OPCODE   = 0x0001;
const short ARP_REPLY_OPCODE     = 0x0002;

const short ETHERNET_HW_ADXSPACE = 0x0001;
const short IP_PROTO_ADXSPACE    = 0x0800;


//
// Note that this supports only IP->Ethernet address translation
//

class ARPPacket : public Packet {
 public:
    enum OpcodeType {Request, Reply};

    ARPPacket();
    ARPPacket(const ARPPacket &rhs);
    ARPPacket(const RawEthernetPacket &rhs);
    ARPPacket(const Packet &rhs);
    ARPPacket(const OpcodeType     opcode,
	      const EthernetAddr   &sender_hw_addr,
	      const IPAddress      &sender_ip_addr,
	      const EthernetAddr   &target_hw_addr,
	      const IPAddress      &target_ip_addr);
    const ARPPacket & operator=(const ARPPacket &rhs);
    virtual ~ARPPacket();
    
    bool IsIPToEthernet() const;
    bool IsIPToEthernetRequest() const;
    bool IsIPToEthernetReply() const;
    
    void GetHWAddressSpace(ARPHWAddressSpaceType &haddr) const;
    void SetHWAddressSpace(const ARPHWAddressSpaceType &haddr);
    
    void GetProtocolAddressSpace(ARPProtoAddressSpaceType &paddr) const;
    void SetProtocolAddressSpace(const ARPProtoAddressSpaceType &paddr);
    
    void GetHWAddressLen(ARPHWAddressLenType &hlen) const;
    void SetHWAddressLen(const ARPHWAddressLenType &hlen);
    
    void GetProtoAddressLen(ARPProtoAddressLenType &hlen) const;
    void SetProtoAddressLen(const ARPProtoAddressLenType &hlen);
    
    void GetOpcode(OpcodeType &opcode) const;
    void SetOpcode(const OpcodeType &opcode);
    
    void SetSenderEthernetAddr(const EthernetAddr &sender_hw_addr);
    void GetSenderEthernetAddr(EthernetAddr &sender_hw_addr) const;
    
    void SetSenderIPAddr(const IPAddress &sender_ip_addr);
    void GetSenderIPAddr(IPAddress &sender_ip_addr) const;
    
    void SetTargetEthernetAddr(const EthernetAddr &target_hw_addr);
    void GetTargetEthernetAddr(EthernetAddr &target_hw_addr) const;
    
    void SetTargetIPAddr(const IPAddress &target_ip_addr);
    void GetTargetIPAddr(IPAddress &target_ip_addr) const;
    
    std::ostream & Print(std::ostream &os) const;
    
    friend std::ostream &operator<<(std::ostream &os, const ARPPacket& pkt) {
	return pkt.Print(os);
    }
    
};

// Note: local

struct ARPRequestResponse {
    IPAddress    ipaddr;
    EthernetAddr ethernetaddr;

    enum Flag {REQUEST = 1, RESPONSE_OK = 2, RESPONSE_UNKNOWN = 4} flag;
    
    ARPRequestResponse();
    ARPRequestResponse(const ARPRequestResponse &rhs);
    ARPRequestResponse(const IPAddress     &ip,
		       const EthernetAddr  &hw,
		       const Flag          flag);

    virtual ~ARPRequestResponse();

    ARPRequestResponse & operator=(const ARPRequestResponse &rhs);
    
    void Serialize(const int fd) const;
    void Unserialize(const int fd);
    
    std::ostream & Print(std::ostream &os) const;
    
    friend std::ostream &operator<<(std::ostream &os, const ARPRequestResponse& msg) {
	return msg.Print(os);
    }
};




struct eqipaddress {
    bool operator()(const IPAddress lhs, const IPAddress rhs) const { 
	return (lhs == rhs);
    }
};

struct hashipaddress : public std::hash<unsigned> {
    std::hash<unsigned> H;

    size_t operator()(const IPAddress &x) const { 
	return H((unsigned)x); 
    }
};

std::ostream & operator<< (std::ostream &os, const std::pair<IPAddress, ARPRequestResponse> &pair);


class ARPCache {
 private:

    //typedef hash_map<IPAddress,ARPRequestResponse,hashipaddress,eqipaddress> DataType;
    typedef std::unordered_map<IPAddress, ARPRequestResponse, hashipaddress, eqipaddress> DataType;

    DataType data;

 public:
    void Update(const ARPRequestResponse &x);
    void Delete(const IPAddress &a);
    void Lookup(ARPRequestResponse &x) const;
    
    std::ostream & Print(std::ostream &os) const;
    
    friend std::ostream &operator<<(std::ostream &os, const ARPCache& cache) {
	return cache.Print(os);
    }
};



#endif
