/*
	Joshua Miner (jmm299)
	Ethan Welsh (ejw45)
	CS1652 Fall 2014
	Project 2: TCP Module for Minet
	Due: 16Nov2014
*/

// Include other libraries needed
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
#include "Minet.h"

// Declare functions before main
void build_packet(Packet &, ConnectionToStateMapping<TCPState> &, int, int);
int SendData(const MinetHandle &, const MinetHandle &, ConnectionToStateMapping<TCPState> &, Buffer);

using namespace std;

// For use in setting the flags
#define SYN 1
#define ACK 2
#define SYN_ACK 3
#define PSHACK 4
#define FIN 5
#define FIN_ACK 6
#define RST 7

int main(int argc, char *argv[])
{
	// This was all given to us in the initial file.
    MinetHandle mux; 
    MinetHandle sock; 
    ConnectionList<TCPState> conn_list;
    
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
    MinetSendToMonitor(MinetMonitoringEvent("tcp_module STUB VERSION handling tcp traffic........"));

    MinetEvent event;
    int timeout = 1;

    while (MinetGetNextEvent(event, timeout) == 0)
    {
        if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
        {
            /* * * * * * * * * * * *
             * * * * * * * * * * * *
             * *       MUX       * *
             * * * * * * * * * * * *
             * * * * * * * * * * * */
            if (event.handle == mux)
            {
                // ip packet has arrived!
                cerr<<"MUX"<<endl;
                SockRequestResponse request;
                SockRequestResponse response;

                // Tear apart packet for data
                Packet recv_packet; // Packet that has been received
                Buffer data_buffer;	// Buffer to hold data

                MinetReceive(mux, recv_packet); // Receive packet

                Connection conn;	// Hold the information for the connection
                Packet send_packet;	// Packet that will be built and transmitted to the destination
                unsigned char tcph_size, iph_size;	// Store the size of the tcp and ip headers
                unsigned short window, is_urgent, total_size;	// other size variables
                unsigned int seq_num, ack_num;	// hold the sequence and ack number values
                unsigned char recv_flags;	// To hold the flags from the packet

                recv_packet.ExtractHeaderFromPayload<TCPHeader>(TCPHeader::EstimateTCPHeaderLength(recv_packet));	// Estimate the length and extract the header
                TCPHeader recv_tcph; // For storing the TCP header
                recv_tcph = recv_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet

                IPHeader recv_iph;	// For holding the IP header
                recv_iph = recv_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet

                cerr << "Gathering the connection information from the packet" << endl;
                recv_iph.GetDestIP(conn.src);	// Get the destination IP Address and store in the source
                recv_iph.GetSourceIP(conn.dest);	// Get the sourcec IP Address and store in the destination
                recv_iph.GetProtocol(conn.protocol);	// Get the protocol used
                recv_tcph.GetDestPort(conn.srcport);	// Get the destination port and store in the source port
                recv_tcph.GetSourcePort(conn.destport);	// Get the source port and store in the destination port

                recv_iph.GetFlags(recv_flags);	// Pull the flags from the header

                recv_tcph.GetSeqNum(seq_num);	// Get the sequence number
                recv_tcph.GetAckNum(ack_num);	// Get the ack number
                recv_tcph.GetFlags(recv_flags);	// Get flags
                recv_tcph.GetWinSize(window);	// Update the window size
                recv_tcph.GetUrgentPtr(is_urgent);	// Is this an important packet
                recv_tcph.GetHeaderLen(tcph_size);	// Size of the header

                recv_iph.GetTotalLength(total_size);	// Total packet size
                recv_iph.GetHeaderLength(iph_size);	// size of the ip header
                total_size = total_size - tcph_size - iph_size;	// Get the amount of data contained in packet
                data_buffer = recv_packet.GetPayload().ExtractFront(total_size);	// Get the data

                ConnectionList<TCPState>::iterator CL_iterator = conn_list.FindMatching(conn);	// Iterate through to see if the connection is in the list
                if (CL_iterator == conn_list.end())	// If the search makes it to the end of the list
                {
                    cerr << "The connection was not in the list!" << endl;
                }
                unsigned int curr_state; // This holds the current state of the connection
                curr_state = CL_iterator->state.GetState();	// get the state of the connection
                switch(curr_state)
                {
                    case LISTEN:
                        cerr << "LISTENING in Mux...." << endl;
                        if (IS_SYN(recv_flags))	// If a SYN Packet was received
                        {
                            cerr << "Packet received is a SYN" << endl;
                            // Update all the data in the CTSM
                            CL_iterator->connection = conn; // Set conn to connection
                            CL_iterator->state.SetState(SYN_RCVD);
                            CL_iterator->state.last_acked = CL_iterator->state.last_sent;
                            CL_iterator->state.SetLastRecvd(seq_num + 1);
                            CL_iterator->bTmrActive = true; // Timeout set
                            CL_iterator->timeout=Time() + 5; // Set to 5 seconds

                            // Make the SYNACK packet
                            CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
                            build_packet(send_packet, *CL_iterator, SYN_ACK, 0);	// Function call to build the SYNACK packet`
                            MinetSend(mux, send_packet);	// Send the packet to the mux
                            sleep(2);	// Wait a couple of seconds
                            MinetSend(mux, send_packet);	// Resend because minet is a work in progress
                            cerr << "Finished SYN operations" << endl;
                        }
                        cerr << "Finished LISTENING..." << endl;
                        break;
                    case SYN_RCVD:	// Server side case	
                        // Received a SYN packet and need to send a SYNACK
                        CL_iterator->state.SetState(ESTABLISHED);	// The connection is established so update the state
                        cerr << "Established the connection!" << endl;

                        CL_iterator->state.SetLastAcked(ack_num);	// Update the ack number
                        CL_iterator->state.SetSendRwnd(window);	// Update the window size
                        CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;	// Update the sequence number
                        CL_iterator->bTmrActive = false;
                        static SockRequestResponse * write = NULL;	// Write the resonse
                        write = new SockRequestResponse(WRITE, CL_iterator->connection, data_buffer, 0, EOK);	// Nothing to write to the sock layer
                        MinetSend(sock, *write);	// Send the response to the sock layer
                        delete write;

                        cerr << "Finished with the syn received portion\n" << endl;
                        break;
                    case SYN_SENT:	// Client side case
                        // We are expecting a SYNACK
                        if (IS_SYN(recv_flags) && IS_ACK(recv_flags))	// Is a SYNACK
                        {
                            cerr << "It is a SYNACK... YAY!" << endl;
							// Update the state
                            CL_iterator->state.SetSendRwnd(window);
                            CL_iterator->state.SetLastRecvd(seq_num + 1);
                            CL_iterator->state.last_acked = ack_num;
                            // Need to make an ACK packet
                            CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
                            build_packet(send_packet, *CL_iterator, ACK, 0);	// Function call to build the ACK
                            MinetSend(mux, send_packet);	// Send the packet
                            CL_iterator->state.SetState(ESTABLISHED);	// Update the state to be established
                            CL_iterator->bTmrActive = false;	
                            SockRequestResponse write (WRITE, CL_iterator->connection, data_buffer, 0, EOK);
                            MinetSend(sock, write);
                            cerr << "Finished with the SYNACK... and sent a SYN" << endl;
                        }
                        break;
                    case ESTABLISHED:
                        cerr << "START CASE ESTABLISHED\n" << endl;
                        if(IS_PSH(recv_flags) && IS_ACK(recv_flags))	// The packet contains data
                        {
                            cerr << " Start case for data packet received\n" << endl;

                            CL_iterator->state.SetSendRwnd(window);	// Update the window size
                            char buff[recv_packet.GetPayload().GetSize()+1];	// Establish char array to the size of the amount of data
                            recv_packet.GetPayload().GetData(buff, recv_packet.GetPayload().GetSize(), 0);	// Get the amount of data contained in the packet
                            buff[recv_packet.GetPayload().GetSize()] = 0; // Chuck a null terminating character on the end
                            int actualSize = strlen(buff);	// Get the actual size of the array

                            CL_iterator->state.last_recvd = seq_num + actualSize;	// Update the sequence number
                            CL_iterator->state.last_acked = ack_num;	// Update the ack number
                            // Write to socket
                            CL_iterator->state.RecvBuffer.AddBack(data_buffer);
                            SockRequestResponse write(WRITE, CL_iterator->connection, CL_iterator->state.RecvBuffer, actualSize, EOK);
                            MinetSend(sock, write);

                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, ACK, 0);
                            MinetSend(mux, send_packet);
                            cerr << "Finished dealing with a data packet\n" << endl;
                        }
                        else if(IS_FIN(recv_flags) && IS_ACK(recv_flags)) // finack packet
                        {
                            // Client sends a FINACK packet
                            cerr << "Dealing with a FINACK" << endl;
                            CL_iterator->state.SetState(CLOSE_WAIT);
                            CL_iterator->state.SetSendRwnd(window);
                            CL_iterator->state.last_recvd = seq_num + 1;
                            CL_iterator->state.last_acked = ack_num;

                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, FIN_ACK, 0);	// Function call to build the finack packet
                            MinetSend(mux, send_packet);	// Send the packet
                            cerr << "Finished with the FINACK" << endl;
                        }
                        else if(IS_ACK(recv_flags))	// We receive an ack for the packet we sent
                        {
                            cerr << "Got an ACK for what we just sent" << endl;
                            CL_iterator->state.SetLastRecvd((unsigned int)seq_num);	// Update the sequence number
                            CL_iterator->state.last_acked = ack_num;	// Update the ack number
                        }
                        else	// Somthing weird happened
                        {
                            cerr << "ERROR... unknown packet: " << recv_tcph << endl;
                        }
                        cerr << "===============END CASE ESTABLISHED===============\n" << endl;
                        break;
                    case FIN_WAIT1:
                        if(IS_FIN(recv_flags) && IS_ACK(recv_flags))	// If received a FINACK
                        {
                            cerr << "===============START CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
                            CL_iterator->state.SetState(FIN_WAIT2);	// Change the state to prepare to close the connection
                            CL_iterator->state.SetSendRwnd(window);

                            cerr<<"STATE AFTER:"<<CL_iterator->state<<endl;
							// Update the message numbers
                            CL_iterator->state.last_recvd = seq_num + 1;
                            CL_iterator->state.last_acked = ack_num;
                            CL_iterator->state.last_sent++; // Such hacky. Much bad.

                            cerr<<"STATE AFTER:"<<CL_iterator->state<<endl;

                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, ACK, 0);
                            MinetSend(mux, send_packet);
                            cerr << "===============END CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
                        }
                        break;
                    case FIN_WAIT2:
                        if(IS_ACK(recv_flags))	// Received an ACK when closing the connection
                        {
                            cerr << "===============START CASE FIN_WAIT2 + IS_ACK===============\n" << endl;
                            CL_iterator->state.SetSendRwnd(window);
							// Update the message numbers
                            CL_iterator->state.last_recvd = seq_num + 1;
                            CL_iterator->state.last_acked = ack_num;

                            cerr<<"CLIST_STATE:::"<<CL_iterator->state<<endl;	// Debug statement to check the state of the connection

                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, ACK, 0);
                            MinetSend(mux, send_packet);

                            // Finished communicating so closing the connection
                            SockRequestResponse finished;
                            finished.type = CLOSE;
                            finished.connection = CL_iterator->connection;
                            finished.bytes = 0;
                            finished.error = EOK;
                            MinetSend(sock, finished);
                            conn_list.erase(CL_iterator);	// Remove from the list of connections
                            cerr << "===============END CASE FIN_WAIT2 + IS_ACK===============\n" << endl;
                        }
                        break;
                    case CLOSE_WAIT:
                        cerr<<"ENTERED CLOSE WAIT."<<endl;
                        if(IS_ACK(recv_flags))
                        {
                            cerr << "===============START CASE CLOSE_WAIT + IS_ACK===============\n" << endl;
                            // Finished communicating now closing the connection
                            SockRequestResponse finished;
                            finished.type = CLOSE;
                            finished.connection = CL_iterator->connection;
                            finished.bytes = 0;
                            finished.error = EOK;
                            MinetSend(sock, finished);
                            conn_list.erase(CL_iterator);
                            cerr << "Connection closed!" << endl;
                            cerr << "===============END CASE CLOSE_WAIT + IS_ACK===============\n" << endl;
                        }
                        break;
                    case LAST_ACK:
                        cerr<<"Got LAST ACK"<<endl;
                        break;
                }
                cerr << "Finished in the mux portion!" << endl;
            }

            /* * * * * * * * * * * *
             * * * * * * * * * * * *
             * *      SOCK       * *
             * * * * * * * * * * * *
             * * * * * * * * * * * */

            if (event.handle == sock)
            {
                cerr<<"I got a sock event."<<endl;

                SockRequestResponse request, response;
                MinetReceive(sock, request);
                Packet recv_packet;	// For the packet that was received
                // Check to see if there is a matching connection in the ConnectionList
                ConnectionList<TCPState>::iterator CL_iterator = conn_list.FindMatching(request.connection);
                if (CL_iterator == conn_list.end())	// If the iterator makes it to the end of the list
                {
                    cerr<< "**********Connection was not found in the list**********" << endl;
                    switch (request.type)	// Based on the request type
                    {
                        case CONNECT:	// If the case is a connection
                        {
                            cerr << " Working in the connect case of sock\n" << endl;
                            TCPState client(1, SYN_SENT, 5);	// Update the state of the connection

                            ConnectionToStateMapping<TCPState> new_CTSM(request.connection, Time()+2, client, true);	// Create a new entry for the mapping
                            conn_list.push_back(new_CTSM);	// Add the entry to the mapping

                            build_packet(recv_packet, new_CTSM, SYN, 0);	// Build the syn packet
							// Send the packet twice since minet is a work in progress
                            MinetSend(mux, recv_packet);
                            sleep(1);	// Wait a second between transmissions.
                            MinetSend(mux, recv_packet);
							// Update and send the response
                            response.type = STATUS;
                            response.connection = request.connection;
                            response.bytes = 0;
                            response.error = EOK;
                            MinetSend(sock, response);

                            cerr << "Done with the connection case" << endl;
                        }
                            break;
                        case ACCEPT:	// As the client you want to accept the connection
                        {
                            cerr << "===============START CASE ACCEPT===============\n" << endl;
                            TCPState server(1, LISTEN, 5);	// Create a server state
                            ConnectionToStateMapping<TCPState> new_CTSM(request.connection, Time(), server, false);	// Create a mapping entry
                            conn_list.push_back(new_CTSM);	// Add the entry to the mapping
							// Update and send the reponse
                            response.type = STATUS;	
                            response.connection = request.connection;
                            response.bytes = 0;
                            response.error = EOK;
                            MinetSend(sock, response);
                            cerr << "===============END CASE ACCEPT===============" << endl;
                        }
                            break;
                        case STATUS:
                        {
                        }
                            break;
                        case WRITE:	// If the state is writing
                        {	
							// Update and send the response
                            response.type = STATUS;
                            response.connection = request.connection;
                            response.bytes = 0;
                            response.error = ENOMATCH;
                            MinetSend(sock, response);
                        }
                            break;
                        case FORWARD:
                        {
                        }
                            break;
                        case CLOSE:	// When the connection is in the process of being closed
                        {
							// Update and send the response
                            response.type = STATUS;
                            response.connection = request.connection;
                            response.bytes = 0;
                            response.error = EOK;
                            MinetSend(sock, response);
                        }
                            break;
                        default:
                        {
                        }
                            break;
                    }
                }
                else	// The connection was found in the list
                {
                    cerr<< "**********Connection was found in the list**********" << endl;
                    unsigned int curr_state; // State of the tcp connection on our side.
                    curr_state = CL_iterator->state.GetState();	// Get the state
                    Buffer data_buff;	// buffer for storing data
                    switch (request.type)
                    {
                        case CONNECT:	// This should not be an option since the connection must be established and put in the list in connect
                        {
                            cerr<< "Shouldn't see this... Sock-> Connect" << endl;
                        }
                            break;
                        case ACCEPT:	// Same as connect
                        {
                            cerr<< "Shouldn't see this... Sock-> Accept" << endl;
                        }
                            break;
                        case FORWARD:	// Does nothing for us
                        {
                            cerr<< "FORWARD"<<endl;
                        }
                            break;
                        case STATUS:
                        {
                            if (curr_state == ESTABLISHED)	// If the connection is established
                            {
                                unsigned data_sent = request.bytes; // number of bytes to send
                                CL_iterator->state.RecvBuffer.Erase(0,data_sent);	// empty out the buffer in the state
                                if(0 != CL_iterator->state.RecvBuffer.GetSize())	//if unable to 0 out the buffer
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
                            if (curr_state == ESTABLISHED)	// The connection state is established
                            {
                                if(CL_iterator->state.SendBuffer.GetSize()+request.data.GetSize() > CL_iterator->state.TCP_BUFFER_SIZE)	// If the amount of data is larger than the buffer
                                {
                                    // If there isn't enough space in the buffer
									// Update and send the response
                                    response.type = STATUS;
                                    response.connection = request.connection;
                                    response.bytes = 0;
                                    response.error = EBUF_SPACE;	// Error in the space available in the buffer
                                    MinetSend(sock, response);	// Send the response
                                }
                                else	// Less data than the buffer size
                                {
                                    Buffer to_copy= request.data;	// Get the data into the buffer
                                    int ret_value = 0;	// Store the return value from sending the data

                                    ret_value = SendData(mux, sock, *CL_iterator, to_copy);	// Function call to send the data

                                    if (ret_value == 0)	// If the send was successful
                                    {
										// Update and send the response
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
                            break;
                        case CLOSE:	// In the process of closing the connection
                        {
                            cerr << "===============Starting to close the connection===============\n" << endl;
                            if (curr_state == ESTABLISHED)	// If the state is presently established
                            {
                                CL_iterator->state.SetState(FIN_WAIT1);	// Set the state to finwait
                                CL_iterator->state.last_acked = CL_iterator->state.last_acked + 1;	// update the ack number

                                Packet send_packet;	// Declare a packet to be sent
                                build_packet(send_packet, *CL_iterator, FIN_ACK, 0); // WE SEND FIRST FINACK.
                                MinetSend(mux, send_packet);	// Send to the mux

								// Update and send the response
                                response.type = STATUS;
                                response.connection = request.connection;
                                response.bytes = 0;
                                response.error = EOK;
                                MinetSend(sock, response);
                            }
                            cerr << "===============END CASE CLOSE===============" << endl;
                        }
                            break;
                        default:
                            break;
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


/*
	This function is used to build the packets sent between the clients and server.
	
	Its' arguments include a pointer to the packet being sent, the connection mapping, type of message, and how 
	much data if any.
*/
void build_packet(Packet &to_build, ConnectionToStateMapping<TCPState> &the_mapping, int TCPHeaderType, int size_of_data)
{
    cerr<< "+--------------BUILD PACKET-----------+" << endl;
    unsigned char alerts = 0;	// Flags for the packet
    int packet_size = size_of_data + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH; // TODO changed to size of data...

    IPHeader new_ipheader;	// For building the ip header
    TCPHeader new_tcpheader;	// For building the tcp header

    new_ipheader.SetSourceIP(the_mapping.connection.src); // Set the source ip address
    new_ipheader.SetDestIP(the_mapping.connection.dest);	// Set the destination ip address
    new_ipheader.SetTotalLength(packet_size);	// set the size of the packet
    new_ipheader.SetProtocol(IP_PROTO_TCP);	// define the protocol used
    to_build.PushFrontHeader(new_ipheader);	// add the ip header into the packet

    new_tcpheader.SetSourcePort(the_mapping.connection.srcport, to_build);	// add in the source port
    new_tcpheader.SetDestPort(the_mapping.connection.destport, to_build);	// add in the destination port
    new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_build);	// define the tcp header size
    new_tcpheader.SetAckNum(the_mapping.state.GetLastRecvd(),to_build);	// set the ack number
    new_tcpheader.SetWinSize(the_mapping.state.GetRwnd(), to_build);	// set the window size
    new_tcpheader.SetUrgentPtr(0, to_build);	// set the urgency to 0.
    switch (TCPHeaderType)	// switch the case based on the header type
    {
        case SYN:	// If it is a SYN
        {
            SET_SYN(alerts);	// Set the flags as a SYN
        }
            break;
        case ACK:	// If it is an ACK
        {
            SET_ACK(alerts);	// Set the flag as an ACK
        }
            break;
        case SYN_ACK:	// If it is a SYNACK
        {
			// Update flags for both
            SET_ACK(alerts);
            SET_SYN(alerts);
        }
            break;
        case PSHACK:	// If it is a packet with data
        {
			// Update flags for both
            SET_PSH(alerts);
            SET_ACK(alerts);
        }
            break;
        case FIN:	// If it is a fin packet
        {
            SET_FIN(alerts);	// Update the flag
        }
            break;
        case FIN_ACK:	// If it is a FINACK
        {
			// Update flags for both
            SET_FIN(alerts);	
            SET_ACK(alerts);
        }
            break;
        case RST:	// Issue encountered
        {
            SET_RST(alerts);	// Set the flag so it is a reset
        }
            break;
        default:
        {
            break;
        }
    }

    new_tcpheader.SetFlags(alerts, to_build);   // Set the flag in the header
    new_tcpheader.SetSeqNum(the_mapping.state.GetLastSent(), to_build);	// Set the sequence number
    new_tcpheader.RecomputeChecksum(to_build);	// compute the checksum

    to_build.PushBackHeader(new_tcpheader);	// Push the header into the packet
    cerr<< "---------------Finished BUILD PACKET------------" << endl;
}

/*
	SendData is design to process data into chunks and send packets until all data has been sent to its destination.
	
	Its arguments include the mux and sock handlers, connection mapping, and the buffer of data to be sent.
	
	This will return the amount of bits left when the process is completed. 0 will mean success and and anything else will 
	result in an error.
*/
int SendData(const MinetHandle &mux, const MinetHandle &sock, ConnectionToStateMapping<TCPState> &the_mapping, Buffer data_buffer)
{
    Packet data_packet;	// Packet to send
    the_mapping.state.SendBuffer.AddBack(data_buffer);	// Get the data to send
    unsigned int bytes_to_send = data_buffer.GetSize();	// Get how much data to send
    while(bytes_to_send!= 0)	// While there is still something to send
    {
        unsigned int bytes_being_sent = min(bytes_to_send, TCP_MAXIMUM_SEGMENT_SIZE);	// How much being sent- which is less TCP size or amount of data`
        data_packet = the_mapping.state.SendBuffer.Extract(0, bytes_being_sent);	
        build_packet(data_packet, the_mapping, PSHACK, bytes_being_sent); // Build a packet with the data and the amount of it
        MinetSend(mux, data_packet);	// Send to the mux portion
        the_mapping.state.last_sent = the_mapping.state.last_sent + bytes_being_sent;	// Update the last amount sent
        bytes_to_send-= bytes_being_sent;	// Subtract the amount just sent from the amount left to send
    }
    cerr << "DATA HAS BEEN SENT\n" << endl;
    return bytes_to_send;	// return the amount left
}