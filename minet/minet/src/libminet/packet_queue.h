#ifndef _packet_queue
#define _packet_queue

#include <deque>
#include "packet.h"


class PacketQueue {
 private:
  std::deque<Packet>  queue;
 public:
  PacketQueue();
  PacketQueue(const PacketQueue &rhs);
  const PacketQueue & operator=(const PacketQueue &rhs);
  virtual ~PacketQueue();

  bool     IsEmpty() const ;
  unsigned NumItems() const;
  void     PushPacket(const Packet &packet);
  Packet & PullPacket();
};



#endif
