#include <unistd.h>
#include <cstdio>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <sys/time.h>
#include <sys/types.h>

#include "ethernet.h"
#include "config.h"
#include "error.h"
#include "util.h"

#include <libnet.h>

#include "debug.h"


EthernetAddr MyEthernetAddr() {
    char * eth_str = getenv("MINET_ETHERNETADDR");

    if (eth_str == NULL) {
	Die("MINET_ETHERNETADDR is not set!");
    }

    return EthernetAddr(eth_str);
}


int ethernet_reader_pair[2];
int ethernet_reader_fd;
int ethernet_reader_pid;

int ethernet_writer_pair[2];
int ethernet_writer_fd;
int ethernet_writer_pid;

static EthernetConfig ethernet_config = {0, 0, NULL} ;

static int ethernet_inited = 0;

static RawEthernetPacket EthernetIncomingPacket;
static int  EthernetIncomingPacketsAvailable;
static RawEthernetPacket EthernetOutgoingPacket;
static int  EthernetOutgoingPacketsAvailable;

// MIGHT REGRET: commenting this out (since it is already defined in util.h)
/*
int MAX(int x, int y)
{
  return x > y ? x : y;
}
*/

void KillHandler(int sig)
{
    EthernetShutdown(&ethernet_config);
}

void EthernetInputHandler(int sig)
{
    static int n = 0;
    fd_set read_fds;
    struct timeval to;
    int rc;
    

    while (1) {
	/* either reader, writer, or both have something for us */
	FD_ZERO(&read_fds);
	FD_SET(ethernet_reader_fd, &read_fds);
	FD_SET(ethernet_writer_fd, &read_fds);
	
	to.tv_sec  = 0;
	to.tv_usec = 0;
	
	do {
	    rc = select(MAX(ethernet_reader_fd, ethernet_writer_fd) + 1,
			&read_fds, 0, 0, &to);
	} while ((rc < 0) && (errno == EINTR));

	if (rc < 0) {
	    PERROR();

	    exit(-1);
	} else if (rc == 0) {
	    /* nothing to do, time out */

	    return;
	} else if (rc > 0) {

	    if (FD_ISSET(ethernet_reader_fd, &read_fds)) {

		EthernetIncomingPacket.Unserialize(ethernet_reader_fd);
		EthernetIncomingPacketsAvailable = 1;

		ethernet_config.ISR(ethernet_config.device,
				    ETHERNET_SERVICE_PACKET_READY);

		DEBUGPRINTF(5,"read packet %d\n",n++);
	    }

#if ETHERNET_WRITER_ON
	    if (FD_ISSET(ethernet_writer_fd, &read_fds)) {
		int flag;

		if (readall(ethernet_writer_fd, (char *)&flag, 
			    sizeof(flag), 0, 1) != sizeof(flag)) {
		    PERROR();
		    exit(-1);
		}

		ethernet_config.ISR(ethernet_config.device, flag);

		DEBUGPRINTF(5, "write response - %s\n",
			    (flag == ETHERNET_SERVICE_OUTPUT_BUFFER_FULL) ? "OUTPUT_BUFFER_FULL" :
			    (flag == ETHERNET_SERVICE_DMA_DONE) ? "DMA_DONE" :
			    (flag == ETHERNET_SERVICE_DMA_FAIL) ? "DMA_FAIL" :
			    "unknown");
	    }
#endif
	}
    }
}


int EthernetStartInternal() {
    /*
      Note the use of socketpair here is because linux does not
      support async i/o on pipes.  Real OSes do.
    */
    
    struct sigaction sa;

    memset(&sa, 0 ,sizeof(sa));

    sa.sa_handler = EthernetInputHandler;

    sigaction(ETHERNET_SIGNAL, &sa, 0);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, ethernet_reader_pair)) {
	PERROR();
	return -1;
    }

    ethernet_reader_fd = ethernet_reader_pair[0];

    ethernet_reader_pid = fork();

    if (ethernet_reader_pid < 0) {
	/* fork failed */

	PERROR();

	close(ethernet_reader_pair[0]);
	close(ethernet_reader_pair[1]);
	return -1;

    } else if (ethernet_reader_pid==0) {
	/* I am child */

	//AttachDebuggerHere("/home/pdinda/netclass/stack++/ISR");
	//BreakHere();

	close(ethernet_reader_pair[0]);

	if (dup2(ethernet_reader_pair[1], fileno(stdout)) < 0) {
	    PERROR();
	    exit(-1);
	}

	if (dup2(ethernet_reader_pair[1], fileno(stdin)) < 0) {
	    PERROR();
	    exit(-1);
	}

	DEBUGPRINTF(2, "Starting reader - exec '%s %s %s'\n",
		    READER_ARGS[0], READER_ARGS[1], READER_ARGS[2]);

	execv(READER_BINARY, READER_ARGS);
	PERROR();

	exit(-1);

    } else {
	/* I am parent */

	close(ethernet_writer_pair[1]);

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, ethernet_writer_pair)) {
	    PERROR();

	    kill(ethernet_reader_pid, SIGKILL);
	    close(ethernet_reader_fd);

	    return -1;
	}

	ethernet_writer_fd = ethernet_writer_pair[0];

	ethernet_writer_pid = fork();

	if (ethernet_writer_pid < 0) {
	    /* fork failed */

	    PERROR();

	    kill(ethernet_reader_pid, SIGKILL);

	    close(ethernet_reader_fd);
	    close(ethernet_writer_pair[0]);
	    close(ethernet_writer_pair[1]);

	    return -1;

	} else if (ethernet_writer_pid == 0) {
	    /* I am child */

	    //AttachDebuggerHere("/home/pdinda/netclass/stack++/ISR");
	    //BreakHere();

	    close(ethernet_writer_pair[0]);

	    if (dup2(ethernet_writer_pair[1], fileno(stdout)) < 0) {
		PERROR();
		exit(-1);
	    }

	    if (dup2(ethernet_writer_pair[1], fileno(stdin)) < 0) {
		PERROR();
		exit(-1);
	    }

	    DEBUGPRINTF(2, "Starting writer - exec '%s %s %s'\n",
			WRITER_ARGS[0], WRITER_ARGS[1], WRITER_ARGS[2]);
	    
	    execv(WRITER_BINARY, WRITER_ARGS);

	    PERROR();
	    exit(-1);
	} else {
	    /* I am parent */
	    close(ethernet_writer_pair[1]);
	}
	
	return 0;
    }
}


int EthernetShutdownInternal()
{
    signal(ETHERNET_SIGNAL, SIG_DFL);

    kill(ethernet_reader_pid, SIGKILL);
    kill(ethernet_writer_pid, SIGKILL);

    close(ethernet_reader_fd);
    close(ethernet_writer_fd);

    return 0;
}


int EthernetStartup(EthernetConfig * conf)
{
    int rc;

    assert(ethernet_inited == 0);
    assert(conf != NULL);
    assert(conf->device == 0);
    assert(conf->flags == 0);

    ethernet_config = *conf;

    rc = EthernetStartInternal();

    if (rc < 0) {
	return rc;
    }

    EthernetIncomingPacketsAvailable = 0;
    EthernetOutgoingPacketsAvailable = 0;

    ethernet_inited = 1;

    return rc;

}

int EthernetShutdown(EthernetConfig * conf)
{
    assert(ethernet_inited == 1); 

    assert(conf != NULL);
    assert(conf->device == 0);
    assert(conf->flags == 0);

    EthernetIncomingPacketsAvailable = 0;
    EthernetOutgoingPacketsAvailable = 0;

    return EthernetShutdownInternal();
}


int EthernetGetNextPacket(EthernetConfig * conf, RawEthernetPacket * p)
{
    assert(ethernet_inited == 1);

    assert(conf != NULL);
    assert(conf->device == 0);
    assert(conf->flags == 0);

    if (EthernetIncomingPacketsAvailable <= 0) {
	return -1;
    }

    *p = EthernetIncomingPacket;

    EthernetIncomingPacketsAvailable--;

    return p->size;
}


int EthernetWriteNextPacket()
{
    int rc = 0;

    assert(EthernetOutgoingPacketsAvailable > 0);

    EthernetOutgoingPacket.Serialize(ethernet_writer_fd);

    EthernetOutgoingPacketsAvailable--;

    DEBUGPRINTF(5, "Finished ethernetwritenextpacket\n");
    
    return rc;
}


int EthernetInitiateSend(EthernetConfig *conf, RawEthernetPacket *p)
{
    assert(ethernet_inited == 1);

    assert(conf != NULL);
    assert(conf->device == 0);
    assert(conf->flags == 0);

    assert(EthernetOutgoingPacketsAvailable == 0);

    EthernetOutgoingPacket = *p;
    EthernetOutgoingPacketsAvailable = 1;

    return EthernetWriteNextPacket();
}



EthernetAddr::EthernetAddr()
{
    bzero(addr, 6);
}


EthernetAddr::EthernetAddr(const EthernetAddr &rhs)
{
    memcpy(addr, rhs.addr, 6);
}

EthernetAddr::EthernetAddr(const EthernetAddrString rhs)
{
    SetToString(rhs);
}


const EthernetAddr & EthernetAddr::operator=(const EthernetAddr &rhs)
{
    memcpy(addr, rhs.addr, 6);
    return *this;
}

bool EthernetAddr::operator==(const EthernetAddr &rhs) const
{
    return (memcmp(addr, rhs.addr, 6) == 0);
}

bool EthernetAddr::operator!=(const EthernetAddr &rhs) const
{
    return !((*this) == rhs);
}

void EthernetAddr::SetToString(const EthernetAddrString s)
{
    int i = 0;
    int j = 0;

    for (i = 0, j = 0; i < 6; i++, j += 3) {
	hexbytetobyte(&(s[j]), &(addr[i]));
    }

}

void EthernetAddr::GetAsString(EthernetAddrString s) const
{
    int i = 0;
    int j = 0;

    for (i = 0, j = 0; i < 6; i++, j += 3) {
	bytetohexbyte(addr[i], &(s[j]));

	if (i < 5) {
	    s[j + 2] = ':';
	} else {
	    s[j + 2] = 0;
	}
    }
}


std::ostream & EthernetAddr::Print(std::ostream &os) const
{
    EthernetAddrString s;

    GetAsString(s);

    os << "EthernetAddr(" << (char *)s << ")";

    return os;
}

void EthernetAddr::Serialize(const int fd) const
{
    if (writeall(fd, addr, 6) != 6) {
	throw SerializationException();
    }
}

void EthernetAddr::Unserialize(const int fd)
{
    if (readall(fd, addr, 6) != 6) {
	throw SerializationException();
    }
}


EthernetHeader::EthernetHeader() : Header(Headers::EthernetHeader)
{}

EthernetHeader::EthernetHeader(const Header &rhs) : Header(rhs)
{}

EthernetHeader::EthernetHeader(const Buffer &rhs) : Header(Headers::EthernetHeader,rhs)
{}


const EthernetHeader & EthernetHeader::operator=(const Header &rhs)
{
    Header::operator=(rhs);
    return *this;
}


void EthernetHeader::GetSrcAddr(EthernetAddr &addr) const
{
    GetData((char *)(addr.addr), 6, 6);
}

void EthernetHeader::SetSrcAddr(const EthernetAddr &addr)
{
    SetData((const char *)(addr.addr), 6, 6);
}

void EthernetHeader::GetDestAddr(EthernetAddr &addr) const
{
    GetData((char *)(addr.addr), 6, 0);
}

void EthernetHeader::SetDestAddr(const EthernetAddr &addr)
{
    SetData((const char *)(addr.addr), 6, 0);
}

void EthernetHeader::GetProtocolType(EthernetProtocol &protocoltype) const
{
    GetData((char *)&protocoltype, 2, 12);
    protocoltype = ntohs(protocoltype);
}

void EthernetHeader::SetProtocolType(const EthernetProtocol &protocoltype)
{
    short temp = htons(protocoltype);
    SetData((const char *)&temp, 2, 12);
}

std::ostream & EthernetHeader::Print(std::ostream &os) const
{
    EthernetAddr addr;
    EthernetProtocol proto;

    os << "EthernetHeader(";

    GetSrcAddr(addr);
    os << " src=" << addr;
    
    GetDestAddr(addr);    
    GetProtocolType(proto);

    os << ", dst=" << addr
       << ", proto=" << proto
       << ")";

    return os;
}



EthernetTrailer::EthernetTrailer() : 
    Trailer(Trailers::EthernetTrailer)
{}

EthernetTrailer::EthernetTrailer(const Trailer &rhs) : 
    Trailer(rhs)
{}

EthernetTrailer::EthernetTrailer(const Buffer &rhs) : 
    Trailer(Trailers::EthernetTrailer, rhs)
{}

const EthernetTrailer & EthernetTrailer::operator=(const Trailer &rhs)
{
    Trailer::operator=(rhs);
    return *this;
}


void EthernetTrailer::GetCRC(EthernetCRC &crc) const
{
    GetData((char *)&crc, 4, 0);
    crc = ntohl(crc);
}

void EthernetTrailer::SetCRC(const EthernetCRC &crc)
{
    EthernetCRC crc2 = htonl(crc);

    SetData((const char *)&crc2, 4, 0);
}


std::ostream & EthernetTrailer::Print(std::ostream &os) const
{
    EthernetCRC crc;

    GetCRC(crc);
    os << "EthernetTrailer(CRC=" << *((unsigned *)crc) << ")";

    return os;
}
