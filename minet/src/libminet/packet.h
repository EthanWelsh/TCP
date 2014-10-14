#ifndef _packet
#define _packet

#include <deque>
#include "config.h"
#include "buffer.h"
#include "headertrailer.h"
#include "raw_ethernet_packet.h"

struct RawEthernetPacket;

class Packet {
 protected:
  std::deque<Header>  headers;
  Buffer         payload;
  std::deque<Trailer> trailers;
 public:
  Packet();
  Packet(const Packet &rhs);
  Packet(const Buffer &rhs);
  Packet(const char *buf, size_t size);
  Packet(const RawEthernetPacket &rhs);
  virtual ~Packet();

  virtual const Packet & operator= (const Packet &rhs);

  virtual void Serialize(const int fd) const;
  virtual void Unserialize(const int fd);


  virtual size_t GetRawSize() const;

  virtual void WriteRaw(const int fd) const;
  virtual void DupeRaw(char *buf, size_t size) const;

  virtual void       PushHeader(const Header &header);
  virtual void       PushFrontHeader(const Header &header);
  virtual void       PushBackHeader(const Header &header);

  virtual Header &   PopHeader();
  virtual Header &   PopFrontHeader();
  virtual Header &   PopBackHeader();

  virtual void       PushTrailer(const Trailer &trailer);
  virtual void       PushFrontTrailer(const Trailer &trailer);
  virtual void       PushBackTrailer(const Trailer &trailer);

  virtual Trailer &  PopTrailer();
  virtual Trailer &  PopFrontTrailer();
  virtual Trailer &  PopBackTrailer();

  virtual Header &   FindHeader(Headers::HeaderType ht) const;
  virtual void       SetHeader(const Header &h);
  virtual Trailer &  FindTrailer(Trailers::TrailerType tt) const;
  virtual void       SetTrailer(const Trailer &h);

  virtual Buffer &   GetPayload();

  virtual void ExtractHeaderFromPayload(Headers::HeaderType type, size_t bytes);
  virtual void ExtractTrailerFromPayload(Trailers::TrailerType type, size_t bytes);

  template <class HEADER> void ExtractHeaderFromPayload(size_t size) {
    headers.push_back(HEADER(payload.ExtractFront(size)));
  }

  template <class TRAILER> void ExtractTrailerFromPayload(size_t size) {
    trailers.push_front(TRAILER(payload.ExtractBack(size)));
  }

  virtual std::ostream & Print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const Packet& L) {
    return L.Print(os);
  }
};


#endif
