#include "headertrailer.h"

std::ostream & operator<< (std::ostream &os, const Headers::HeaderType &h)
{
    switch (h) {
	case Headers::EthernetHeader:
	    return (os << "EthernetHeader");
	case Headers::ARPHeader:
	    return (os << "ARPHeader");
	case Headers::IPHeader:
	    return (os << "IPHeader");
	case Headers::UDPHeader:
	    return (os << "UDPHeader");
	case Headers::TCPHeader:
	    return (os << "TCPHeader");
	case Headers::ICMPHeader:
	    return (os << "ICMPHeader");
    }

    return (os << "UNKNOWN HEADER TYPE" );
}

std::ostream & operator<< (std::ostream &os, const Trailers::TrailerType &t)
{

    switch (t) {
	case Trailers::EthernetTrailer:
	    return (os << "EthernetTrailer");
    }

    return (os << "UNKNOWN TRAILER TYPE");
}

