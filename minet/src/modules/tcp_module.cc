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

void build_packet(Packet &, ConnectionToStateMapping<TCPState> &, int, int);
int SendData(const MinetHandle &, const MinetHandle &, ConnectionToStateMapping<TCPState> &, Buffer);

using namespace std;

#define SYN 1
#define ACK 2
#define SYN_ACK 3
#define PSHACK 4
#define FIN 5
#define FIN_ACK 6
#define RST 7

int main(int argc, char *argv[])
{
    MinetHandle mux;
    MinetHandle sock;
    ConnectionList<TCPState> conn_list;

    MinetInit(MINET_TCP_MODULE);

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
            /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
             * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
             * *                                     MUX                                   * *
             * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
             * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
            if (event.handle == mux)
            { // ip packet has arrived!
                SockRequestResponse request;
                SockRequestResponse response;

                Packet recv_packet; // Receipt packet
                Buffer data_buffer;

                MinetReceive(mux, recv_packet); // Receive packet

                Connection conn;
                Packet send_packet;
                unsigned char tcph_size, iph_size;
                unsigned short window, is_urgent, total_size;
                unsigned int seq_num, ack_num;
                unsigned char recv_flags;	// To hold the flags from the packet

                recv_packet.ExtractHeaderFromPayload<TCPHeader>(TCPHeader::EstimateTCPHeaderLength(recv_packet));

                TCPHeader recv_tcph; // For storing the TCP header of the incoming packet
                recv_tcph = recv_packet.FindHeader(Headers::TCPHeader); // Get the TCP header from the MUX packet

                IPHeader recv_iph;	// For holding the IP header of the incoming packet
                recv_iph = recv_packet.FindHeader(Headers::IPHeader);	// Get the IP header from the MUX packet

                cerr << "Gathering the connection information from the packet" << endl;
                recv_iph.GetDestIP(conn.src);
                recv_iph.GetSourceIP(conn.dest);
                recv_iph.GetProtocol(conn.protocol);
                recv_tcph.GetDestPort(conn.srcport);
                recv_tcph.GetSourcePort(conn.destport);

                recv_iph.GetFlags(recv_flags);

                recv_tcph.GetSeqNum(seq_num);
                recv_tcph.GetAckNum(ack_num);
                recv_tcph.GetFlags(recv_flags);
                recv_tcph.GetWinSize(window);
                recv_tcph.GetUrgentPtr(is_urgent);
                recv_tcph.GetHeaderLen(tcph_size);

                recv_iph.GetTotalLength(total_size);
                recv_iph.GetHeaderLength(iph_size);
                total_size = total_size - tcph_size - iph_size;	// Get the amount of data contained in packet
                data_buffer = recv_packet.GetPayload().ExtractFront(total_size);	// Get the data

                ConnectionList<TCPState>::iterator CL_iterator = conn_list.FindMatching(conn);
                if (CL_iterator == conn_list.end())
                {
                    cerr << "The connection was not in the list!" << endl;
                }
                unsigned int curr_state; // This holds the current state of the connection
                curr_state = CL_iterator->state.GetState();

                switch(curr_state)
                {
                    case LISTEN:
                        cerr << "LISTENING in Mux...." << endl;
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

                            // Make the SYNACK packet
                            CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;

                            build_packet(send_packet, *CL_iterator, SYN_ACK, 0);
                            MinetSend(mux, send_packet);
                            sleep(2);
                            MinetSend(mux, send_packet);

                            cerr << "Finished SYN operations" << endl;
                        }
                        cerr << "Finished LISTENING..." << endl;
                        break;
                    case SYN_RCVD:
                        // Received a SYN packet and need to send a SYNACK

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

                        cerr << "Finished with the syn received portion\n" << endl;
                        break;
                    case SYN_SENT:
                        // We are expecting a SYNACK
                        if (IS_SYN(recv_flags) && IS_ACK(recv_flags))
                        {
                            cerr << "It is a SYNACK... YAY!" << endl;
                            CL_iterator->state.SetSendRwnd(window);
                            CL_iterator->state.SetLastRecvd(seq_num + 1);
                            CL_iterator->state.last_acked = ack_num;
                            // Need to make an ACK packet
                            CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
                            build_packet(send_packet, *CL_iterator, ACK, 0);
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

                            CL_iterator->state.SetSendRwnd(window);

                            char buff[recv_packet.GetPayload().GetSize()+1];
                            recv_packet.GetPayload().GetData(buff, recv_packet.GetPayload().GetSize(), 0);
                            buff[recv_packet.GetPayload().GetSize()] = 0; // Chuck a null terminating character on the end
                            int actualSize = strlen(buff);

                            CL_iterator->state.last_recvd = seq_num + actualSize;

                            CL_iterator->state.last_acked = ack_num;
                            // Write to socket
                            CL_iterator->state.RecvBuffer.AddBack(data_buffer);
                            SockRequestResponse write(WRITE, CL_iterator->connection, CL_iterator->state.RecvBuffer, actualSize, EOK);
                            MinetSend(sock, write);

                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, ACK, 0);
                            MinetSend(mux, send_packet);
                            cerr << "Finished dealing with a data packet\n" << endl;
                        }
                        else if(IS_FIN(recv_flags) && IS_ACK(recv_flags))
                        {
                            // This is when the client sends a FINACK
                            cerr << "Dealing with a FINACK" << endl;
                            CL_iterator->state.SetState(CLOSE_WAIT);
                            CL_iterator->state.SetSendRwnd(window);
                            CL_iterator->state.last_recvd = seq_num + 1;
                            CL_iterator->state.last_acked = ack_num;

                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, FIN_ACK, 0);
                            MinetSend(mux, send_packet);
                            cerr << "Finished with the FINACK" << endl;
                        }
                            // we are the client, the server is ACK'ing our packet
                        else if(IS_ACK(recv_flags))
                        {
                            cerr << "Got an ACK for what we just sent" << endl;
                            CL_iterator->state.SetLastRecvd((unsigned int)seq_num);
                            CL_iterator->state.last_acked = ack_num;
                        }
                        else
                        {
                            cerr << "ERROR... unknown packet: " << recv_tcph << endl;
                        }
                        cerr << "===============END CASE ESTABLISHED===============\n" << endl;
                        break;
                    case FIN_WAIT1:
                        if(IS_FIN(recv_flags) && IS_ACK(recv_flags))
                        {
                            cerr << "===============START CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
                            CL_iterator->state.SetState(FIN_WAIT2);
                            CL_iterator->state.SetSendRwnd(window);

                            cerr<<"STATE AFTER:"<<CL_iterator->state<<endl;

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

                        if(IS_ACK(recv_flags))
                        {
                            cerr << "===============START CASE FIN_WAIT2 + IS_ACK===============\n" << endl;
                            CL_iterator->state.SetSendRwnd(window);
                            CL_iterator->state.last_recvd = seq_num + 1;
                            CL_iterator->state.last_acked = ack_num;

                            cerr<<"CLIST_STATE:::"<<CL_iterator->state<<endl;



                            // Need to make an ACK packet
                            build_packet(send_packet, *CL_iterator, ACK, 0);
                            MinetSend(mux, send_packet);

                            // Finished and closing
                            SockRequestResponse finished;
                            finished.type = CLOSE;
                            finished.connection = CL_iterator->connection;
                            finished.bytes = 0;
                            finished.error = EOK;
                            MinetSend(sock, finished);

                            conn_list.erase(CL_iterator);
                            cerr << "===============END CASE FIN_WAIT2 + IS_ACK===============\n" << endl;
                        }
                        break;
                    case CLOSE_WAIT:
                        cerr<<"ENTERED CLOSE WAIT."<<endl;
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

            /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
             * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
             * *                                     MUX                                   * *
             * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
             * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
            if (event.handle == sock)
            {
                cerr<<"I got a sock event."<<endl;

                SockRequestResponse request, response;
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
                            build_packet(recv_packet, new_CTSM, SYN, 0);

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
                            conn_list.push_back(new_CTSM);
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
                        case WRITE:
                        {
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
                        case CLOSE:
                        {
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
                else
                {
                    cerr<< "**********Connection was found in the list**********" << endl;
                    unsigned int curr_state; // State of the tcp connection on our side.
                    curr_state = CL_iterator->state.GetState();	// Get the state
                    Buffer data_buff;	// buffer for storing data
                    switch (request.type)
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
                        case FORWARD:
                        {
                            cerr<< "FORWARD"<<endl;
                        }
                            break;
                        case STATUS:
                        {
                            if (curr_state == ESTABLISHED)
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
                                if(CL_iterator->state.SendBuffer.GetSize()+request.data.GetSize() > CL_iterator->state.TCP_BUFFER_SIZE)
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
                                    Buffer to_copy= request.data;
                                    int ret_value = 0;

                                    ret_value = SendData(mux, sock, *CL_iterator, to_copy);

                                    if (ret_value == 0)
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
                            break;
                        case CLOSE:
                        {
                            cerr << "===============Starting to close the connection===============\n" << endl;
                            if (curr_state == ESTABLISHED)
                            {
                                CL_iterator->state.SetState(FIN_WAIT1);
                                CL_iterator->state.last_acked = CL_iterator->state.last_acked + 1;

                                Packet send_packet;
                                build_packet(send_packet, *CL_iterator, FIN_ACK, 0); // WE SEND FIRST FINACK.
                                MinetSend(mux, send_packet);

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

void build_packet(Packet &to_build, ConnectionToStateMapping<TCPState> &the_mapping, int TCPHeaderType, int size_of_data)
{
    cerr<< "+--------------BUILD PACKET-----------+" << endl;
    unsigned char alerts = 0;
    int packet_size = size_of_data + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH; // TODO changed to size of data...

    IPHeader new_ipheader;
    TCPHeader new_tcpheader;

    new_ipheader.SetSourceIP(the_mapping.connection.src);
    new_ipheader.SetDestIP(the_mapping.connection.dest);
    new_ipheader.SetTotalLength(packet_size);
    new_ipheader.SetProtocol(IP_PROTO_TCP);
    to_build.PushFrontHeader(new_ipheader);

    new_tcpheader.SetSourcePort(the_mapping.connection.srcport, to_build);
    new_tcpheader.SetDestPort(the_mapping.connection.destport, to_build);
    new_tcpheader.SetHeaderLen(TCP_HEADER_BASE_LENGTH, to_build);

    new_tcpheader.SetAckNum(the_mapping.state.GetLastRecvd(),to_build);
    new_tcpheader.SetWinSize(the_mapping.state.GetRwnd(), to_build);
    new_tcpheader.SetUrgentPtr(0, to_build);

    switch (TCPHeaderType)
    {
        case SYN:
        {
            SET_SYN(alerts);
        }
            break;
        case ACK:
        {
            SET_ACK(alerts);
        }
            break;
        case SYN_ACK:
        {
            SET_ACK(alerts);
            SET_SYN(alerts);
        }
            break;
        case PSHACK:
        {
            SET_PSH(alerts);
            SET_ACK(alerts);
        }
            break;
        case FIN:
        {
            SET_FIN(alerts);
        }
            break;
        case FIN_ACK:
        {
            SET_FIN(alerts);
            SET_ACK(alerts);
        }
            break;
        case RST:
        {
            SET_RST(alerts);
        }
            break;
        default:
        {
            break;
        }
    }

    // Set the flag
    new_tcpheader.SetFlags(alerts, to_build);

    new_tcpheader.SetSeqNum(the_mapping.state.GetLastSent(), to_build);

    new_tcpheader.RecomputeChecksum(to_build);

    // Push the header into the packet
    to_build.PushBackHeader(new_tcpheader);
    cerr<< "---------------BUILD PACKET------------" << endl;
}

int SendData(const MinetHandle &mux, const MinetHandle &sock, ConnectionToStateMapping<TCPState> &the_mapping, Buffer data_buffer)
{
    Packet data_packet;
    the_mapping.state.SendBuffer.AddBack(data_buffer);
    unsigned int bytes_to_send = data_buffer.GetSize();

    while(bytes_to_send!= 0)
    {
        unsigned int bytes_being_sent = min(bytes_to_send, TCP_MAXIMUM_SEGMENT_SIZE);
        data_packet = the_mapping.state.SendBuffer.Extract(0, bytes_being_sent);
        build_packet(data_packet, the_mapping, PSHACK, bytes_being_sent); // TODO we the last argument could be wrong...
        MinetSend(mux, data_packet);
        the_mapping.state.last_sent = the_mapping.state.last_sent + bytes_being_sent;
        bytes_to_send-= bytes_being_sent;
    }
    cerr << "DATA HAS BEEN SENT\n" << endl;
    return bytes_to_send;
}