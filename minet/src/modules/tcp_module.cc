#include "tcpstate.h"
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

#include <iostream>

#include "Minet.h"

using namespace std;
/* ***************************
 * Don't need this in here. Defined in tcpstate.h. Also it's a CLASS in tcpstate.h
 struct TCPState {};
 * *************************** */

// Could do this with DEFINE, but this might be better in the end. TOFIX?
// For use with ForgePacket
static const int HEADERTYPE_SYN = 1;
static const int HEADERTYPE_ACK = 2;
static const int HEADERTYPE_SYNACK = 3;
static const int HEADERTYPE_PSHACK = 4;
static const int HEADERTYPE_FIN = 5;
static const int HEADERTYPE_FINACK = 6;
static const int HEADERTYPE_RST = 7;


/* ***************************
 * void ForgePacket
 * Arguments:
 * Packet &packet_tomake - This is the packet that is being forged
 * ConnectionToStateMapping<TCPState> &theCTSM - This is the CTSM for the packet
 * int TCPHeaderType - This is the type of header
 * int size_of_data
 * bool isTimeout
 * 
 * Returns: N/A
 * 
 * Use:
 * Forges a packet based upon the conditions laid out by the arguments.
 * Takes most of the info from the TCPState and Connection inside the CTSM.
 * *************************** */
void ForgePacket(Packet &packet_tomake, ConnectionToStateMapping<TCPState> &theCTSM, int TCPHeaderType, int size_of_data, bool isTimeout) {
        cerr << "===============Forging a packet!===============" << endl;
        unsigned char flags = 0;
        int packet_size = size_of_data + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH;
        IPHeader ipheader_new;
        TCPHeader tcpheader_new;
        
        // Forge the IP header
        ipheader_new.SetSourceIP(theCTSM.connection.src);
        ipheader_new.SetDestIP(theCTSM.connection.dest);
        ipheader_new.SetTotalLength(packet_size);
        ipheader_new.SetProtocol(IP_PROTO_TCP);
        packet_tomake.PushFrontHeader(ipheader_new);
        cerr << "\nipheader_new: \n" << ipheader_new << endl;
        
        // Forge the TCP header
        tcpheader_new.SetSourcePort(theCTSM.connection.srcport, packet_tomake);
        tcpheader_new.SetDestPort(theCTSM.connection.destport, packet_tomake);
        tcpheader_new.SetHeaderLen(TCP_HEADER_BASE_LENGTH, packet_tomake);
        
        tcpheader_new.SetAckNum(theCTSM.state.GetLastRecvd(), packet_tomake);
        tcpheader_new.SetWinSize(theCTSM.state.GetRwnd(), packet_tomake);
        tcpheader_new.SetUrgentPtr(0, packet_tomake);
        
        // Determine the flag type
        switch (TCPHeaderType) {
                case HEADERTYPE_SYN:
                        SET_SYN(flags);
                        cerr << "It is a HEADERTYPE_SYN!" << endl;
                        break;

                case HEADERTYPE_ACK:
                        SET_ACK(flags);
                        cerr << "It is a HEADERTYPE_ACK!" << endl;
                        break;

                case HEADERTYPE_SYNACK:
                        SET_ACK(flags);
                        SET_SYN(flags);
                        cerr << "It is a HEADERTYPE_SYNACK!" << endl;
                        break;
                
                case HEADERTYPE_PSHACK:
                        SET_PSH(flags);
                        SET_ACK(flags);
                        cerr << "It is a HEADERTYPE_PSHACK!" << endl;
                        break;

                case HEADERTYPE_FIN:
                        SET_FIN(flags);
                        cerr << "It is a HEADERTYPE_FIN!" << endl;
                        break;

                case HEADERTYPE_FINACK:
                        SET_FIN(flags);
                        SET_ACK(flags);
                        cerr << "It is a HEADERTYPE_FINACK!" << endl;
                        break;

                case HEADERTYPE_RST:
                        SET_RST(flags);
                        cerr << "It is a HEADERTYPE_RST!" << endl;
                        break;
                
                default:
                        break;
        }
        
        // Set the flag type
        tcpheader_new.SetFlags(flags, packet_tomake);
        
        // Print out the finished TCP header
        cerr << "\ntcpheader_new: \n" << tcpheader_new << endl;
        
        // Determine if it is a packet related to a timeout
        if (isTimeout) { // If this is a packet responding to a timeout scenario
                tcpheader_new.SetSeqNum(theCTSM.state.GetLastSent()+1, packet_tomake);
        }
        else { // If this is NOT a packet responding to a timeout scenario
                //tcpheader_new.SetSeqNum(theCTSM.state.GetLastAcked()+1, packet_tomake);
                tcpheader_new.SetSeqNum(theCTSM.state.GetLastSent(), packet_tomake);
        }
        
        // Use instead of SetChecksum to set to zero first
        tcpheader_new.RecomputeChecksum(packet_tomake);
        
        // Push the header into the packet
        packet_tomake.PushBackHeader(tcpheader_new);
        
        // Done forging the packet
        cerr << "===============End forging!===============\n" << endl;
}


int SendData(const MinetHandle &mux, const MinetHandle &sock, ConnectionToStateMapping<TCPState> &theCTSM, Buffer data) {
                                        /*CL_iterator->state.SendBuffer.AddBack(Buffer(data.c_str(), data.length()));
                                        packet_tosend = CL_iterator->state.SendBuffer;
                                        ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_PSHACK, data.length(), false);
                                        MinetSend(mux, packet_tosend);*/
        cerr << "===============Start SendData!===============\n" << endl;
                Packet p;
        theCTSM.state.SendBuffer.AddBack(data);
        unsigned int bytesLeft = data.GetSize();
        //theCTSM.state.SetLastSent(theCTSM.state.GetLastAcked());
                //theCTSM.state.last_sent = theCTSM.state.last_sent + 1;
        while(bytesLeft != 0) {
                unsigned int bytesToSend = min(bytesLeft, TCP_MAXIMUM_SEGMENT_SIZE);
                p = theCTSM.state.SendBuffer.Extract(0, bytesToSend);
                ForgePacket (p, theCTSM, HEADERTYPE_PSHACK, bytesToSend, false);
                MinetSend(mux, p);
                
                //theCTSM.state.SetLastSent(theCTSM.state.GetLastSent()+bytesToSend);
                theCTSM.state.last_sent = theCTSM.state.last_sent + bytesToSend;
                                
                bytesLeft -= bytesToSend;
        }
        cerr << "===============End SendData!===============\n" << endl;
        return bytesLeft;
}









/* ***************************
 * void mux_event_handler
 * Arguments:
 * const MinetHandle &mux - The mux from the MinetEvent (in main)
 * const MinetHandle &sock - The socket from the MinetEvent (in main)
 * ConnectionList<TCPState> &clist - The "global" clist from main
 * 
 * Returns: N/A
 * 
 * Use:
 * Handles an incoming IP packet. This deals with the mux MinetEvent.
 * Deconstructs the IP packet into the IP header, TCP header, and data
 * *************************** */
void mux_event_handler(const MinetHandle &mux, const MinetHandle &sock, ConnectionList<TCPState> &clist) {
        cerr << "\n===============Mux Handler Starting===============\n" << endl;
        Packet packet_received;
        Buffer buffer;
        MinetReceive(mux, packet_received);
        Connection conn_tbd;
        
        Packet packet_tosend;
        TCPHeader tcpheader_recv;
        IPHeader ipheader_recv;
        
        unsigned char flags;
        unsigned int ack;
        unsigned int seq;
        SockRequestResponse repl, req;
        unsigned short total_size;
        unsigned char tcpheader_size;
        unsigned char ipheader_size;
        unsigned short window_size, isUrgentPacket;
        
        // Extract the headers
        packet_received.ExtractHeaderFromPayload<TCPHeader>(TCPHeader::EstimateTCPHeaderLength(packet_received));
        tcpheader_recv = packet_received.FindHeader(Headers::TCPHeader);
        ipheader_recv = packet_received.FindHeader(Headers::IPHeader);
        
        bool checksum = tcpheader_recv.IsCorrectChecksum(packet_received);
        cerr << tcpheader_recv << endl;
        if (!checksum) {
                cerr << "Incorrect checksum! But apparently that doesn't matter.... (<.<) \n";
                return;
        }
        
        cerr << "\nipheader: \n" << ipheader_recv << endl;
        cerr << "\ntcpheader: \n" << tcpheader_recv << endl;
        // The following are needed for identifying the connection (tuple of 5 values)
        cerr << "Note: some values are reversed because that way its easier to craft a packet" << endl;
        ipheader_recv.GetDestIP(conn_tbd.src);
        ipheader_recv.GetSourceIP(conn_tbd.dest);
        ipheader_recv.GetProtocol(conn_tbd.protocol);
        tcpheader_recv.GetSourcePort(conn_tbd.destport);
        tcpheader_recv.GetDestPort(conn_tbd.srcport);
        cerr << "Printing the connection received:\n" << conn_tbd << endl;
        
        // Get the flags from the TCP header
        ipheader_recv.GetFlags(flags);
        cerr << "flags: " << flags << endl;
        
        tcpheader_recv.GetSeqNum(seq);
        tcpheader_recv.GetAckNum(ack);
        tcpheader_recv.GetFlags(flags);
        tcpheader_recv.GetWinSize(window_size); 
        tcpheader_recv.GetUrgentPtr(isUrgentPacket);
        tcpheader_recv.GetHeaderLen(tcpheader_size);
        
        ipheader_recv.GetTotalLength(total_size);
        ipheader_recv.GetHeaderLength(ipheader_size);
        
        total_size = total_size - tcpheader_size - ipheader_size;
        cerr << "ipheader_recv len = " << total_size << endl;
        buffer = packet_received.GetPayload().ExtractFront(total_size);
        
        ConnectionList<TCPState>::iterator CL_iterator = clist.FindMatching(conn_tbd);
        
        unsigned int current_state; // This is the current state of the connection
        
        if (CL_iterator == clist.end()) {
                cerr << "Didn't find the connection in the list!" << endl;
        }
        current_state = CL_iterator->state.GetState();
        
        cerr << "current_state: " << current_state << endl;
        
        switch(current_state){
                case LISTEN:
                        //
                        cerr << "===============START CASE LISTEN===============\n" << endl;
                        if (IS_SYN(flags)) {
                                cerr << "===============START IS_SYN===============" << endl;
                                // Update all the data in the CTSM
                                CL_iterator->connection = conn_tbd; // Set conn_tbd to connection
                                CL_iterator->state.SetState(SYN_RCVD);
                                CL_iterator->state.last_acked = CL_iterator->state.last_sent;
                                CL_iterator->state.SetLastRecvd(seq + 1);
                                CL_iterator->bTmrActive = true; // Timeout set
                                CL_iterator->timeout=Time() + 5; // Set to 5 seconds
                                cerr << "\nseq: " << seq << " and ack: " << ack << endl;
                                
                                // Forge the SYNACK packet
                                CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
                                
                                ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_SYNACK, 0, false);
                                
                                MinetSend(mux, packet_tosend);
                                sleep(2);
                                MinetSend(mux, packet_tosend);
                                
                                cerr << "===============END IS_SYN===============" << endl;
                        }
                        
                        cerr << "===============END CASE LISTEN===============\n" << endl;
                        break;
                case SYN_RCVD:
                        // If you are in this state, you should have received the SYN packet
                        // and thus sent a SYNACK and be expecting an ACK back
                        cerr << "===============START CASE SYN_RCVD===============\n" << endl;
                        CL_iterator->state.SetState(ESTABLISHED);
                        cerr << "Established the connection!" << endl;
                        CL_iterator->state.SetLastAcked(ack);
                        CL_iterator->state.SetSendRwnd(window_size); 
                        CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
                        CL_iterator->bTmrActive = false;
                        static SockRequestResponse * write = NULL;
                        write = new SockRequestResponse(WRITE, CL_iterator->connection, buffer, 0, EOK);
                        MinetSend(sock, *write);
                        delete write;
                        
                        cerr << "===============END CASE SYN_RCVD===============\n" << endl;
                        break;
                case SYN_SENT:
                        // If you are in this state, you sent an SYN packet and should
                        // now receive a SYNACK back
                        cerr << "===============START CASE SYN_SENT===============\n" << endl;
                        if (IS_SYN(flags) && IS_ACK(flags)) {
                                cerr << "===============START IS_SYN + IS_ACK===============" << endl;
                                CL_iterator->state.SetSendRwnd(window_size);
                                CL_iterator->state.SetLastRecvd(seq + 1);
                                CL_iterator->state.last_acked = ack;
                                // Need to make an ACK packet
                                CL_iterator->state.last_sent = CL_iterator->state.last_sent + 1;
                                ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
                                MinetSend(mux, packet_tosend);
                                CL_iterator->state.SetState(ESTABLISHED);
                                CL_iterator->bTmrActive = false;
                                SockRequestResponse write (WRITE, CL_iterator->connection, buffer, 0, EOK);
                                MinetSend(sock, write);
                                cerr << "===============END IS_SYN + IS_ACK===============" << endl;
                        }
                        cerr << "===============END CASE SYN_SENT===============\n" << endl;
                        break;
                case ESTABLISHED:
                        cerr << "===============START CASE ESTABLISHED===============\n" << endl;
                        
                        // stuff being received from a client
                        if(IS_PSH(flags) && IS_ACK(flags)) {
                                        cerr << "===============START CASE IS_PSH + IS_ACK===============\n" << endl;
                                        cerr << "Received \"" << buffer << "\", buffer size: " << buffer.GetSize() << "." << endl;
                                        CL_iterator->state.SetSendRwnd(window_size);
                                        //CL_iterator->state.SetLastAcked(CL_iterator->state.last_acked+buffer.GetSize()+1);
                                        CL_iterator->state.last_recvd = seq+buffer.GetSize();
                                        CL_iterator->state.last_acked = ack;
                                        //CL_iterator->state.SetLastRecvd(seq+1);
                                        // Write to socket
                                        CL_iterator->state.RecvBuffer.AddBack(buffer);           
                                        SockRequestResponse write (WRITE, CL_iterator->connection, CL_iterator->state.RecvBuffer, CL_iterator->state.RecvBuffer.GetSize(), EOK);
                                        MinetSend(sock,write);
                                        
                                        // Need to make an ACK packet
                                        ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
                                        MinetSend(mux, packet_tosend);
                                        cerr << "===============END CASE IS_PSH + IS_ACK===============\n" << endl;
                        }
                        // we are the server, the client wants to close the connection
                        else if(IS_FIN(flags) && IS_ACK(flags)) {
                                        cerr << "===============START CASE IS_FIN + IS_ACK===============\n" << endl;
                                        CL_iterator->state.SetState(CLOSE_WAIT);
                                        CL_iterator->state.SetSendRwnd(window_size);
                                        //CL_iterator->state.SetLastAcked(CL_iterator->state.last_acked+buffer.GetSize()+1);
                                        CL_iterator->state.last_recvd = seq+1;
                                        CL_iterator->state.last_acked = ack;
                                        //CL_iterator->state.SetLastRecvd(seq+1);        
                                        
                                        // Need to make an ACK packet
                                        ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_FINACK, 0, false);
                                        MinetSend(mux, packet_tosend);
                                        cerr << "===============END CASE IS_FIN + IS_ACK===============\n" << endl;
                        }
                        // we are the client, the server is ACK'ing our packet
                        else if(IS_ACK(flags)) {
                                        cerr << "Received an ACK for our last packet" << endl;
                                        CL_iterator->state.SetLastRecvd((unsigned int)seq);
                                        CL_iterator->state.last_acked = ack;
                        }
                        else {
                                        cerr << "Unknown packet: " << tcpheader_recv << endl;
                        }
                        cerr << "===============END CASE ESTABLISHED===============\n" << endl;
                        break;
                // waiting for FIN/ACK in response to our FIN/ACK
                case FIN_WAIT1:
                        // got FIN/ACK from server
                        if(IS_FIN(flags) && IS_ACK(flags)) {
                                        cerr << "===============START CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
                                        CL_iterator->state.SetState(FIN_WAIT2);
                                        CL_iterator->state.SetSendRwnd(window_size);
                                        //CL_iterator->state.SetLastAcked(CL_iterator->state.last_acked+buffer.GetSize()+1);
                                        CL_iterator->state.last_recvd = seq+1;
                                        CL_iterator->state.last_acked = ack;
                                        //CL_iterator->state.SetLastRecvd(seq+1);        
                                        
                                        // Need to make an ACK packet
                                        ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
                                        MinetSend(mux, packet_tosend);
                                        cerr << "===============END CASE FIN_WAIT1 + IS_FIN + IS_ACK===============\n" << endl;
                        }
                        break;
                // waiting for ACK to close the connection
                case FIN_WAIT2:
                        if(IS_ACK(flags)) {
                                        cerr << "===============START CASE FIN_WAIT2 + IS_FIN + IS_ACK===============\n" << endl;
                                        CL_iterator->state.SetSendRwnd(window_size);
                                        //CL_iterator->state.SetLastAcked(CL_iterator->state.last_acked+buffer.GetSize()+1);
                                        CL_iterator->state.last_recvd = seq+1;
                                        CL_iterator->state.last_acked = ack;
                                        //CL_iterator->state.SetLastRecvd(seq+1);        
                                        
                                        // Need to make an ACK packet
                                        ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_ACK, 0, false);
                                        MinetSend(mux, packet_tosend);
                                        
                                        // bye bye
                                                SockRequestResponse close;
                                                close.type = CLOSE;
                                                close.connection = CL_iterator->connection;
                                                close.bytes = 0;
                                                close.error = EOK;
                                                MinetSend(sock, close);
                                        
                                        clist.erase(CL_iterator);
                                        cerr << "===============END CASE FIN_WAIT2 + IS_FIN + IS_ACK===============\n" << endl;
                        }
                        break;
                case TIME_WAIT:
                        break;
                // waiting for an ACK for our FIN/ACK
                case CLOSE_WAIT:
                        if(IS_ACK(flags)) {
                                        cerr << "===============START CASE CLOSE_WAIT + IS_ACK===============\n" << endl;
                                        // bye bye
                                                SockRequestResponse close;
                                                close.type = CLOSE;
                                                close.connection = CL_iterator->connection;
                                                close.bytes = 0;
                                                close.error = EOK;
                                                MinetSend(sock, close);
                                        clist.erase(CL_iterator);
                                        cerr << "Connection closed!" << endl;
                                        cerr << "===============END CASE CLOSE_WAIT + IS_ACK===============\n" << endl;
                        }
                        break;
                case LAST_ACK:
                        break;
        
                return;
        }
        cerr << "\n===============Mux Handler Ending===============\n" << endl;
}






/* ***************************
 * void socket_event_handler
 * Arguments:
 * const MinetHandle &mux - The mux from the MinetEvent (in main)
 * const MinetHandle &sock - The socket from the MinetEvent (in main)
 * ConnectionList<TCPState> &clist - The "global" clist from main
 * 
 * Returns: N/A
 * 
 * Use:
 * Handles an incoming IP packet. This deals with the mux MinetEvent.
 * Deconstructs the IP packet into the IP header, TCP header, and data
 * *************************** */
void socket_event_handler(const MinetHandle &mux, const MinetHandle &sock, ConnectionList<TCPState> &clist) {
        SockRequestResponse req;
        SockRequestResponse repl;
        MinetReceive(sock, req);
        Packet packet_tosend;
        
        // Check to see if there is a matching connection in the ConnectionList
        ConnectionList<TCPState>::iterator CL_iterator = clist.FindMatching(req.connection);
        
        if (CL_iterator == clist.end()) { // Connection does not exist, thus it probably wants a new connection
                cerr << "Didn't find the connection in the list!" << endl;
                
                switch (req.type) {
                        case CONNECT: {
                                cerr << "===============START CASE CONNECT===============\n" << endl;
                                
                                TCPState client(1, SYN_SENT, 5);
                                
                                ConnectionToStateMapping<TCPState> new_CTSM(req.connection, Time()+2, client, true);
                                //client.SetLastSent(client.GetLastSent()+1);
                                clist.push_back(new_CTSM);
                                
                                ForgePacket(packet_tosend, new_CTSM, HEADERTYPE_SYN, 0, false);
                                
                                MinetSend(mux, packet_tosend);
                                sleep(2);
                                MinetSend(mux, packet_tosend);
                                cerr << clist << endl;
                                
                                repl.type = STATUS;
                                repl.connection = req.connection;
                                repl.bytes = 0;
                                repl.error = EOK;
                                MinetSend(sock, repl);
                                
                                cerr << "===============END CASE CONNECT===============" << endl;
                                }
                                break;
                        case ACCEPT: {
                                cerr << "===============START CASE ACCEPT===============\n" << endl;
                                TCPState server(1, LISTEN, 5);
                                ConnectionToStateMapping<TCPState> new_CTSM(req.connection, Time(), server, false);
                                clist.push_back(new_CTSM);
                                repl.type = STATUS;
                                repl.connection = req.connection;
                                repl.bytes = 0;
                                repl.error = EOK;
                                MinetSend(sock, repl);
                                cerr << "===============END CASE ACCEPT===============" << endl;
                                }
                                break;
                        case STATUS: {
                                }
                                break;
                        case WRITE: {
                                repl.type = STATUS;
                                repl.connection = req.connection;
                                repl.bytes = 0;
                                repl.error = ENOMATCH;
                                MinetSend(sock, repl);
                                }
                                break;
                        case FORWARD: {
                                }
                                break;
                        case CLOSE: {
                                repl.type = STATUS;
                                repl.connection = req.connection;
                                repl.bytes = 0;
                                repl.error = ENOMATCH;
                                MinetSend(sock, repl);
                                }
                                break;
                        default: {
                                }
                                break;
                }
        }
        else { // It found an existing connection in the list!
                cerr << "Found an existing connection in the list!" << endl;
                unsigned int my_state; // This is the current state of the tcp connection on our end.
                my_state = CL_iterator->state.GetState();
                Buffer buff;
                
                switch (req.type) {
                        case CONNECT: {
                                }
                                break;
                        case ACCEPT: { 
                                }
                                break;
                        case STATUS: {
                                if (my_state == ESTABLISHED) {
                                        unsigned datasend = req.bytes; // number of bytes send
                                        CL_iterator->state.RecvBuffer.Erase(0,datasend);
                                        if(0 != CL_iterator->state.RecvBuffer.GetSize()) { // Didn't finish writing
                                                SockRequestResponse write (WRITE, CL_iterator->connection, CL_iterator->state.RecvBuffer, CL_iterator->state.RecvBuffer.GetSize(), EOK);
                                                MinetSend(sock, write);
                                        }
                                }
                                }
                                break;
                        case WRITE: {
                                cerr << "===============START CASE WRITE===============\n" << endl;
                                if (my_state == ESTABLISHED) {
                                        if(CL_iterator->state.SendBuffer.GetSize()+req.data.GetSize() > CL_iterator->state.TCP_BUFFER_SIZE) {
                                                // If there isn't enough space in the buffer
                                                repl.type = STATUS;
                                                repl.connection = req.connection;
                                                repl.bytes = 0;
                                                repl.error = EBUF_SPACE;
                                                MinetSend(sock, repl);
                                                
                                        }
                                        else {
                                                // If there is enough space in the buffer
                                                //CL_iterator->state.SendBuffer.AddBack(req.data); // Push data to the back of the state
                                        
                                        
                                        Buffer copy_buffer = req.data; // Dupe the buffer
                                        
                                        
                                        int return_value = SendData(mux, sock, *CL_iterator, copy_buffer);
                                        
                                        if (return_value == 0) {
                                                repl.type = STATUS;
                                                repl.connection = req.connection;
                                                repl.bytes = copy_buffer.GetSize();
                                                repl.error = EOK;
                                                MinetSend(sock, repl);
                                        }
                                        }
                                }
                                cerr << "===============END CASE WRITE===============\n" << endl;
                                }
                                break;
                        case CLOSE: {
                                cerr << "===============START CASE CLOSE===============\n" << endl;
                                if (my_state == ESTABLISHED) {
                                
                                        CL_iterator->state.SetState(FIN_WAIT1);
                                        //CL_iterator->state.SetLastAcked(CL_iterator->state.last_acked+buffer.GetSize()+1);
                                        //CL_iterator->state.last_recvd = CL_iterator->state.last_recvd+1;
                                        CL_iterator->state.last_acked = CL_iterator->state.last_acked+1;
                                        //CL_iterator->state.SetLastRecvd(seq+1);      
                                        //SockRequestResponse close (CLOSE, CL_iterator->connection, 0, 0, EOK);
                                        //MinetSend(sock,close);
                                        
                                        // Need to make an ACK packet
                                        ForgePacket (packet_tosend, *CL_iterator, HEADERTYPE_FINACK, 0, false);
                                        MinetSend(mux, packet_tosend);
                                
                                        repl.type = STATUS;
                                        repl.connection = req.connection;
                                        repl.bytes = 0;
                                        repl.error = EOK;
                                        MinetSend(sock, repl);
                                }
                                cerr << "===============END CASE CLOSE===============" << endl;
                        }
                        default:
                                break;
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                }
        }
}







int main(int argc, char * argv[]) {
    MinetHandle mux;
    MinetHandle sock;
    
    ConnectionList<TCPState> clist;
        
    MinetInit(MINET_TCP_MODULE);

    mux = MinetIsModuleInConfig(MINET_IP_MUX) ?  
        MinetConnect(MINET_IP_MUX) : 
        MINET_NOHANDLE;
    
    sock = MinetIsModuleInConfig(MINET_SOCK_MODULE) ? 
        MinetAccept(MINET_SOCK_MODULE) : 
        MINET_NOHANDLE;

    if ( (mux == MINET_NOHANDLE) && 
         (MinetIsModuleInConfig(MINET_IP_MUX)) ) {

        MinetSendToMonitor(MinetMonitoringEvent("Can't connect to ip_mux"));

        return -1;
    }

    if ( (sock == MINET_NOHANDLE) && 
         (MinetIsModuleInConfig(MINET_SOCK_MODULE)) ) {

        MinetSendToMonitor(MinetMonitoringEvent("Can't accept from sock_module"));

        return -1;
    }
    
    cerr << "tcp_module STUB VERSION handling tcp traffic.......\n";

    MinetSendToMonitor(MinetMonitoringEvent("tcp_module STUB VERSION handling tcp traffic........"));

    MinetEvent event;
    double timeout = 1;
        

    while (MinetGetNextEvent(event, timeout) == 0) {
                if ((event.eventtype == MinetEvent::Dataflow) && 
                        (event.direction == MinetEvent::IN)) {
                
                        if (event.handle == mux) {
                                // ip packet has arrived!
                                // Handle it inside of the mux_event_handler
                                mux_event_handler(mux, sock, clist);
                        }

                        if (event.handle == sock) {
                                // socket request or response has arrived
                                MinetSendToMonitor(MinetMonitoringEvent("tcp_module socket request or response has arrived"));
                                cerr << "tcp_module socket request or response has arrived\n";
                                socket_event_handler(mux, sock, clist);
                        }
                }

                if (event.eventtype == MinetEvent::Timeout) {
                        // timeout ! probably need to resend some packets
                }
                
    }

    MinetDeinit();

    return 0;
}
