#include "packet_queue.h"

PacketQueue::PacketQueue()
{}

PacketQueue::PacketQueue(const PacketQueue &rhs) : queue(rhs.queue)
{}

const PacketQueue & PacketQueue::operator=(const PacketQueue &rhs)
{
  queue = rhs.queue;
  return *this;
}



PacketQueue::~PacketQueue()
{
  queue.clear();
}

bool PacketQueue::IsEmpty() const
{
  return queue.empty();
}

unsigned PacketQueue::NumItems() const
{
  return queue.size();
}

void PacketQueue::PushPacket(const Packet &p)
{
  queue.push_back(p);
}

Packet & PacketQueue::PullPacket()
{
  Packet &p = queue.front();
  queue.pop_front();
  return p;
}
