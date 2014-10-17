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
        new_tcpheader.SetSeqNum(1, to_build);
    }
    else // Not responding to a timeout
    {
        new_tcpheader.SetSeqNum(1, to_build); // TODO change this....
    }
    
    new_tcpheader.RecomputeChecksum(to_build);
    
    // Push the header into the packet
    to_build.PushBackHeader(new_tcpheader);
    cerr<< "---------------Packet is built------------" << endl;
}


int main(int argc, char *argv[])
{
	bool isClient=true;
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
	// End what was given for the moment




        Packet envelope; // Declare the packet
        IPHeader new_ipheader;	// Holds the IP Header
        TCPHeader new_tcpheader;	// Holds the TCP Header
		// Send a SYN packet in client mode
		cerr<< "---------------Building a packet to send off------------" << endl;
		
		unsigned char alerts = 0; 	// Alerts are flags for the packet being sent
		int packet_size = TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH;
		
		new_ipheader.SetSourceIP("192.168.128.1");	// Set the source IP --- my IP Address
		new_ipheader.SetDestIP("192.168.42.5");	// Set the destination IP --- NETLAB-3
		new_ipheader.SetTotalLength(packet_size);	 // Total length of the packet being sent off
		new_ipheader.SetProtocol(IP_PROTO_TCP);	// Set protocol to TCP
		envelope.PushFrontHeader(new_ipheader);	// Add the IPHeader into the packet
		cerr<< "---------------------------------" << endl;
		cerr << "\n new_ipheader: \n" << new_ipheader << endl;	// Print the header for testing and Part 1
		cerr<< "---------------------------------" << endl;
		new_tcpheader.SetSourcePort(5050, envelope);
		new_tcpheader.SetDestPort(5050, envelope);
		new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, envelope);
		
		new_tcpheader.SetAckNum(1, envelope);
        new_tcpheader.SetSeqNum(1, envelope);

		new_tcpheader.SetWinSize(100, envelope);
		new_tcpheader.SetUrgentPtr(0, envelope);
		
		SET_SYN(alerts); // Set the flag that this is a SYN packet
		new_tcpheader.SetFlags(alerts, envelope);	// Set the flag in the header
		
		// Print out the finished TCP header for testing
		cerr<< "---------------------------------" << endl;
		cerr << "\new_tcpheader: \n" << new_tcpheader<< endl;
		cerr<< "---------------------------------" << endl;
		
		new_tcpheader.RecomputeChecksum(envelope);
		
		envelope.PushBackHeader(new_tcpheader);		// Push the header into the packet
		cerr<< "---------------Packet is built------------" << endl;
		
		cerr<< "---------------Sending the Packet------------" << endl;
		MinetSend(mux, envelope); // Send the packet to mux
		sleep(1);
		MinetSend(mux, envelope);
		cerr<< "---------------SYN HAS BEEN SENT------------" << endl;

    SockRequestResponse req;	// Hold the request
    SockRequestResponse reply;	// Hold the response


    while (MinetGetNextEvent(event, timeout) == 0)
    {
        if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
        {
            if (event.handle == sock)
            {
                printf("Well... I am in the Sock... Is it good or bad?\n");
                // socket request or response has arrived

            }
            if (event.handle == mux)
            {
                //cerr<< "---------------Welcome to the mux portion------------" << endl;
                Packet mux_packet;	// Receipt packet
                MinetReceive(mux, mux_packet);	// Receive packet

				unsigned short length = TCPHeader::EstimateTCPHeaderLength(mux_packet);	// Estimate length
                mux_packet.ExtractHeaderFromPayload<TCPHeader>(length);	// Get the Header from the packet

                TCPHeader tcp_header;	// For storing the TCP header
                tcp_header = mux_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet.

                IPHeader ip_header;	// For holding the IP header
                ip_header = mux_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet

                unsigned char f;	// To hold the flags from the packet
                unsigned char cap_flags = 0;

                tcp_header.GetFlags(f); //	Assign f with flags received from TCP Header

                unsigned int ack_num = 0;
                unsigned int seq_num = 0;

                tcp_header.GetSeqNum(seq_num);

                if(IS_SYN(f) && !IS_ACK(f)) // If it's just a SYN packet
                {
                    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                    SET_SYN(cap_flags);
                    SET_ACK(cap_flags);

                    cerr<<"SYN"<<tcp_header<<endl;

                    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");



                }
                else if(IS_ACK(f) && !IS_SYN(f))
                {
                    //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

                    //cerr<<"\nACK\n"<<endl;

                    //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                }
                else if(IS_SYN(f) && IS_ACK(f)) // If it's a SYN-ACK
                {
                    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

                    tcp_header.GetAckNum(ack_num);



                    SET_ACK(cap_flags);

                    cerr<<"\nSYN ACK\n"<<tcp_header<<endl;
                    printf("\nWe have ACK #%d\n", ack_num);

                    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");


                }


				Packet to_send;

                unsigned int one = 1;

				TCPHeader new_tcphead;	// Holds the TCP Header
				to_send.PushFrontHeader(new_ipheader);	// Add the IPHeader into the packet
				//cerr<< "---------------------------------" << endl;
				//cerr << "\n new_ipheader: \n" << new_ipheader << endl;	// Print the header for testing and Part 1
				//cerr<< "---------------------------------" << endl;
				new_tcphead.SetSourcePort(5050, to_send);
				new_tcphead.SetDestPort(5050, to_send);
				new_tcphead.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_send);

                new_tcphead.SetSeqNum(one, to_send);
				new_tcphead.SetAckNum(one, to_send);

                new_tcphead.SetWinSize(100, to_send);
				new_tcphead.SetUrgentPtr(0, to_send);

				new_tcphead.SetFlags(cap_flags, to_send);	// Set the flag in the header

				// Print out the finished TCP header for testing
				//cerr<< "---------------------------------" << endl;
				//cerr << "\new_tcpheader: \n" << new_tcphead<< endl;
				//cerr<< "---------------------------------" << endl;

				new_tcphead.RecomputeChecksum(to_send);

				to_send.PushBackHeader(new_tcphead);		// Push the header into the packet

				//printf("Sending Packet\n");
				MinetSend(mux, to_send);
				//printf("Packet Sent\n");

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