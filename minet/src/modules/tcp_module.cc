/*
	Joshua Miner (jmm299)
	Ethan Welsh	(ejw45)
	CS1652 Project 2
	tcp_module.cc
*/

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

#include "tcpstate.h"
#include "Minet.h"

using namespace std;

static const int SYN = 1;
static const int ACK = 2;
static const int SYN_ACK = 3;
static const int PSH_ACK = 4;
static const int FIN = 5;
static const int FIN_ACK = 6;
static const int RST = 7;

struct TCPState
{
    // need to write this
    std::ostream &Print(std::ostream &os) const
    {
        os << "TCPState()";
        return os;
    }
};

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

    cerr << "tcp_module STUB VERSION handling tcp traffic.......\n";

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
							c_mapping.connection= req.connection;
							c_mapping.timeout = Time()+2;
							c_mapping.state= curr;
							c_mapping.bTmrActive= true;
						}
						break;
						case ACCEPT:
						{ 
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


							MinetSend(sock, reply);
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

                    /*
                     * then within the (event.handle == mux) triggers whenever you receive a packet
                     *
                     * I check my ConnecitonList clist if I have the connection (which I should)
                     *
                     * extract the state.  check to see which state it is in using a switch statement (i.e. LISTEN state)
                     *
                     * Then I respond to the handshake (parse the packet using TCPHeader and IPHeader classes.  similar
                     *  to how udp_module does it)
                     *
                     * check to make sure its a SYN packet
                     *
                     * if it is, I construct my own SYN-ACK packet
                     *
                     * then I use MinetSent() to send it to the socket to forward along
                     *
                     * so as far as I understand it, you have two main tasks.  1 if statement that handles all socket
                     * changes (connect, accept, close, etc.)  and one if statement that handles packets (mux)
                     *
                     *
                     */


                    Packet p;
                    unsigned short len = TCPHeader::EstimateTCPHeaderLength(p);

                    bool checksumok;
                    MinetReceive(mux, p);

                    p.ExtractHeaderFromPayload<TCPHeader>(8);

                    TCPHeader tcph;
                    tcph = p.FindHeader(Headers::TCPHeader);

                    checksumok = tcph.IsCorrectChecksum(p);

                    IPHeader iph;
                    iph = p.FindHeader(Headers::IPHeader);

                    Connection c;
                    // note that this is flipped around because
                    // "source" is interepreted as "this machine"
                    iph.GetDestIP(c.src);
                    iph.GetSourceIP(c.dest);
                    iph.GetProtocol(c.protocol);
                    tcph.GetDestPort(c.srcport);
                    tcph.GetSourcePort(c.destport);
                    ConnectionList<TCPState>::iterator cs = clist.FindMatching(c);

                    if (cs != clist.end())
                    {
                        tcph.GetLength(len);
                        len -= TCP_HEADER_LENGTH;
                        Buffer &data = p.GetPayload().ExtractFront(len);
                        SockRequestResponse write(WRITE, (*cs).connection, data, len, EOK);

                        if (!checksumok)
                        {
                            MinetSendToMonitor(MinetMonitoringEvent("forwarding packet to sock even though checksum failed"));
                        }
                        MinetSend(sock, write);
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
