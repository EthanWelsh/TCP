// You will build this in project part B - this is merely a
// stub that does nothing but integrate into the stack

// For project parts A and B, an appropriate binary will be
// copied over as part of the build process



#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/**************/

#include "../libminet/tcpstate.h"


void build_packet(Packet &, ConnectionToStateMapping<TCPState> &, int , int , bool);


#include <iostream>

//#include "Minet.h"

using namespace std;


static const int HEADERTYPE_SYN = 1;
static const int HEADERTYPE_ACK = 2;
static const int HEADERTYPE_SYNACK = 3;
static const int HEADERTYPE_PSHACK = 4;
static const int HEADERTYPE_FIN = 5;
static const int HEADERTYPE_FINACK = 6;
static const int HEADERTYPE_RST = 7;


void build_packet(Packet &to_build, ConnectionToStateMapping<TCPState> &c_mapping, int HeaderType, int data_amount, bool timed_out)
{
cerr<< "---------------Building a packet to send off------------" << endl;
unsigned char alerts = 0;
int packet_size = data_amount + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH;
IPHeader new_ipheader;
TCPHeader new_tcpheader;
new_ipheader.SetSourceIP(c_mapping.connection.src);
new_ipheader.SetDestIP(c_mapping.connection.dest);
new_ipheader.SetTotalLength(packet_size);
new_ipheader.SetProtocol(IP_PROTO_TCP);
to_build.PushFrontHeader(new_ipheader);
cerr << "\n new_ipheader: \n" << new_ipheader << endl;

new_tcpheader.SetSourcePort(c_mapping.connection.srcport, to_build);
new_tcpheader.SetDestPort(c_mapping.connection.destport, to_build);
new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_build);

new_tcpheader.SetAckNum(c_mapping.state.GetLastRecvd(),to_build);
new_tcpheader.SetWinSize(c_mapping.state.GetRwnd(), to_build);
new_tcpheader.SetUrgentPtr(0, to_build);

// Determine the flag type
switch (HeaderType)
{
case HEADERTYPE_SYN:
        SET_SYN(alerts);
cerr << "It is a HEADERTYPE_SYN!" << endl;
break;

case HEADERTYPE_ACK:
        SET_ACK(alerts);
cerr << "It is a HEADERTYPE_ACK!" << endl;
break;

case HEADERTYPE_SYNACK:
        SET_ACK(alerts);
SET_SYN(alerts);
cerr << "It is a HEADERTYPE_SYNACK!" << endl;
break;

case HEADERTYPE_PSHACK:
        SET_PSH(alerts);
SET_ACK(alerts);
cerr << "It is a HEADERTYPE_PSHACK!" << endl;
break;

case HEADERTYPE_FIN:
        SET_FIN(alerts);
cerr << "It is a HEADERTYPE_FIN!" << endl;
break;

case HEADERTYPE_FINACK:
        SET_FIN(alerts);
SET_ACK(alerts);
cerr << "It is a HEADERTYPE_FINACK!" << endl;
break;

case HEADERTYPE_RST:
        SET_RST(alerts);
cerr << "It is a HEADERTYPE_RST!" << endl;
break;

default:
break;
}

// Set the flag
new_tcpheader.SetFlags(alerts, to_build);

// Print out the finished TCP header for testing
cerr << "\new_tcpheader: \n" << new_tcpheader<< endl;

if (timed_out) // If dealing with a timeout
{
new_tcpheader.SetSeqNum(c_mapping.state.GetLastSent()+1, to_build);
}
else // Not responding to a timeout
{
new_tcpheader.SetSeqNum(c_mapping.state.GetLastSent(), to_build);
}

new_tcpheader.RecomputeChecksum(to_build);

// Push the header into the packet
to_build.PushBackHeader(new_tcpheader);
cerr<< "---------------Packet is built------------" << endl;
}










int main(int argc, char *argv[])
{
    MinetHandle mux;
    MinetHandle sock;

    ConnectionList <TCPState> clist;

    MinetInit(MINET_TCP_MODULE);

    mux = MinetIsModuleInConfig(MINET_IP_MUX) ?
            MinetConnect(MINET_IP_MUX) :
            MINET_NOHANDLE;

    sock = MinetIsModuleInConfig(MINET_SOCK_MODULE) ?
            MinetAccept(MINET_SOCK_MODULE) :
            MINET_NOHANDLE;

    if ((mux == MINET_NOHANDLE) &&
            (MinetIsModuleInConfig(MINET_IP_MUX)))
    {

        MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ip_mux"));

        return -1;
    }

    if ((sock == MINET_NOHANDLE) &&
            (MinetIsModuleInConfig(MINET_SOCK_MODULE)))
    {

        MinetSendToMonitor(MinetMonitoringEvent("Can't accept from sock_module"));

        return -1;
    }

    cerr << "tcp_module STUB VERSION handling tcp traffic.......\n";

    MinetSendToMonitor(MinetMonitoringEvent("tcp_module STUB VERSION handling tcp traffic........"));

    MinetEvent event;
    double timeout = 1;

    int i = 0;


    while (MinetGetNextEvent(event, timeout) == 0)
    {
        if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
        {
            if (event.handle == mux)
            {
                // ip packet has arrived!
                Packet p;
                MinetReceive(mux, p);
                Connection c;

                printf("An IP packet has arrived: %d\n", i);
                i++;
            }

            if (event.handle == sock)
            {
                SockRequestResponse req;
                MinetReceive(sock, req);

                switch (req.type)
                {

                    case CONNECT:
                    case ACCEPT:
                    { // ignored, send OK response
                        SockRequestResponse repl;
                        repl.type = STATUS;
                        repl.connection = req.connection;
                        // buffer is zero bytes
                        repl.bytes = 0;
                        repl.error = EOK;
                        MinetSend(sock, repl);
                    }
                        break;
                    case STATUS:
                        // ignored, no response needed
                        break;
                        // case SockRequestResponse::WRITE:
                    case WRITE:
                    {
                        unsigned bytes = MIN_MACRO(UDP_MAX_DATA, req.data.GetSize());
                        // create the payload of the packet
                        Packet p(req.data.ExtractFront(bytes));
                        // Make the IP header first since we need it to do the udp checksum
                        IPHeader ih;
                        ih.SetProtocol(IP_PROTO_UDP);
                        ih.SetSourceIP(req.connection.src);
                        ih.SetDestIP(req.connection.dest);
                        ih.SetTotalLength(bytes + UDP_HEADER_LENGTH + IP_HEADER_BASE_LENGTH);
                        // push it onto the packet
                        p.PushFrontHeader(ih);
                        // Now build the UDP header
                        // notice that we pass along the packet so that the udpheader can find
                        // the ip header because it will include some of its fields in the checksum
                        UDPHeader uh;
                        uh.SetSourcePort(req.connection.srcport, p);
                        uh.SetDestPort(req.connection.destport, p);
                        uh.SetLength(UDP_HEADER_LENGTH + bytes, p);
                        // Now we want to have the udp header BEHIND the IP header
                        p.PushBackHeader(uh);
                        MinetSend(mux, p);
                        SockRequestResponse repl;
                        // repl.type=SockRequestResponse::STATUS;
                        repl.type = STATUS;
                        repl.connection = req.connection;
                        repl.bytes = bytes;
                        repl.error = EOK;
                        MinetSend(sock, repl);
                    }
                        break;
                        // case SockRequestResponse::FORWARD:
                    case FORWARD:
                    {
                        ConnectionToStateMapping<TCPState> m;
                        m.connection = req.connection;
                        // remove any old forward that might be there.
                        ConnectionList<TCPState>::iterator cs = clist.FindMatching(req.connection);
                        if (cs != clist.end())
                        {
                            clist.erase(cs);
                        }
                        clist.push_back(m);
                        SockRequestResponse repl;
                        // repl.type=SockRequestResponse::STATUS;
                        repl.type = STATUS;
                        repl.connection = req.connection;
                        repl.error = EOK;
                        repl.bytes = 0;
                        MinetSend(sock, repl);
                    }
                        break;
                        // case SockRequestResponse::CLOSE:
                    case CLOSE:
                    {
                        ConnectionList<TCPState>::iterator cs = clist.FindMatching(req.connection);
                        SockRequestResponse repl;
                        repl.connection = req.connection;
                        // repl.type=SockRequestResponse::STATUS;
                        repl.type = STATUS;
                        if (cs == clist.end())
                        {
                            repl.error = ENOMATCH;
                        } else
                        {
                            repl.error = EOK;
                            clist.erase(cs);
                        }
                        MinetSend(sock, repl);
                    }
                        break;
                    default:
                    {
                        SockRequestResponse repl;
                        // repl.type=SockRequestResponse::STATUS;
                        repl.type = STATUS;
                        repl.error = EWHAT;
                        MinetSend(sock, repl);
                    }
                }
                /*
                 * if the state is currently accept then do the server stuff
                 * (go to listen)
                 *
                 * if the state is connect, you've got a client so create a packet (Packet p) and
                 * construct a TCPHeader which holds an IPHeader attach the TCP and IP header to
                 * the packet
                 *
                 * then minetSend(mux,p)
                 *
                 *
                 *
                 *
                 *
                 * in connect, store a connection variable with a connectionToStateMapping
                 *
                 * switch on this connection and then do something
                 *
                 *
                 */




                // socket request or response has arr
                //SockRequestResponse req;
                //MinetReceive(sock, req);
                printf("Socket request has arrived\n");
            }
        }

        if (event.eventtype == MinetEvent::Timeout)
        {
            // timeout ! probably need to resend some packets
            //  printf("~~~~TIMEOUT~~~~\n");
        }

    }

    MinetDeinit();

    return 0;
}