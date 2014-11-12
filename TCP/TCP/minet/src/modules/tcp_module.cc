#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include "../libminet/tcpstate.h"
#include "ip.h"

void handshake(IPAddress, int, IPAddress, int, int, int, unsigned char, bool);
void build_packet(Packet &, IPAddress, int, IPAddress, int, int, unsigned int, unsigned int, int);
void server(IPAddress, int, IPAddress, int, unsigned int, int);
void client(IPAddress, int, IPAddress, int);

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

int bad_programming;
int port_num = 7878;

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
				// ip packet has arrived!
                cerr<<"I got a mux event."<<endl;
                SockRequestResponse request;
                SockRequestResponse response;
				
				// Tear apart packet for data
				Packet recv_packet; // Receipt packet
				Buffer data_buffer;
				
				MinetReceive(mux, recv_packet); // Receive packet
				
				Connection conn;
				Packet send_packet;
				unsigned char tcph_size, iph_size;
				unsigned short window, is_urgent, total_size;
		
	            unsigned short length = TCPHeader::EstimateTCPHeaderLength(recv_packet);	// Estimate length
				recv_packet.ExtractHeaderFromPayload<TCPHeader>(length);	// Get the Header from the packet
				TCPHeader recv_tcph; // For storing the TCP header
				recv_tcph = recv_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet
				
				IPHeader recv_iph;	// For holding the IP header
				recv_iph = recv_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet
				
				unsigned char recv_flags = 0;	// To hold the flags from the packet
				recv_tcph.GetFlags(recv_flags); // Assign with flags received from TCP Header
				
				recv_tcph.GetSeqNum(seq_num);
				
				// The following are needed for identifying the connection (tuple of 5 values)
				cerr << "Gathering the connection information from the packet" << endl;
				recv_iph.GetDestIP(conn.src);
				recv_iph.GetSourceIP(conn.dest);
				recv_iph.GetProtocol(conn.protocol);
				recv_tcph.GetSourcePort(conn.destport);
				recv_tcph.GetDestPort(conn.srcport);
				cerr << "Printing the connection received:\n" << conn << endl;
				
				recv_tcph.GetSeqNum(seq_num);
				recv_tcph.GetAckNum(ack_num);
				recv_tcph.GetWinSize(window); 
				recv_tcph.GetUrgentPtr(is_urgent);
				recv_tcph.GetHeaderLen(tcph_size);
        
				recv_iph.GetTotalLength(total_size);
				recv_iph.GetHeaderLength(iph_size);
				total_size = total_size- tcph_size- iph_size;	// Get the amount of data contained in packet
				data_buffer = recv_packet.GetPayload().ExtractFront(total_size);	// Get the data

				ConnectionList<TCPState>::iterator CL_iterator = conn_list.FindMatching(conn);        
				if (CL_iterator == conn_list.end())
				{
					cerr << "The connection was not in the list!" << endl;
				}
				unsigned int curr_state; // This holds the current state of the connection
				curr_state = CL_iterator->state.GetState();
				cerr << " The current state: " << current_state << endl;
				
				switch(curr_state)
				{
					case LISTEN:
						cerr << "Listening for communications" << endl;
						if (IS_SYN(recv_flags)) 
						{
							cerr << "Packet received is a SYN" << endl;
							// Update all the data in the CTSM
							CL_iterator->connection = conn; // Set conn to connection
							CL_iterator->state.SetState(SYN_RCVD);
							CL_iterator->state.last_acked = CL_iterator->state.last_sent;
							CL_iterator->state.SetLastRecvd(seq_num + 1);
							CL_iterator->bTmrActive = true; // Timeout set
							CL_iterator->timeout=Time() + 5; // Set to 5 seconds
							cerr << "\nseq: " << seq_num << " and ack: " << ack_num << endl;
							
							// Forge the SYNACK packet
							CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
							
							// build_packet(packet_tosend, *CL_iterator, HEADERTYPE_SYNACK, 0, false);
							MinetSend(mux, send_packet);
							sleep(2);
							MinetSend(mux, send_packet);
							
							cerr << "Finished SYN operations" << endl;
						}
						break;
						case SYN_RCVD:
							// Received a SYN packet and need to send a SYNACK
							cerr << "Starting to build the SYNACK packet\n" << endl;
							CL_iterator->state.SetState(ESTABLISHED);
							cerr << "Established the connection!" << endl;
							CL_iterator->state.SetLastAcked(ack_num);
							CL_iterator->state.SetSendRwnd(window); 
							CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
							CL_iterator->bTmrActive = false;
							static SockRequestResponse * write = NULL;
							write = new SockRequestResponse(WRITE, CL_iterator->connection, data_buffer, 0, EOK);
							MinetSend(sock, *write);
							delete write;
							
							cerr << "Finished building the SYNACK\n" << endl;
                        break;
						case SYN_SENT:
							// We are expecting a SYNACK
							cerr << "We should be receiving a SYNACK right in order to be here\n" << endl;
							if (IS_SYN(flags) && IS_ACK(flags)) 
							{
									cerr << "It is a SYNACK... YAY!" << endl;
									CL_iterator->state.SetSendRwnd(window);
									CL_iterator->state.SetLastRecvd(seq_num + 1);
									CL_iterator->state.last_acked = ack_num;
									// Need to make an ACK packet
									CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
									// build_packet (packet_tosend, *CL_iterator, ACK, 0, false);
									MinetSend(mux, send_packet);
									CL_iterator->state.SetState(ESTABLISHED);
									CL_iterator->bTmrActive = false;
									SockRequestResponse write (WRITE, CL_iterator->connection, data_buffer, 0, EOK);
									MinetSend(sock, write);
									cerr << "Finished with the SYNACK... and sent a SYN" << endl;
							}
							break;
						case ESTABLISHED:
                        cerr << "START CASE ESTABLISHED\n" << endl;
                        if(IS_PSH(recv_flags) && IS_ACK(recv_flags))
						{
							cerr << " Start case for data packet received\n" << endl;
							cerr << "Received \"" << data_buffer << "\", buffer size: " << data_buffer.GetSize() << "." << endl;
							CL_iterator->state.SetSendRwnd(window);
							CL_iterator->state.last_recvd = seq_num+ data_buffer.GetSize();
							CL_iterator->state.last_acked = ack_num;
							// Write to socket
							CL_iterator->state.RecvBuffer.AddBack(data_buffer);           
							SockRequestResponse write(WRITE, CL_iterator->connection, CL_iterator->state.RecvBuffer, CL_iterator->state.RecvBuffer.GetSize(), EOK);
							MinetSend(sock, write);
                                        
							// Need to make an ACK packet
							// build_packet (packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
							MinetSend(mux, send_packet);
							cerr << "Finished dealing with a data packet\n" << endl;
                        }
                        else if(IS_FIN(recv_flags) && IS_ACK(recv_flags))
						{
							// This is when the client sends a FINACK
							cerr << "Dealing with a FINACK\n" << endl;
							CL_iterator->state.SetState(CLOSE_WAIT);
							CL_iterator->state.SetSendRwnd(window);
							CL_iterator->state.last_recvd = seq_num+1;
							CL_iterator->state.last_acked = ack_num;        
							
							// Need to make an ACK packet
							// build_packet(packet_tosend, *CL_iterator, HEADERTYPE_FINACK, 0, false);
							MinetSend(mux, send_packet);
							cerr << "Finished with the FINACK\n" << endl;
						}
                        // we are the client, the server is ACK'ing our packet
                        else if(IS_ACK(flags))
						{
							cerr << "Got an ACK for what we just sent" << endl;
							CL_iterator->state.SetLastRecvd((unsigned int)seq_num);
							CL_iterator->state.last_acked = ack_num;
                        }
                        else 
						{
							cerr << "Unknown packet: " << recv_tcph << endl;
                        }
                        cerr << "===============END CASE ESTABLISHED===============\n" << endl;
                        break;
						case FIN_WAIT1:
							if(IS_FIN(recv_flags) && IS_ACK(recv_flags))
							{
								cerr << "===============START CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
								CL_iterator->state.SetState(FIN_WAIT2);
								CL_iterator->state.SetSendRwnd(window);
								CL_iterator->state.last_recvd = seq_num+1;
								CL_iterator->state.last_acked = ack_num;
                                        
								// Need to make an ACK packet
								// build_packet(packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
								MinetSend(mux, send_packet);
								cerr << "===============END CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
							}
							break;
						case FIN_WAIT2:
							if(IS_ACK(recv_flags))
							{
								CL_iterator->state.SetSendRwnd(window);
								CL_iterator->state.last_recvd = seq_num+1;
								CL_iterator->state.last_acked = ack_num;
								
								// Need to make an ACK packet
								// build_packet(packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
								MinetSend(mux, send_packet);
								
								// Finished and closing
								SockRequestResponse finished;
								finished.type = CLOSE;
								finished.connection = CL_iterator->connection;
								finished.bytes = 0;
								finished.error = EOK;
								MinetSend(sock, finished);
								
								clist.erase(CL_iterator);
								cerr << "===============END CASE FIN_WAIT2 + IS_FIN + IS_ACK===============\n" << endl;
							}
							break;							
						case TIME_WAIT:
							break;
						case CLOSE_WAIT:
							if(IS_ACK(recv_flags))
							{
								cerr << "===============START CASE CLOSE_WAIT + IS_ACK===============\n" << endl;
								// Finished and closing
								SockRequestResponse finished;
								finished.type = CLOSE;
								finished.connection = CL_iterator->connection;
								finished.bytes = 0;
								finished.error = EOK;
								MinetSend(sock, finished);
								clist.erase(CL_iterator);
								cerr << "Connection closed!" << endl;
								cerr << "===============END CASE CLOSE_WAIT + IS_ACK===============\n" << endl;
							}
							break;
						case LAST_ACK:
							break;
						return;
				}
				cerr << "Finished in the mux portion!" << endl;
			}
			if (event.handle == sock)
			{
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

							// Can we convert this to work with build_packet
							// build_packet(packet_tosend, new_CTSM, SYN, 0);
							handshake("192.168.128.1", port_num, "192.168.42.8", port_num, 0, 0, 0, true);
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
						case ACCEPT:
						{
							cerr << "===============START CASE ACCEPT===============\n" << endl;
							TCPState server(1, LISTEN, 5);
							ConnectionToStateMapping<TCPState> new_CTSM(request.connection, Time(), server, false);
							clist.push_back(new_CTSM);
							response.type = STATUS;
							response.connection = request.connection;
							response.bytes = 0;
							response.error = EOK;
							MinetSend(sock, response);
							cerr << "===============END CASE ACCEPT===============" << endl;
						}
						break;
						case WRITE: 
						{
							response.type = STATUS;
							response.connection = request.connection;
							response.bytes = 0;
							response.error = ENOMATCH;
							MinetSend(sock, response);
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
					unsigned int curr_state; // State of the tcp connection on our side.
					curr_state = CL_iterator->state.GetState();	// Get the state
					Buffer data_buff;	// buffer for storing data
					switch (req.type)
					{
						case CONNECT:
						{
							cerr<< "Shouldn't see this... Sock-> Connect" << endl;
						}
						break;
						case ACCEPT:
						{ 
							cerr<< "Shouldn't see this... Sock-> Accept" << endl;
						}
						break;
						case STATUS:
						{
							if (my_state == ESTABLISHED)
							{
								unsigned data_sent = request.bytes; // number of bytes send
								CL_iterator->state.RecvBuffer.Erase(0,data_sent);
								if(0 != CL_iterator->state.RecvBuffer.GetSize()) 
								{ // Didn't finish writing
									SockRequestResponse write (WRITE, CL_iterator->connection, CL_iterator->state.RecvBuffer, CL_iterator->state.RecvBuffer.GetSize(), EOK);
									MinetSend(sock, write);
								}
							}
						}
						break;
						case WRITE:
						{
							cerr << "===============Write Case in SOCK===============\n" << endl;
							if (curr_state == ESTABLISHED)
							{
								if(CL_iterator->state.SendBuffer.GetSize()+req.data.GetSize() > CL_iterator->state.TCP_BUFFER_SIZE)
								{
									// If there isn't enough space in the buffer
									response.type = STATUS;
									response.connection = request.connection;
									response.bytes = 0;
									response.error = EBUF_SPACE;
									MinetSend(sock, response);
								}
								else
								{
									Buffer copy_buffer = req.data; // Dupe the buffer
									int ret_value = 0;
									// TODO
									//SendData(mux, sock, *CL_iterator, copy_buffer);
                                        
									if (return_value == 0) 
									{
										response.type = STATUS;
										response.connection = request.connection;
										response.bytes = data_buff.GetSize();
										response.error = EOK;
										MinetSend(sock, response);
									}
								}
							}
							cerr << "===============Finished in SOCK- Write===============\n" << endl;
						}
						case CLOSE: 
						{
							cerr << "===============Starting to close the connection===============\n" << endl;
							if (curr_state == ESTABLISHED)
							{
								CL_iterator->state.SetState(FIN_WAIT1);
								CL_iterator->state.last_acked = CL_iterator->state.last_acked+1;
				
								//build_packet(packet_tosend, *CL_iterator, FINACK, 0, false);
								MinetSend(mux, packet_tosend);
	
								response.type = STATUS;
								response.connection = request.connection;
								response.bytes = 0;
								response.error = EOK;
								MinetSend(sock, response);
							}
							cerr << "===============END CASE CLOSE===============" << endl;
						}
					}
				}
			}
			cerr << "Finished in the sock portion!" << endl;
		}
		if (event.eventtype == MinetEvent::Timeout)
		{
			// timeout ! probably need to resend some packets
		}        
    }
    MinetDeinit();	// Deinitialize the minet stack
    return 0;	// Program finished
}

void build_packet(Packet &to_build, ConnectionToStateMapping<TCPState> &the_mapping, int TCPHeaderType, int size_of_data) 
{
	//cerr << "Beginning to build the packet" << endl;
    unsigned char alerts = 0;
    int packet_size = data_amount + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH;

    IPHeader new_ipheader;
    TCPHeader new_tcpheader;

    new_ipheader.SetSourceIP(the_mapping.src);
    new_ipheader.SetDestIP(the_mapping.dest);
    new_ipheader.SetTotalLength(packet_size);
    new_ipheader.SetProtocol(IP_PROTO_TCP);
    to_build.PushFrontHeader(new_ipheader);

    new_tcpheader.SetSourcePort(the_mapping.src, to_build);
    new_tcpheader.SetDestPort(the_mapping.dest, to_build);
    new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_build);

    new_tcpheader.SetAckNum(the_mapping.state.GetLastRecvd(),to_build);
    new_tcpheader.SetWinSize(the_mapping.state.GetRwnd(), to_build);
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
            cerr << "It is a SYN_ACK!" << endl;
        }
            break;
        case PSHACK:
        {
            SET_PSH(alerts);
            SET_ACK(alerts);
            cerr << "It is a PSH_ACK!" << endl;
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

    new_tcpheader.SetSeqNum(the_mapping.state.GetLastSent(), to_build);

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
                        IPHeader iph;     // Holds the IP Header

                        IPAddress temp;               // hold the IP Address for switching around
                        ip_header.GetSourceIP(temp); // Should give us the source
                        iph.SetDestIP(temp);        // Set the destination IP --- NETLAB-3
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

            bad_programming = seq_num + 1;

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
            server(                 src_ip,     src_port,           dest_ip,     dest_port,     3,           4);
            //void server(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, int seq_num, int ack_num)
		}
		else if(IS_FIN(recv_flags) && !IS_ACK(recv_flags))	// If it is a FIN
		{
			cerr<<"I received a FIN... YAY!"<<endl;
			// work_finished();
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

    int d_am = 0;

    string dayName[] = {"Sunday", "Monday", "Tuesday", "Wedensday", "Thursday", "Friday", "Saturday"};

    int seq_num = 2;

    for(int i = 0; i < 7; i++)
    {
        const char *data = dayName[i].c_str();

        Buffer *b = new Buffer(data, strlen(data) + 1);
        Packet data_packet(*b);

        d_am = d_am + strlen(data);
		
        build_packet(data_packet, src_ip, port_num,     dest_ip, 0,           src_port,  seq_num,       0,     strlen(data));
        seq_num = seq_num + strlen(data);

        MinetSend(mux, data_packet);
        sleep(1);
    }
	// Disconnect function call will be placed here.
	// disconnect()
}

void server(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, unsigned int seq_num, int ack_num)
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


                    unsigned int their_seq;

                    tcp_header.GetSeqNum(their_seq);


                    Buffer d = data_packet.GetPayload();
                    cerr<<"Payload: "<<d<<endl;

                    Packet ack_packet;

                    char buff[data_packet.GetPayload().GetSize()+1];
                    data_packet.GetPayload().GetData(buff, data_packet.GetPayload().GetSize(), 0);
                    buff[data_packet.GetPayload().GetSize()] = 0; // Chuck a null terminating character on the end
                    int actualSize = strlen(buff);

                    ack_num = their_seq + actualSize; // TODO GET PACKET LENGTH

                    cerr<<"@The size is "<<actualSize<<endl;
                    cerr<<"Sending out an ACK of "<<ack_num<<endl;
                    cerr<<"The ack_num is: "<< ack_num<<endl;
                    build_packet(ack_packet, src_ip, port_num, dest_ip, ACK, src_port,  seq_num, ack_num, 0);

                    MinetSend(mux, ack_packet);

                    cerr<<"ACK SENT!!!!!!!"<<endl;
                }

            }
        }
    }
}

/*
// TODO Still need to fix the seq and ack numbers
// TODO Still need to configure this for server
	-- Create a function to build_fin
	-- Create a function to build_finack
void disconnect(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, int seq_num, int ack_num, unsigned char recv_flags, bool is_client)
{
    double timeout = 1;
    MinetEvent event;

    if(is_client)
    {
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
        SET_FIN(flags); // Set the flag that this is a FIN packet
        tcph.SetFlags(flags, p); // Set the flag in the header
        tcph.RecomputeChecksum(p);
        tcph.SetHeaderLen(TCP_HEADER_BASE_LENGTH, p);
        p.PushBackHeader(tcph);    // Push the header into the packet

        MinetSend(mux, p); // Send the packet to mux
        sleep(1);
        MinetSend(mux, p);

        while(1)
        {
			while (MinetGetNextEvent(event, timeout) == 0)
            {
                if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
                {
                    if (event.handle == mux)
                    {
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
                        IPHeader iph;     // Holds the IP Header

                        IPAddress temp;               // hold the IP Address for switching around
                        ip_header.GetSourceIP(temp); // Should give us the source
                        iph.SetDestIP(temp);        // Set the destination IP --- NETLAB-3
                        ip_header.GetDestIP(temp); // Should give us the source
                        iph.SetSourceIP(temp);    // Set the source IP --- my IP Address

                        iph.SetProtocol(IP_PROTO_TCP);    // Set protocol to TCP

                        iph.SetTotalLength(IP_HEADER_BASE_LENGTH + TCP_HEADER_BASE_LENGTH);
                        to_send.PushFrontHeader(iph);    // Add the IPHeader into the packet

                        TCPHeader new_tcphead;    // Holds the TCP Header

                        if (IS_FIN(f) && !IS_ACK(f))
                        {
                            seq_num = seq_num + 1;

                            new_tcphead.SetSeqNum(2, to_send);
                            new_tcphead.SetAckNum(seq_num, to_send);

                            SET_ACK(cap_flags);
							SET_FIN(cap_flags);
                            cerr << "\nFIN_ACK\n" << tcp_header << endl;
                        }
                        else
                        {
                            cerr << "This should never happen!" << endl;
							cerr << "Encountered an error in generating a FINACK!" << endl;
                        }

                        new_tcphead.SetSourcePort(src_port, to_send);
                        new_tcphead.SetDestPort(dest_port, to_send);

                        new_tcphead.SetWinSize(100, to_send);
                        new_tcphead.SetUrgentPtr(0, to_send);

                        new_tcphead.SetFlags(cap_flags, to_send); // Set the flag in the header

                        new_tcphead.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_send);

                        new_tcphead.RecomputeChecksum(to_send);

                        to_send.PushBackHeader(new_tcphead); // Push the header into the packet

                        cerr << "I'm sending the FIN_ACK" << endl;
                        MinetSend(mux, to_send);

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

            bad_programming = seq_num + 1;

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
            server(                 src_ip,     src_port,           dest_ip,     dest_port,     3,           4);
            //void server(IPAddress src_ip, int src_port, IPAddress dest_ip, int dest_port, int seq_num, int ack_num)
		}
		else if(IS_FIN(recv_flags) && !IS_ACK(recv_flags))	// If it is a FIN
		{
			cerr<<"I received a FIN... YAY!"<<endl;
			// work_finished();
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
*/