#include <assert.h>
#include "packet.h"

Packet::Packet()
{}

Packet::Packet(const Packet &rhs) : headers(rhs.headers), payload(rhs.payload), trailers(rhs.trailers)
{}

Packet::Packet(const Buffer &rhs) : payload(rhs)
{}

Packet::Packet(const RawEthernetPacket &rhs) : payload(rhs.data,rhs.size)
{}

Packet::Packet(const char *buf, size_t size) : payload(buf,size)
{}

Packet::~Packet()
{}

//const char Packet::operator[] (const unsigned offset)
//{
//  return datarope[offset];
//}


const Packet & Packet::operator= (const Packet &rhs)
{
  headers=rhs.headers;
  payload=rhs.payload;
  trailers=rhs.trailers;
  return *this;
}


void Packet::Serialize(const int fd) const
{
  size_t num;

  num=headers.size();
  if (writeall(fd,(char*)&num,sizeof(num))!=sizeof(num)) {
    throw SerializationException();
  }
  for (std::deque<Header>::const_iterator p=headers.begin();p!=headers.end();p++) {
    (*p).Serialize(fd);
  }
  payload.Serialize(fd);
  num=trailers.size();
  if (writeall(fd,(char*)&num,sizeof(num))!=sizeof(num)) {
    throw SerializationException();
  }
  for (std::deque<Trailer>::const_iterator p=trailers.begin();p!=trailers.end();p++) {
    (*p).Serialize(fd);
  }
}


void Packet::Unserialize(const int fd)
{
  size_t num;
  unsigned i;

  headers.clear();
  payload.Clear();
  trailers.clear();

  if (readall(fd,(char*)&num,sizeof(num))!=sizeof(num)) {
    throw SerializationException();
  }
  for (i=0;i<num;i++) {
    Header h;
    h.Unserialize(fd);
    headers.push_back(h);
  }
  payload.Unserialize(fd);
  if (readall(fd,(char*)&num,sizeof(num))!=sizeof(num)) {
    throw SerializationException();
  }
  for (i=0;i<num;i++) {
    Trailer t;
    t.Unserialize(fd);
    trailers.push_back(t);
  }
}

size_t Packet::GetRawSize() const
{
  size_t sum=0;

  for (std::deque<Header>::const_iterator p=headers.begin();p!=headers.end();p++) {
    sum+=(*p).GetSize();
  }
  sum+=payload.GetSize();
  for (std::deque<Trailer>::const_iterator p=trailers.begin();p!=trailers.end();p++) {
    sum+=(*p).GetSize();
  }
  return sum;
}


void Packet::DupeRaw(char * buf, size_t size) const
{
  int offset=0;

  assert(size>=GetRawSize());

  for (std::deque<Header>::const_iterator p=headers.begin();p!=headers.end();p++) {
    (*p).GetData(&(buf[offset]),(*p).GetSize(),0);
    offset+=(*p).GetSize();
  }
  payload.GetData(&(buf[offset]),payload.GetSize(),0);
  offset+=payload.GetSize();
  for (std::deque<Trailer>::const_iterator p=trailers.begin();p!=trailers.end();p++) {
    (*p).GetData(&(buf[offset]),(*p).GetSize(),0);
    offset+=(*p).GetSize();
  }
}


void Packet::WriteRaw(const int fd) const
{
  char *buf = new char [GetRawSize()];

  DupeRaw(buf,GetRawSize());

  writeall(fd,buf,GetRawSize());
}

void Packet::PushHeader(const Header &header)
{
  PushFrontHeader(header);
}

void Packet::PushFrontHeader(const Header &header)
{
  headers.push_front(header);
}

void Packet::PushBackHeader(const Header &header)
{
  headers.push_back(header);
}


Header & Packet::PopHeader()
{
  return PopFrontHeader();
}

Header & Packet::PopFrontHeader()
{
  Header &x=headers.front();
  headers.pop_front();
  return x;
}

Header & Packet::PopBackHeader()
{
  Header &x=headers.front();
  headers.pop_back();
  return x;
}

template <class T, typename TAGTYPE>
struct find_pred : public std::unary_function<T,bool> {
  TAGTYPE tag;
  find_pred(const TAGTYPE &t) : tag(t) {}
  bool operator() (T rhs) { return rhs.GetTag()==tag; }
};


Header &  Packet::FindHeader(Headers::HeaderType ht) const
{
  std::deque<Header>::const_iterator i = find_if(headers.begin(), headers.end(), find_pred<Header,Headers::HeaderType>(ht));
  return *(new Header(*i));
}

void Packet::SetHeader(const Header &h)
{
  replace_if(headers.begin(), headers.end(), find_pred<Header,Headers::HeaderType>(h.GetTag()), h);
}

Trailer & Packet::FindTrailer(Trailers::TrailerType ht) const
{
  std::deque<Trailer>::const_iterator i = find_if(trailers.begin(), trailers.end(), find_pred<Trailer,Trailers::TrailerType>(ht));
  return *(new Trailer(*i));
}

void Packet::SetTrailer(const Trailer &h)
{
  replace_if(trailers.begin(), trailers.end(), find_pred<Trailer,Trailers::TrailerType>(h.GetTag()), h);
}


Buffer & Packet::GetPayload()
{
  return *(new Buffer(payload));
}

void       Packet::PushTrailer(const Trailer &trailer)
{
  PushBackTrailer(trailer);
}

void       Packet::PushBackTrailer(const Trailer &trailer)
{
  trailers.push_back(trailer);
}

void       Packet::PushFrontTrailer(const Trailer &trailer)
{
  trailers.push_front(trailer);
}


Trailer &  Packet::PopTrailer()
{
  return PopFrontTrailer();
}

Trailer &  Packet::PopFrontTrailer()
{
  Trailer &x=trailers.front();
  trailers.pop_front();
  return x;
}

Trailer &  Packet::PopBackTrailer()
{
  Trailer &x=trailers.front();
  trailers.pop_back();
  return x;
}

void Packet::ExtractHeaderFromPayload(Headers::HeaderType type, size_t size)
{
  headers.push_back(Header(type,payload.ExtractFront(size)));
}

void Packet::ExtractTrailerFromPayload(Trailers::TrailerType type, size_t size)
{
  trailers.push_front(Trailer(type,payload.ExtractBack(size)));
}


std::ostream & Packet::Print(std::ostream &os) const
{
  os << "Packet(headers={";
  for_each(headers.begin(), headers.end(),PrintFunc<Header>(os));
  os << "}, payload=";
  os << payload ;
  os << ", trailers={";
  for_each(trailers.begin(), trailers.end(),PrintFunc<Trailer>(os));
  os << "}";
  os << ")";
  return os;
}



