#ifndef _headertrailer
#define _headertrailer

#include "config.h"
#include "buffer.h"
#include <deque>
#include <iostream>

namespace Headers {
    enum HeaderType {EthernetHeader, 
		     ARPHeader, 
		     IPHeader, 
		     UDPHeader, 
		     TCPHeader, 
		     ICMPHeader};
}

namespace Trailers {
    enum TrailerType {EthernetTrailer};
}

std::ostream & operator<< (std::ostream &os, const Headers::HeaderType &h);
std::ostream & operator<< (std::ostream &os, const Trailers::TrailerType &t);

typedef TaggedBuffer<Headers::HeaderType>  Header;
typedef TaggedBuffer<Trailers::TrailerType> Trailer;

#endif
