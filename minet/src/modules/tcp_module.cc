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

int main(int argc, char *argv[])
{


    bool isClient=false;
    // This was all included in the code
    MinetHandle mux;    // Mutex to ensure not preempted
    MinetHandle sock;    // Socket

    ConnectionList <TCPState> clist;    // Maintains a list of all current connections

    MinetInit(MINET_TCP_MODULE); // Initialize the minet tcp module

    mux = MinetIsModuleInConfig(MINET_IP_MUX) ?
            MinetConnect(MINET_IP_MUX) :
            MINET_NOHANDLE;
    sock = MinetIsModuleInConfig(MINET_SOCK_MODULE) ?
            MinetAccept(MINET_SOCK_MODULE) :
            MINET_NOHANDLE;

    if ((mux == MINET_NOHANDLE) && (MinetIsModuleInConfig(MINET_IP_MUX)))
    {
        MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ip_mux"));
        return -1;
    }
    if ((sock == MINET_NOHANDLE) && (MinetIsModuleInConfig(MINET_SOCK_MODULE)))
    {
        MinetSendToMonitor(MinetMonitoringEvent("Can't accept from sock_module"));
        return -1;
    }
    cerr << "tcp_module STUB VERSION handling tcp traffic.......\n"<< endl;
    MinetSendToMonitor(MinetMonitoringEvent("tcp_module STUB VERSION handling tcp traffic........"));
    MinetEvent event;
    double timeout = 1;
    ConnectionToStateMapping<TCPState> c_mapping;
    
    Packet envelope; // Declare the packet
    IPHeader new_ipheader;	// Holds the IP Header
    TCPHeader new_tcpheader;	// Holds the TCP Header
    
    cerr<< "---------------Building a packet to send off------------" << endl;

    unsigned char alerts = 0; // Alerts are flags for the packet being sent
    
    // Send a SYN packet in client mode
    if(isClient)
    {
        new_ipheader.SetSourceIP("192.168.128.1"); // Set the source IP --- my IP Address
        new_ipheader.SetDestIP("192.168.42.5");	// Set the destination IP --- NETLAB-3

        new_ipheader.SetProtocol(IP_PROTO_TCP);	// Set protocol to TCP

        new_ipheader.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH); // Total length of the packet being sent off
        envelope.PushFrontHeader(new_ipheader);	// Add the IPHeader into the packet

        new_tcpheader.SetSourcePort(2020, envelope);
        new_tcpheader.SetDestPort(2020, envelope);

        new_tcpheader.SetSeqNum(1, envelope);

        new_tcpheader.SetWinSize(100, envelope);
        new_tcpheader.SetUrgentPtr(0, envelope);

        SET_SYN(alerts); // Set the flag that this is a SYN packet
        new_tcpheader.SetFlags(alerts, envelope); // Set the flag in the header

        new_tcpheader.RecomputeChecksum(envelope);

        new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, envelope);
        envelope.PushBackHeader(new_tcpheader);	// Push the header into the packet

        MinetSend(mux, envelope); // Send the packet to mux
        sleep(1);
        MinetSend(mux, envelope);
        cerr<< "---------------SYN HAS BEEN SENT------------" << endl;

        SockRequestResponse req;	// Hold the request
        SockRequestResponse reply;	// Hold the response
    }
    else
    {
        printf("Running Server Code\n");
    }



    while (MinetGetNextEvent(event, timeout) == 0)
    {
        if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
        {
            if (event.handle == sock)
            {
                printf("Well... I am in the Sock... Is it good or bad?\n");
            }
            if (event.handle == mux)
            {
                Packet mux_packet; // Receipt packet
                MinetReceive(mux, mux_packet); // Receive packet
                
                unsigned short length = TCPHeader::EstimateTCPHeaderLength(mux_packet);	// Estimate length
                mux_packet.ExtractHeaderFromPayload<TCPHeader>(length);	// Get the Header from the packet

                TCPHeader tcp_header; // For storing the TCP header
                tcp_header = mux_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet

                IPHeader ip_header;	// For holding the IP header
                ip_header = mux_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet

                unsigned char f;	// To hold the flags from the packet
                unsigned char cap_flags = 0;

                tcp_header.GetFlags(f); // Assign f with flags received from TCP Header

                unsigned int ack_num = 0;
                unsigned int seq_num = 0;

                tcp_header.GetSeqNum(seq_num);

                Packet to_send;	// Declare the response packet
                IPHeader iph;	// Holds the IP Header

                IPAddress temp;		// hold the IP Address for switching around
                ip_header.GetSourceIP(temp); // Should give us the source
                iph.SetDestIP(temp);	// Set the destination IP --- NETLAB-3
                ip_header.GetDestIP(temp); // Should give us the source
                iph.SetSourceIP(temp);	// Set the source IP --- my IP Address

                iph.SetProtocol(IP_PROTO_TCP);	// Set protocol to TCP

                iph.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH);
                to_send.PushFrontHeader(iph);	// Add the IPHeader into the packet

                TCPHeader new_tcphead;	// Holds the TCP Header

                if(IS_SYN(f) && !IS_ACK(f)) // ___SYN___
                {
                    printf("~~~~~~~~~~~~Got a SYN~~~~~~~~~~~~~~~~~~~\n");
                    SET_SYN(cap_flags);
                    SET_ACK(cap_flags);

                    new_tcphead.SetSeqNum(1, to_send);
                    new_tcphead.SetAckNum(seq_num+1, to_send);

                    cerr<<"SYN"<<tcp_header<<endl;
                    printf("~~~~~~~~~~~Done with SYN~~~~~~~~~~~~~~~~~~~~\n");


                }
                else if(IS_ACK(f) && !IS_SYN(f)) // ___ACK___
                {
                    printf("Three way handshake is complete. Nice to meet you.\n");
                    for(;;);
                }
                else if(IS_SYN(f) && IS_ACK(f)) // ___SYN-ACK___
                {
                    printf("~~~~~~~~~~~SYNACK Actions~~~~~~~~~~~~~~~~~~~~\n");

                    seq_num= seq_num + 1;

                    new_tcphead.SetSeqNum(ack_num, to_send);
                    new_tcphead.SetAckNum(seq_num, to_send);

                    SET_ACK(cap_flags);

                    cerr<<"\nSYN ACK\n"<<tcp_header<<endl;

                    printf("~~~~~~~ Finished SYNACK Actions~~~~~~~~~~~~~~~~~~~~~~~~\n");
                }

                unsigned short theRealPort = 0;

                if(IS_SYN(f) && !IS_ACK(f))
                {
                    tcp_header.GetSourcePort(theRealPort);
                    new_tcphead.SetDestPort(theRealPort, to_send);
                    new_tcphead.SetSourcePort(2121, to_send);


                }
                else
                {
                    new_tcphead.SetSourcePort(2121, to_send);
                    new_tcphead.SetDestPort(2121, to_send);
                }

                new_tcphead.SetWinSize(100, to_send);
                new_tcphead.SetUrgentPtr(0, to_send);

                new_tcphead.SetFlags(cap_flags, to_send); // Set the flag in the header

                new_tcphead.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_send);

                new_tcphead.RecomputeChecksum(to_send);

                to_send.PushBackHeader(new_tcphead); // Push the header into the packet

                MinetSend(mux, to_send);

            }
            if (event.eventtype == MinetEvent::Timeout)
            {
                // timeout ! probably need to resend some packets
            }
        }
    }
    MinetDeinit();	// Deinitialize the minet stack
    return 0;	// Program finished
}
