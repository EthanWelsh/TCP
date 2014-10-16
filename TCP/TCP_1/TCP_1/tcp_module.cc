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

    while (MinetGetNextEvent(event, timeout) == 0)
    {
        if ((event.eventtype == MinetEvent::Dataflow) && (event.direction == MinetEvent::IN))
        {
            if (event.handle == sock)
            {
                // socket request or response has arrived
                SockRequestResponse req;	// Hold the request
                SockRequestResponse reply;	// Hold the response

                MinetReceive(sock, req);
                Packet envelope;
                // Check to see if the connection exists
                ConnectionList<TCPState>::iterator check_exists= clist.FindMatching(req.connection);
                if(check_exists == clist.end())
                {
                    switch (req.type)
                    {
                        case CONNECT:
                        {
                            cerr<< "Working on the connection\n" <<endl;
                            // Build a state -- initialSequenceNum, state, timertries
                            TCPState curr= TCPState(1, 0, 3);
                            // Link connection and curr state
                            ConnectionToStateMapping<TCPState> c_mapping;
                            c_mapping.connection= req.connection;	// Update map connection
                            c_mapping.timeout = Time()+2;	// Update map timeout value
                            c_mapping.state= curr;	// Update map state
                            c_mapping.bTmrActive= true;

                            clist.push_back(c_mapping); // Push the mapping on the back of the Connection List

                            build_packet(envelope, c_mapping, SYN_SENT, 0, false);	// Make the packet
                            // Send the packet twice
                            MinetSend(mux, envelope);
                            MinetSend(mux, envelope);

                            reply.type = STATUS;
                            reply.connection = req.connection;
                            reply.bytes = 0;
                            reply.error = EOK;
                            MinetSend(sock, reply);
                        }
                            break;
                        case ACCEPT:
                        { /*
							reply.type = STATUS;
							reply.connection = req.connection;
							// buffer is zero bytes
							reply.bytes = 0;
							reply.error = EOK;

							// **** CREATE SOCKET ****
							//ConnectionToStateMapping <TCPDriver> m = new ConnectionToStateMapping<TCPDriver>();
							//m.connection = req.connection;


							TCPDriver tstate = new TCPDriver(req.connection.src, req.connection.dest, req.connection.srcport, LISTEN);

							Connection connect = new Connection();

							connect.src = MyIPAddr();
							connect.dest = IPAddress(req.connection.src);
							connect.protocol = IP_PROTO_TCP;
							connect.srcport = req.connection.srcport;
							connect.destport = req.connection.destport; // TODO should this be 0?

							ConnectionToStateMapping <TCPDriver> m = new ConnectionToStateMapping<TCPDriver>();
							m.connection = connect;
							m.state = tstate;
							clist.push_back(m);

							// ***********************


							// **** CREATE


							MinetSend(sock, reply);*/
                            break;
                        }

                        case STATUS:
                        {
                            cout<<"status: "<<endl;
                        }
                        case WRITE:
                        {
                            break;
                        }
                        case FORWARD:
                        {
                            break;
                        }
                        case CLOSE:
                        {
                            break;
                        }
                        default:
                        {
                            SockRequestResponse repl;
                            // repl.type=SockRequestResponse::STATUS;
                            repl.type = STATUS;
                            repl.error = EWHAT;
                            MinetSend(sock, repl);
                        }
                    }
                }
            }
            if (event.handle == mux)
            {
                // ip packet has arrived!
                cerr<< "---------------In the mux portion------------" << endl;

                Packet mux_packet;

                MinetReceive(mux, mux_packet);	// Receive packet



                unsigned char alerts = 0; //f stands for flags.
                unsigned char f;

                unsigned int status;

                unsigned short length = TCPHeader::EstimateTCPHeaderLength(mux_packet);
                mux_packet.ExtractHeaderFromPayload<TCPHeader>(length);




                mux_packet.ExtractHeaderFromPayload<TCPHeader>(8);	// Extract Header from packet
                TCPHeader tcp_header;
                tcp_header = mux_packet.FindHeader(Headers::TCPHeader);


                IPHeader ip_header;
                ip_header = mux_packet.FindHeader(Headers::IPHeader);

                Connection c;
                ip_header.GetDestIP(c.src);
                ip_header.GetSourceIP(c.dest);
                ip_header.GetProtocol(c.protocol);
                tcp_header.GetDestPort(c.srcport);
                tcp_header.GetSourcePort(c.destport);
                tcp_header.GetFlags(f); //	Assign f with flags received from TCP Header

                // Iterate through looking for this connection in the list
                ConnectionList<TCPState>::iterator cs = clist.FindMatching(c);
                if (cs != clist.end())
                {
                    //tcp_header.GetHeaderLength(length);	// Get the length of the header TODO I MESSED WITH THIS A LOT
                    //length -= TCP_HEADER_LENGTH; // find the start of the data segment

                    //SockRequestResponse reply;


                    //SockRequestResponse write(WRITE, (*cs).connection, data, length, EOK);

                    //Buffer empty;
                    //reply.data = empty;
                    //reply.connection = c;
                    //reply.error = EOK;

                    printf("We're here\n");
                    //MinetSend(sock, reply);


                    //MinetSend(sock, write);
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