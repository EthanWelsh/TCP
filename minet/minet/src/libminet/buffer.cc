#include "buffer.h"
#include "util.h"


Buffer::Buffer()
{
}

Buffer::Buffer(const char * data, size_t size) : datarope(data, size)
{
}

Buffer::Buffer(const Buffer &rhs) : datarope(rhs.datarope)
{
}

Buffer::~Buffer()
{
}


char Buffer::operator[] (const unsigned offset) {
    return datarope[offset];
}

const Buffer & Buffer::operator = (const Buffer & rhs) {
    datarope = rhs.datarope;
    return *this;
}

void Buffer::Clear() {
    datarope.erase(0, datarope.size());
}

void Buffer::Insert(const Buffer &rhs, unsigned offset) {
    datarope.insert(offset, rhs.datarope);
}


void Buffer::AddBack(const Buffer &rhs) {
    datarope += rhs.datarope;
}

void Buffer::AddFront(const Buffer &rhs) {
    datarope.insert(0, rhs.datarope);
}

void Buffer::Erase(unsigned offset, size_t size) {
    datarope.erase(offset, size);
}

Buffer & Buffer::Extract(unsigned offset, size_t size) {
    Buffer * temp = new Buffer();

    temp->datarope = datarope.substr(offset, size);
    datarope.erase(offset, size);

    return *temp;
}

Buffer & Buffer::ExtractFront(size_t size) {
    return Extract(0, size);
}


Buffer & Buffer::ExtractBack(size_t size) {
    return Extract(GetSize() - size, size);
}

size_t Buffer::GetSize() const {
    return datarope.size();
}

size_t Buffer::GetData(char * buf, size_t size, unsigned offset) const {
    //  datarope.copy(offset,size,buf);
    //  ROPE_COPY(datarope,offset,size,buf);
    
    // MIGHT REGRET: making crope -> string conversion permanent
    datarope.copy(buf, size, offset);
    
    return size;
}

size_t Buffer::SetData(const char *buf, size_t len, unsigned offset) {
    size_t size = datarope.size();

    if ((offset + len) <= size) {
	datarope.replace(offset, len, buf, len);
    } else if (offset >= size) {
	
	if (offset - size > 0) {
	    datarope.append(offset - size, 0);
	}
	
	datarope.append(buf, len);
	
    } else {
	size_t r1 = size - offset;
	size_t r2 = offset + len - size;
	
	if (r1 > 0) {
	    datarope.replace(offset,r1,buf,r1);
	}
	
	if (r2 > 0) {
	    datarope.append(&(buf[r1]),r2);
	}
    }

    return len;
}

void Buffer::Serialize(const int fd) const {
    int size = GetSize();
    char * buf = new char[size];
    
    if (writeall(fd, (char *)&size, sizeof(size), 0, 1) != sizeof(size)) {
	throw SerializationException();
    }
    
    GetData(buf, size, 0);

    if (writeall(fd, buf, size, 0, 1) != size) {
	throw SerializationException();
    }
    
    delete [] buf;
}

void Buffer::Unserialize(const int fd) {
    int size;
    char * buf = NULL;

    if (readall(fd, (char *)&size, sizeof(size)) != sizeof(size)) {
	throw SerializationException();
    }
    
    buf = new char[size];

    if (readall(fd, buf, size) != size) {
	throw SerializationException();
    }
    
    datarope = std::string(buf, size);
    
    delete [] buf;

}


std::ostream & Buffer::Print(std::ostream &os) const {
    size_t size = GetSize();
    char hex[2];
    unsigned i;

    os << "Buffer(size=" << size << ", data=";
    
    for (i = 0; i < size; i++) {
	bytetohexbyte(datarope[i], hex);
	os << hex[0] << hex[1];
    }

    os << ", text=\"";

    for (i = 0; i < size; i++) {
	char c = datarope[i];
	
	if ((c < 32) || (c > 126)) {
	    c = '.';
	}

	os << c;
    }

    os << "\")";

    return os;
}


