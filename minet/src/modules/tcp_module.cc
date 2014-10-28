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
void handshake(IPAddress, int, IPAddress, int, bool);

#include <iostream>

//#include "Minet.h"

using namespace std;


#define SYN 1
#define ACK 2
#define SYN_ACK 3
#define PSHACK 4
#define FIN 5
#define FIN_ACK 6
#define RST 7

MinetHandle mux; // Mutex to ensure not preempted
MinetHandle sock; // Socket


int main(int argc, char *argv[])
{
	ConnectionList<TCPState> conn_list;
    // This was all included in the code

    MinetInit(MINET_TCP_MODULE); // Initialize the minet tcp module

    mux = MinetIsModuleInConfig(MINET_IP_MUX) ?
            MinetConnect(MINET_IP_MUX) :
            MINET_NOHANDLE;
    sock = MinetIsModuleInConfig(MINET_SOCK_MODULE) ?
            MinetAccept(MINET_SOCK_MODULE) :
            MINET_NOHANDLE;

    MinetSendToMonitor(MinetMonitoringEvent("tcp_module STUB VERSION handling tcp traffic........"));

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

    MinetEvent event;
	int timeout=1;
	while (MinetGetNextEvent(event, timeout) == 0)
	{
		if ((event.eventtype == MinetEvent::Dataflow) && 
			(event.direction == MinetEvent::IN))
		{
			if (event.handle == mux)
			{
				// ip packet has arrived!
			}
			if (event.handle == sock)
			{
				SockRequestResponse request;
				SockRequestResponse response;
				MinetReceive(sock, request);
				Packet being_sent;
        
				// Check to see if there is a matching connection in the ConnectionList
				ConnectionList<TCPState>::iterator CL_iterator = conn_list.FindMatching(request.connection);
				
				if (CL_iterator == clist.end())
				{
					cerr<< "**********Connection was not found in the list**********" << endl;
					switch (request.type)
					{
						case CONNECT:
						{
							cerr << " Working in the connect case of sock\n" << endl;
							TCPState client(1, SYN_SENT, 5);
							
							ConnectionToStateMapping<TCPState> new_CTSM(request.connection, Time()+2, client, true);
							conn_list.push_back(new_CTSM);
							
							handshake("192.168.128.1", 5050, "136.142.184.139", 5050, true);
							for(;;);
							
							MinetSend(mux, being_sent);
							sleep(1);
							MinetSend(mux, being_sent);
							
							response.type = STATUS;
							response.connection = request.connection;
							response.bytes = 0;
							response.error = EOK;
							MinetSend(sock, response);
							
							cerr << "Done with the connection case" << endl;
                        }
						break;
						case ACCEPT: 
						{
							cerr << "Starting the accept case" << endl;
							TCPState server(1, LISTEN, 5);
							ConnectionToStateMapping<TCPState> new_CTSM(request.connection, Time(), server, false);
							conn_list.push_back(new_CTSM);
							response.type = STATUS;
							response.connection = req.connection;
							response.bytes = 0;
							response.error = EOK;
							MinetSend(sock, response);
							cerr << "Done with the accept case" << endl;
						}
						break;
						case STATUS:
						{
						}
						break;
						case WRITE:
						{
							response.type = STATUS;
							response.connection = req.connection;
							response.bytes = 0;
							response.error = ENOMATCH;
							MinetSend(sock, response);
						}
						break;
						case FORWARD:
						{
						}
						break;
						case CLOSE:
						{
							response.type = STATUS;
							response.connection = request.connection;
							response.bytes = 0;
							response.error = ENOMATCH;
							MinetSend(sock, response);
						}
						break;
						default:
						{
						}
						break;
					}
				}
				else
				{
					cerr<< "**********Connection was found in the list**********" << endl;
				}
			}
		}
		if (event.eventtype == MinetEvent::Timeout)
		{
			// timeout ! probably need to resend some packets
		}        
    }
    MinetDeinit();	// Deinitialize the minet stack
    return 0;	// Program finished
}

void build_packet(Packet &to_build, ConnectionToStateMapping<TCPState> &c_mapping, int packet_type, int data_amount, bool status)
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
	cerr << "\nNew ipheader: \n" << new_ipheader << endl;
	
    new_tcpheader.SetSourcePort(c_mapping.connection.srcport, to_build);
    new_tcpheader.SetDestPort(c_mapping.connection.destport, to_build);
    new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_build);

    new_tcpheader.SetAckNum(c_mapping.state.GetLastRecvd(),to_build);
    new_tcpheader.SetWinSize(c_mapping.state.GetRwnd(), to_build);
    new_tcpheader.SetUrgentPtr(0, to_build);

    // Set the alerat variable
    switch (packet_type)
    {
        case SYN:
        {
            SET_SYN(alerts);
            cerr << "It is a SYN!" << endl;
        }
		break;
        case ACK:
        {
            SET_ACK(alerts);
            cerr << "It is an ACK!" << endl;
        }
		break;
        case SYN_ACK:
        {
            SET_ACK(alerts);
            SET_SYN(alerts);
            cerr << "It is a HEADERTYPE_SYNACK!" << endl;
        }
		break;
		case PSHACK:
		{ 
			SET_PSH(flags);
			SET_ACK(flags);
			cerr << "It is a HEADERTYPE_PSHACK!" << endl;
		}
		break;
        case FIN:
        {
            SET_FIN(alerts);
            cerr << "It is a FIN!" << endl;
        }
		break;
        case FIN_ACK:
        {
            SET_FIN(alerts);
            SET_ACK(alerts);
            cerr << "It is a FINACK!" << endl;
        }
		break;
        case RST:
        {
            SET_RST(alerts);
            cerr << "It is a RESET!" << endl;
        }
		break;
        default:
        {
            break;
        }
    }

    // Set the flag
    new_tcpheader.SetFlags(alerts);

    new_tcpheader.RecomputeChecksum();

    // Push the header into the packet
    to_build.PushBackHeader(new_tcpheader);
    cerr<< "---------------Packet is built------------" << endl;
}

void handshake(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, bool is_client)
{
    double timeout = 1;
    MinetEvent event;

    if(is_client)
    {
        /* * * * * * * * * * * * * * * * * *\
         *            SEND SYN             *
        \* * * * * * * * * * * * * * * * * */
        Packet p;

        IPHeader iph;    // Holds the IP Header
        TCPHeader tcph;    // Holds the TCP Header

        unsigned char flags = 0;

        iph.SetSourceIP(src_ip); // Set the source IP --- my IP Address
        iph.SetDestIP(dest_ip);    // Set the destination IP --- NETLAB-3

        iph.SetProtocol(IP_PROTO_TCP);    // Set protocol to TCP

        iph.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH); // Total length of the packet being sent off
        p.PushFrontHeader(iph);    // Add the IPHeader into the packet

        tcph.SetSourcePort(src_port, p);
        tcph.SetDestPort(dest_port, p);

        tcph.SetSeqNum(1, p);
        tcph.SetWinSize(100, p);
        tcph.SetUrgentPtr(0, p);

        SET_SYN(flags); // Set the flag that this is a SYN packet
        tcph.SetFlags(flags, p); // Set the flag in the header

        tcph.RecomputeChecksum(p);

        tcph.SetHeaderLen(TCP_HEADER_BASE_LENGTH, p);
        p.PushBackHeader(tcph);    // Push the header into the packet

        MinetSend(mux, p); // Send the packet to mux
        sleep(1);
        MinetSend(mux, p);
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
                Packet recv_packet; // Receipt packet
                MinetReceive(mux, recv_packet); // Receive packet

                unsigned short length = TCPHeader::EstimateTCPHeaderLength(recv_packet);	// Estimate length
                recv_packet.ExtractHeaderFromPayload<TCPHeader>(length);	// Get the Header from the packet

                TCPHeader recv_tcph; // For storing the TCP header
                recv_tcph = recv_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet

                IPHeader recv_iph;	// For holding the IP header
                recv_iph = recv_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet

                unsigned char recv_flags = 0;	// To hold the flags from the packet
                unsigned char new_flags = 0;

                recv_tcph.GetFlags(recv_flags); // Assign f with flags received from TCP Header

                unsigned int ack_num = 0;
                unsigned int seq_num = 0;

                recv_tcph.GetSeqNum(seq_num);

                Packet to_send;	// Declare the response packet
                IPHeader new_iph;	// Holds the IP Header

                IPAddress temp_ip;		// hold the IP Address for switching around
                recv_iph.GetSourceIP(temp_ip); // Should give us the source
                recv_iph.GetDestIP(temp_ip); // Should give us the source

                new_iph.SetDestIP(temp_ip);	// Set the destination IP --- NETLAB-3
                new_iph.SetSourceIP(temp_ip);	// Set the source IP --- my IP Address

                new_iph.SetProtocol(IP_PROTO_TCP);	// Set protocol to TCP

                new_iph.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH);
                to_send.PushFrontHeader(new_iph);	// Add the IPHeader into the packet

                TCPHeader new_tcph;	// Holds the TCP Header

                if(IS_SYN(recv_flags) && !IS_ACK(recv_flags)) // ___SYN___
                {
                    printf("~~~~~~~~~~~~Got a SYN~~~~~~~~~~~~~~~~~~~\n");
                    SET_SYN(new_flags);
                    SET_ACK(new_flags);

                    new_tcph.SetSeqNum(1, to_send);
                    new_tcph.SetAckNum(seq_num+1, to_send);

                    printf("~~~~~~~~~~~Done with SYN~~~~~~~~~~~~~~~~~~~~\n");
                }
                else if(IS_SYN(recv_flags) && IS_ACK(recv_flags)) // ___SYN-ACK___
                {
                    printf("~~~~~~~~~~~SYNACK Actions~~~~~~~~~~~~~~~~~~~~\n");

                    seq_num = seq_num + 1;

                    new_tcph.SetSeqNum(ack_num, to_send);
                    new_tcph.SetAckNum(seq_num, to_send);

                    SET_ACK(new_flags);

                    printf("~~~~~~~ Finished SYNACK Actions~~~~~~~~~~~~~~~~~~~~~~~~\n");
                }
				else if(IS_ACK(recv_flags) && !IS_SYN(recv_flags)) // ___ACK___
                {
                    cerr<<"TCP HEADER: "<<recv_tcph<<endl;

                    printf("Three way handshake is complete. Nice to meet you.\n");
					/*// Build a packet
					Packet stamped;
					// Add in data --- "Hello World"
					// Update ACK number
					// Update sequence number
					// Recompute checksum
					tcph.RecomputeChecksum(p);
					// Send packet
					MinetSend(mux, p); // Send the packet to mux
					sleep(1);
					MinetSend(mux, p);
                    return;*/
                }
				else		// Received normal packet
				{
					// Tear apart packet
					// Print the contents
					// Build ACK packet
					// Update sequence number
					// recompute checksum
					// send packet
				}

                unsigned short theRealPort = 0;

                if(IS_SYN(recv_flags) && !IS_ACK(recv_flags))
                {
                    recv_tcph.GetSourcePort(theRealPort);
                    new_tcph.SetDestPort(theRealPort, to_send);
                    new_tcph.SetSourcePort(src_port, to_send);
                }
                else
                {
                    new_tcph.SetSourcePort(src_port, to_send);
                    new_tcph.SetDestPort(dest_port, to_send);
                }

                new_tcph.SetWinSize(100, to_send);
                new_tcph.SetUrgentPtr(0, to_send);

                new_tcph.SetFlags(new_flags, to_send); // Set the flag in the header

                new_tcph.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_send);

                new_tcph.RecomputeChecksum(to_send);

                to_send.PushBackHeader(new_tcph); // Push the header into the packet

                MinetSend(mux, to_send);

            }
            if (event.eventtype == MinetEvent::Timeout)
            {
                // timeout ! probably need to resend some packets
            }
        }
    }
}
