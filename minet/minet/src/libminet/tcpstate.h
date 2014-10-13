/*******************************************************************************
 * Project part B
 *
 * Author: Jason Skicewicz
 *
 * TCPState class that performs low level TCP functionality
 *
 ******************************************************************************/

#ifndef _tcpstate
#define _tcpstate

#include <iostream>
#include "Minet.h"

// The following two constants provide the proper sequence increments for an MSL
//  of 2 mins
const unsigned int SECOND_MULTIPLIER=35791394; // (1s * 2^32) / 120s
const unsigned int MICROSEC_MULTIPLIER=36;     // (1us * 2^32) / 120s
const unsigned int MSL_TIME_SECS=120;          // MSL time is 2 minutes
const unsigned int NUM_SYN_TRIES=8;            // Send SYN 8x before fail (~80secs)
const unsigned int SEQ_LENGTH_MASK=0xFFFFFFFF; // Masks off first 32 bits

const unsigned int TCP_MAXIMUM_SEGMENT_SIZE=536;

const unsigned int NUM_TCP_STATES=13;

enum eState { CLOSED      = 0,
	      LISTEN      = 1,
	      SYN_RCVD    = 2,
	      SYN_SENT    = 3,
	      SYN_SENT1   = 4,
	      ESTABLISHED = 5,
	      SEND_DATA   = 6,
	      CLOSE_WAIT  = 7,
	      FIN_WAIT1   = 8,
	      CLOSING     = 9,
	      LAST_ACK    = 10,
	      FIN_WAIT2   = 11,
	      TIME_WAIT   = 12 };


class TCPState {
 public:
    unsigned int stateOfcnx;
    unsigned int tmrTries;

    // Sender side
    unsigned int last_acked, last_sent;
    unsigned short rwnd;
    Buffer SendBuffer;

    //Receiver side
    unsigned int last_recvd;
    Buffer RecvBuffer;

    unsigned int TCP_BUFFER_SIZE; //Buffer size of SendBuffer and RecvBuffer
    unsigned int N; //Window size

    TCPState();
    TCPState(unsigned int initialSequenceNum, unsigned int state, unsigned int timertries);
    virtual ~TCPState();

    inline unsigned int GetN()
    {
      return N;
    }

    inline void SetState(unsigned int newstate)
    {
      stateOfcnx = newstate;
    }

    inline unsigned int GetState()
    {
      return stateOfcnx;
    }

    inline void SetTimerTries(unsigned int numtries)
    {
      tmrTries = numtries;
    }

    bool ExpireTimerTries();

    bool SetLastAcked(unsigned int newack);
    inline unsigned int GetLastAcked()
    {
      return last_acked;
    }

    inline void SetLastSent(unsigned int lastsent)
    {
      last_sent = lastsent;
    }

    inline unsigned int GetLastSent()
    {
      return last_sent;
    }

    inline void SetSendRwnd(unsigned short window)
    {
      rwnd = window;
    }

    unsigned int GetRwnd();

    void SetLastRecvd(unsigned int lastrecvd);
    bool SetLastRecvd(unsigned int lastrecvd, unsigned int length);
    inline unsigned int GetLastRecvd()
    {
      return last_recvd;
    }

    void SendPacketPayload(unsigned &offsetlastsent, size_t &bytesize, unsigned bytes);

    std::ostream & Print(std::ostream &os) const { os <<"TCPState(stateOfcnx=" << stateOfcnx
					  << ", last_acked=" << last_acked
					  << ", last_sent=" << last_sent
					  << ", rwnd=" << rwnd
					  << ", last_recvd=" << last_recvd
					  << ")"; return os;}

   friend std::ostream &operator<<(std::ostream &os, const TCPState& L) {
    return L.Print(os);
  }
};
#endif
