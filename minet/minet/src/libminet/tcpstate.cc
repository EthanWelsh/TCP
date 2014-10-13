/*******************************************************************************
 * Project part B
 *
 * Author: Jason Skicewicz
 *
 ******************************************************************************/

#include "tcpstate.h"

TCPState::TCPState()
{}

// Passive/Active open constructor
TCPState::TCPState(unsigned int initialSequenceNum, unsigned int state, unsigned int timertries)
{
  stateOfcnx = state;
  tmrTries = timertries;
  TCP_BUFFER_SIZE = 100*TCP_MAXIMUM_SEGMENT_SIZE; //100 packets in buffer
  N = 16*TCP_MAXIMUM_SEGMENT_SIZE; //16 packets allowed in flight

  // Send side initialization
  last_acked = initialSequenceNum;
  last_sent  = last_acked;
  rwnd       = 0;

  // Receiver side initialization
  last_recvd = 0;
}


TCPState::~TCPState()
{
}

bool TCPState::ExpireTimerTries()
{
  if (tmrTries == 0) {
    return true;
  } else {
    tmrTries--;
    return false;
  }
}

bool TCPState::SetLastAcked(unsigned int newack)
{
  if(last_acked <= last_sent) {
    if(newack > last_acked && newack <= last_sent + 1) {
      //Delete the front of the buffer
      SendBuffer.Erase(0, newack - last_acked);
      last_acked = newack - 1;
      return true;
    } else {
      return false;
    }
  } else if(newack > last_acked && newack > last_sent) {
    //Delete the front of the buffer
    SendBuffer.Erase(0, newack - last_acked);
    last_acked = newack - 1;
    return true;
  } else if(newack < last_acked && newack <= last_sent + 1) {
    //Delete the front of the buffer
    SendBuffer.Erase(0, (SEQ_LENGTH_MASK - last_acked + 1) + newack);
    last_acked = newack - 1;
    return true;
  } else
    return false;
}

unsigned int TCPState::GetRwnd()
{
  return TCP_BUFFER_SIZE - RecvBuffer.GetSize();
}
void TCPState::SetLastRecvd(unsigned int lastrecvd)
{
  last_recvd = lastrecvd;
}

bool TCPState::SetLastRecvd(unsigned int lastrecvd, unsigned int length)
{
  if(lastrecvd == last_recvd + 1) {
    if((length <= N) && (RecvBuffer.GetSize()+length <= TCP_BUFFER_SIZE)) {
      last_recvd += length;
      return true;
    } else {
      return false;
    }
  }
  else {
    return false;
  }
}

void TCPState::SendPacketPayload(unsigned &offsetlastsent, size_t &bytesize, unsigned bytes)
{
  if(last_acked < last_sent) {
    offsetlastsent = last_sent - last_acked;
  } else {
    offsetlastsent = SEQ_LENGTH_MASK - last_acked + last_sent + 1;
  }

  bytesize = MIN_MACRO((rwnd - offsetlastsent), (N - offsetlastsent));
  bytesize = MIN_MACRO(bytesize, bytes);
  bytesize = MIN_MACRO(bytesize, TCP_MAXIMUM_SEGMENT_SIZE);
}
