#ifndef _ethernet
#define _ethernet

#include "config.h"
#include "raw_ethernet_packet.h"
#include "headertrailer.h"

#define ETHERNET_STATUS_OK   0
#define ETHERNET_STATUS_ERR -1

#define ETHERNET_SERVICE_PACKET_READY 0
#define ETHERNET_SERVICE_DMA_DONE     1
#define ETHERNET_SERVICE_DMA_FAIL     2
#define ETHERNET_SERVICE_OUTPUT_BUFFER_FULL 4

#define ETHERNET_HEADER_LEN 14

typedef struct {
    int device;
    int flags;
    int (*ISR)(int device, int service);
} EthernetConfig;

int EthernetStartup(EthernetConfig * conf);
int EthernetInitiateSend(EthernetConfig * conf, RawEthernetPacket * p);
int EthernetGetNextPacket(EthernetConfig * conf, RawEthernetPacket * p);
int EthernetShutdown(EthernetConfig * conf);


typedef char EthernetAddrString[2 * 6 + 6];
typedef unsigned EthernetCRC;
typedef short EthernetProtocol;

struct EthernetAddr {
    char addr[6];

    EthernetAddr();
    EthernetAddr(const EthernetAddr &rhs);
    EthernetAddr(const EthernetAddrString rhs);
    const EthernetAddr & operator=(const EthernetAddr &rhs);

    bool operator==(const EthernetAddr &rhs) const;
    bool operator!=(const EthernetAddr &rhs) const;

    void SetToString(const EthernetAddrString s);
    void GetAsString(EthernetAddrString s) const;

    void Serialize(const int fd) const;
    void Unserialize(const int fd);

    std::ostream & Print(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, const EthernetAddr& L) {
	return L.Print(os);
    }
};


class EthernetHeader : public Header {
 public:
    EthernetHeader();
    EthernetHeader(const Header &rhs);
    EthernetHeader(const Buffer &rhs);

    const EthernetHeader & operator=(const Header &rhs);
    
    void GetSrcAddr(EthernetAddr &addr) const;
    void SetSrcAddr(const EthernetAddr &addr);
    
    void GetDestAddr(EthernetAddr &addr) const;
    void SetDestAddr(const EthernetAddr &addr);
    
    void GetProtocolType(EthernetProtocol &protocoltype) const;
    void SetProtocolType(const EthernetProtocol &protocoltype);
    
    std::ostream & Print(std::ostream &os) const;
    
    friend std::ostream &operator<<(std::ostream &os, const EthernetHeader& L) {
	return L.Print(os);
    }
};

class EthernetTrailer : public Trailer {
 public:
    EthernetTrailer();
    EthernetTrailer(const Trailer &rhs);
    EthernetTrailer(const Buffer &rhs);

    const EthernetTrailer & operator=(const Trailer &rhs);

    void GetCRC(EthernetCRC &crc) const;
    void SetCRC(const EthernetCRC &crc);

    std::ostream & Print(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, const EthernetTrailer& L) {
	return L.Print(os);
    }
};


const EthernetAddr ETHERNET_BROADCAST_ADDR("ff:ff:ff:ff:ff:ff");
const EthernetAddr ETHERNET_BLANK_ADDR("00:00:00:00:00:00");

const short PROTO_ARP = 0x0806;
const short PROTO_IP = 0x0800;


extern EthernetAddr MyEthernetAddr();

#endif
