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
#include "ip.h"

//void build_packet(Packet &, ConnectionToStateMapping<TCPState> &, int , int , bool);
void handshake(IPAddress, int, IPAddress, int, int, int, unsigned char, bool);
void build_packet(Packet &, IPAddress, int, IPAddress, int, int, int, int, int);
void server(IPAddress, int, IPAddress, int, int, int);
void client(IPAddress, int, IPAddress, int);

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


/*
 * MILESTONE 2


 * ************************************************SERVER***************************************************************
 * - Handshake: We'll wait for a SYN, send back a SYN ACK, then we'll get an ACK.
 * - Wait for Data: We, the server, NEVER SEND DATA. After we get the ACK, we'll wait for data packets to arrive.
 * - ACK Data: Once (and every time) a data packet arrives we'll ACK it.
 *
 * How we test it:
 * - We'll NC into our IP on the correct port number
 * - After we see the handshake go down, we can type into NC and it will send out what we type as data packets.
 * - We should see our ACKS (from the server side) coming back every time we do this.

 * ************************************************CLIENT***************************************************************
 * - Handshake: We send SYN, wait for a SYN ACK to come back, and then send an ACK.
 * - Send Data: Directly after we send the ACK for our handshake, we'll shoot out a data packet with something hardcoded
 * into it.
 *
 * How we test it:
 * - We'll hardcode a (single) data packet to get sent directly after the ACK that we send to complete the handshake.
 * - After we send the data, we should see the server (NC) ACK our packet back.
 */


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
	int timeout = 1;

    cerr<<"About to enter the enent loop."<<endl;

    unsigned int ack_num = 0;
    unsigned int seq_num = 0;

	while (MinetGetNextEvent(event, timeout) == 0)
	{
		if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
		{
			if (event.handle == mux)
			{
                /*
                 * This event describes anything that comes from below. Any packets that are received, be
                 * they SYNs, ACKs, SYN ACks, or just regular data packets will be processed inside of this
                 * event loop.
                 *
                 * This is also where the server will initiate its connections: it will receive a packet
                 * from the client, and it will send a reply using its IP and Port # as the source, and the
                 * IP and port number from the incoming packet as the destination.
                 */
				// ip packet has arrived!
                cerr<<"I got a mux event."<<endl;
                SockRequestResponse request;
                SockRequestResponse response;
				
				// Tear apart packet for data
				Packet recv_packet; // Receipt packet
				
				MinetReceive(mux, recv_packet); // Receive packet
				
	            unsigned short length = TCPHeader::EstimateTCPHeaderLength(recv_packet);	// Estimate length
				recv_packet.ExtractHeaderFromPayload<TCPHeader>(length);	// Get the Header from the packet
				TCPHeader recv_tcph; // For storing the TCP header
				recv_tcph = recv_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet
				
				IPHeader recv_iph;	// For holding the IP header
				recv_iph = recv_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet
				
				unsigned char recv_flags = 0;	// To hold the flags from the packet
				recv_tcph.GetFlags(recv_flags); // Assign with flags received from TCP Header
				

				recv_tcph.GetSeqNum(seq_num);
				
				IPAddress source;
				recv_iph.GetSourceIP(source);	// This will need to be the destination
				IPAddress dest;
				recv_iph.GetDestIP(dest);	// This will need to be the source

                unsigned short my_port = 0;
                recv_tcph.GetSourcePort(my_port);

                if(IS_SYN(recv_flags) && !IS_ACK(recv_flags)) // ___SYN___
				{

                    cerr<<"Nice to meet you..."<<endl;
					handshake(source, 8080, dest, my_port, seq_num, ack_num, recv_flags, false);
                    cerr<<"NEVER"<<endl;
                    ack_num = 3;
				}
                else if (IS_ACK(recv_flags))
                { // It's a data packet

                    cerr<<"Got an ACK... Entering server loop."<<endl;
                    Packet ack_packet;

                    server(dest, my_port, source, 8080, seq_num, 0);
                }
			}
			if (event.handle == sock)
			{
                /*
                 * This is how the application layer talks to us. When a connection is requested, it'll
                 * come from the sock. Once we see something coming from the sock, we will build a SYN
                 * packet using our IPAddress and Port # as the source, and the IP and Port # specified
                 * in the request. We'll build a SYN packet from this information, and send it out to the
                 * server. From then on, everything will be handled in the MUX.
                 */

                cerr<<"I got a sock event."<<endl;

				SockRequestResponse request;
				SockRequestResponse response;
				MinetReceive(sock, request);
				Packet recv_packet;
				// Check to see if there is a matching connection in the ConnectionList
				ConnectionList<TCPState>::iterator CL_iterator = conn_list.FindMatching(request.connection);
				
				if (CL_iterator == conn_list.end())
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

							handshake("192.168.128.1", 8080, "192.168.42.8", 8080, 0, 0, 0, true);
							for(;;);
							
							MinetSend(mux, recv_packet);
							sleep(1);
							MinetSend(mux, recv_packet);
							
							response.type = STATUS;
							response.connection = request.connection;
							response.bytes = 0;
							response.error = EOK;
							MinetSend(sock, response);
							
							cerr << "Done with the connection case" << endl;
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




void build_packet(Packet &to_build, IPAddress src_ip, int src_port, IPAddress dest_ip, int packet_type, int dest_port, int seq_num, int ack_num, int data_amount)
{
    unsigned char alerts = 0;
    int packet_size = data_amount + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH;

    IPHeader new_ipheader;
    TCPHeader new_tcpheader;

    new_ipheader.SetSourceIP(src_ip);
    new_ipheader.SetDestIP(dest_ip);
    new_ipheader.SetTotalLength(packet_size);
    new_ipheader.SetProtocol(IP_PROTO_TCP);
    to_build.PushFrontHeader(new_ipheader);
    //cerr << "\nNew ipheader: \n" << new_ipheader << endl;

    new_tcpheader.SetSourcePort(src_port, to_build);
    new_tcpheader.SetDestPort(dest_port, to_build);
    new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_build);

    new_tcpheader.SetAckNum(ack_num,to_build);
    new_tcpheader.SetWinSize(data_amount, to_build);
    new_tcpheader.SetUrgentPtr(0, to_build);

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
            SET_PSH(alerts);
            SET_ACK(alerts);
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
    new_tcpheader.SetFlags(alerts, to_build);

    new_tcpheader.RecomputeChecksum(to_build);

    new_tcpheader.SetSeqNum(seq_num, to_build);

    // Push the header into the packet
    to_build.PushBackHeader(new_tcpheader);
    //cerr<< "---------------Packet is built------------" << endl;
}


void handshake(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, int seq_num, int ack_num, unsigned char recv_flags, bool is_client)
{
    double timeout = 1;
    MinetEvent event;

    if(is_client)
    {
        /* * * * * * * * * * * * * * * * * *\
         *            SEND SYN             *
        \* * * * * * * * * * * * * * * * * */
        Packet p;

        IPHeader iph; // Holds the IP Header
        TCPHeader tcph; // Holds the TCP Header

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

        while(1)
        { // Wait for a SYN ACK to come back.
            while (MinetGetNextEvent(event, timeout) == 0)
            {
                if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
                {
                    if (event.handle == mux)
                    {
                        /* * * * * * * * * * * * * * * * * *\
                         *            SEND ACK             *
                        \* * * * * * * * * * * * * * * * * */

                        cerr << "U G0t a mux 3vent" << endl;

                        Packet mux_packet; // Receipt packet
                        MinetReceive(mux, mux_packet); // Receive packet

                        unsigned short length = TCPHeader::EstimateTCPHeaderLength(mux_packet);    // Estimate length
                        mux_packet.ExtractHeaderFromPayload<TCPHeader>(length);    // Get the Header from the packet

                        TCPHeader tcp_header; // For storing the TCP header
                        tcp_header = mux_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet

                        IPHeader ip_header;    // For holding the IP header
                        ip_header = mux_packet.FindHeader(Headers::IPHeader);    // Get the IP header from the MUX packet

                        unsigned char f = 0;    // To hold the flags from the packet
                        unsigned char cap_flags = 0;

                        tcp_header.GetFlags(f); // Assign f with flags received from TCP Header

                        unsigned int ack_num = 0;
                        unsigned int seq_num = 0;

                        tcp_header.GetSeqNum(seq_num);

                        Packet to_send;    // Declare the response packet
                        IPHeader iph;    // Holds the IP Header

                        IPAddress temp;        // hold the IP Address for switching around
                        ip_header.GetSourceIP(temp); // Should give us the source
                        iph.SetDestIP(temp);    // Set the destination IP --- NETLAB-3
                        ip_header.GetDestIP(temp); // Should give us the source
                        iph.SetSourceIP(temp);    // Set the source IP --- my IP Address

                        iph.SetProtocol(IP_PROTO_TCP);    // Set protocol to TCP

                        iph.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH);
                        to_send.PushFrontHeader(iph);    // Add the IPHeader into the packet

                        TCPHeader new_tcphead;    // Holds the TCP Header

                        if (IS_SYN(f) && IS_ACK(f))
                        {
                            seq_num = seq_num + 1;

                            new_tcphead.SetSeqNum(2, to_send);
                            new_tcphead.SetAckNum(seq_num, to_send);

                            SET_ACK(cap_flags);

                            cerr << "\nSYN ACK\n" << tcp_header << endl;
                        }
                        else
                        {
                            cerr << "This should never happen!" << endl;
                        }

                        new_tcphead.SetSourcePort(src_port, to_send);
                        new_tcphead.SetDestPort(dest_port, to_send);

                        new_tcphead.SetWinSize(100, to_send);
                        new_tcphead.SetUrgentPtr(0, to_send);

                        new_tcphead.SetFlags(cap_flags, to_send); // Set the flag in the header

                        new_tcphead.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_send);

                        new_tcphead.RecomputeChecksum(to_send);

                        to_send.PushBackHeader(new_tcphead); // Push the header into the packet

                        cerr << "I'm sending the ACK" << endl;
                        MinetSend(mux, to_send);

                        client(src_ip, src_port, dest_ip, dest_port);

                        return;

                    }
                }
            }
        }
    }
	else if (!is_client)	// If operating as the server
	{
		// Declare and build new packet to send off
        Packet to_send;	// Declare the response packet
		IPHeader new_iph;	// Holds the IP Header
		new_iph.SetDestIP(src_ip);	// Set the destination IP --- NETLAB-3
		new_iph.SetSourceIP(dest_ip);	// Set the source IP --- my IP Address
		new_iph.SetProtocol(IP_PROTO_TCP);	// Set protocol to TCP
		new_iph.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH);
		to_send.PushFrontHeader(new_iph);	// Add the IPHeader into the packet
		TCPHeader new_tcph;	// Holds the TCP Header
		unsigned char new_flags = 0;
		// This part sets the flags




        if(IS_SYN(recv_flags) && !IS_ACK(recv_flags)) // ___SYN___
		{
			SET_SYN(new_flags);
			SET_ACK(new_flags);

			new_tcph.SetSeqNum(1, to_send);
			new_tcph.SetAckNum(seq_num+1, to_send);

            cerr<<"Recieved a SYN as server..."<<endl;


		}
		else if(IS_SYN(recv_flags) && IS_ACK(recv_flags)) // ___SYN-ACK___
		{
			seq_num = seq_num + 1;

			new_tcph.SetSeqNum(ack_num, to_send);
			new_tcph.SetAckNum(seq_num, to_send);

			SET_ACK(new_flags);
		}
		else if(IS_ACK(recv_flags)) // ___ACK___
		{
            cerr<<"I recieved an ACK in the handshake, so we're entering the server loop"<<endl;
            server(                 src_ip,     src_port,           dest_ip,     dest_port,     3,           ack_num);
            //void server(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, int seq_num, int ack_num)


            cerr<<"You probably shouldn't see this message"<<endl;
		}
		unsigned short theRealPort = 0;

		new_tcph.SetSourcePort(src_port, to_send);
		new_tcph.SetDestPort(dest_port, to_send);
			
		new_tcph.SetWinSize(100, to_send);
		new_tcph.SetUrgentPtr(0, to_send);

		new_tcph.SetFlags(new_flags, to_send); // Set the flag in the header

		new_tcph.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_send);

		new_tcph.RecomputeChecksum(to_send);

		to_send.PushBackHeader(new_tcph); // Push the header into the packet

		MinetSend(mux, to_send);
	}
}


void client(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port)
{
    cerr<<"Entered into server loop. Sending hello world!!!!"<<endl;

    char *data = "hello world";
    Buffer *b = new Buffer(data, 12);
    Packet data_packet(*b);

    build_packet(data_packet, src_ip, 8080,     dest_ip, 0,           src_port,  2,       0,       11);
                 //&to_build, src_ip, src_port, dest_ip, packet_type, dest_port, seq_num, ack_num, data_amount)
    MinetSend(mux, data_packet);
}

void server(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, int seq_num, int ack_num)
{ // When we get an ACK, we send it here, and this deals with it.

    cerr<<"I got the ACK and now the handshake is over. It's time to wait for some data to come in."<<endl;
    cerr<<"when it does, I'll ACK it back."<<endl;

    MinetEvent event;
    int timeout = 1;

    while(1)
    { // Wait for a SYN ACK to come back.
        while (MinetGetNextEvent(event, timeout) == 0)
        {
            if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
            {
                if (event.handle == mux)
                { // This is automatically a ACK, so it's time for us to send some data.

                    cerr<<"In the loop, waiting for more ACKS."<<endl;
                    Packet data_packet; // Receipt packet
                    MinetReceive(mux, data_packet); // Receive packet

                    unsigned short length = TCPHeader::EstimateTCPHeaderLength(data_packet);    // Estimate length
                    data_packet.ExtractHeaderFromPayload<TCPHeader>(length);    // Get the Header from the packet

                    TCPHeader tcp_header; // For storing the TCP header
                    tcp_header = data_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet

                    IPHeader ip_header;    // For holding the IP header
                    ip_header = data_packet.FindHeader(Headers::IPHeader);    // Get the IP header from the MUX packet

                    Buffer d = data_packet.GetPayload();
                    cerr<<"Payload: "<<d<<endl;

                    Packet ack_packet;
                    build_packet(ack_packet, src_ip, 8080,     dest_ip, ACK,           src_port,  2,       (seq_num+2), 100);
                                //&to_build, src_ip, src_port, dest_ip, packet_type,   dest_port, seq_num, ack_num, data_amount)

                    MinetSend(mux, ack_packet);

                    cerr<<"ACK SENT!!!!!!!"<<endl;
                }

            }
        }
    }
}
