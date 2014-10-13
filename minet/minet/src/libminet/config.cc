#include "config.h"


char * READER_BINARY = getenv("MINET_READER");

char * READER_ARGS[] = {READER_BINARY, 
			getenv("MINET_ETHERNETDEVICE"), 
			getenv("MINET_IPADDR"),
			0} ;

char * WRITER_BINARY = getenv("MINET_WRITER");

char * WRITER_ARGS[] = {WRITER_BINARY, 
			getenv("MINET_ETHERNETDEVICE"), 
			getenv("MINET_WRITERBUFFER"), 
			0};




