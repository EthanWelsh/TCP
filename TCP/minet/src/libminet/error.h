#ifndef _error
#define _error

#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <errno.h>
#include <string.h>

#include <iostream>

#define STRINGIZE(T) #T

#define PERROR() do {							\
	fprintf(stderr, "%d: %s(%s:%d): %s\n",				\
		getpid(),						\
		__PRETTY_FUNCTION__,					\
		__FILE__,						\
		__LINE__,						\
		strerror(errno));					\
    } while (0)

/*
  #define PERROR() do { extern char batman[1024]; sprintf(batman, "%s:%d %s",
  __FILE__, __LINE__, __PRETTY_FUNCTION__); perror(batman); } while(0)
*/

inline int Die(const char * string) {
    fprintf(stderr,"Aborting because: %s\n", string);
    exit(-1);
    return 0;
}


#endif
