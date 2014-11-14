#ifndef _buffer
#define _buffer

#include <iostream>
#include <string>
//#include <rope>
#include "config.h"
#include "util.h"


class Buffer {
 private:
    std::string datarope;
 public:
    Buffer();
    Buffer(const char *data, size_t size);
    Buffer(const Buffer &rhs);
    virtual ~Buffer();
    virtual const Buffer& operator=(const Buffer &rhs);

    virtual char operator[](const unsigned offset);

    virtual void Clear();

    virtual void Insert(const Buffer &rhs, unsigned offset);
    virtual void AddFront(const Buffer &rhs);
    virtual void AddBack(const Buffer &rhs);
    virtual void Erase(unsigned offset, size_t size);

    virtual Buffer & Extract(unsigned offset, size_t size);
    virtual Buffer & ExtractFront(size_t size);
    virtual Buffer & ExtractBack(size_t size);

    virtual size_t GetSize() const;
    virtual size_t GetData(char *buf, size_t size, unsigned offset) const;
    virtual size_t SetData(const char *buf, size_t size, unsigned offset);

    virtual void Serialize(const int fd) const;
    virtual void Unserialize(const int fd);

    virtual std::ostream & Print(std::ostream &) const;

    friend std::ostream &operator<<(std::ostream &os, const Buffer& L) {
	return L.Print(os);
    }
};


template <typename TAGTYPE>
class TaggedBuffer : public Buffer {
private:
    TAGTYPE tag;
public:
    TaggedBuffer() : Buffer(), tag(TAGTYPE()) {}
    TaggedBuffer(TAGTYPE t) : Buffer(), tag(t) {}
    TaggedBuffer(TAGTYPE t, const char *data, size_t size) : Buffer(data,size), tag(t)  {}
    TaggedBuffer(const TaggedBuffer<TAGTYPE> &rhs) : Buffer(rhs), tag(rhs.tag) {}
    TaggedBuffer(TAGTYPE t, const Buffer &rhs) : Buffer(rhs), tag(t) {}

    virtual ~TaggedBuffer() {}

    virtual const TaggedBuffer<TAGTYPE> & operator = (const TaggedBuffer<TAGTYPE> &rhs) {
	tag = rhs.tag;
	Buffer::operator = (rhs);
	return *this;
    }

    virtual TAGTYPE GetTag() const { 
	return tag;
    }

    virtual void Serialize(const int fd) const {
	writeall(fd, (char *)&tag, sizeof(tag));
	Buffer::Serialize(fd);
    }

    virtual void Unserialize(const int fd) {
	readall(fd, (char *)&tag, sizeof(tag));
	Buffer::Unserialize(fd);
    }

    virtual std::ostream & Print(std::ostream &os) const {
	os << "TaggedBuffer(tag=" << tag << ", ";
	return Buffer::Print(os);
    }

    friend std::ostream &operator<<(std::ostream &os, const TaggedBuffer& L) {
	return L.Print(os);
    }
};



#endif
