#ifndef _sock
#define _sock

#define NUM_SOCKS           100   // The maximum number of sockets we support.
                                  // Notes:
                                  //   1) No allowance is made for reusing sock
                                  //      numbers on different addresses; this
                                  //      is an absolute limit.
                                  //   2) Usable socks are numbered from 
                                  //      1..(NUM_SOCKS - 1).  0 is a special 
                                  //      value (as indicated below) which 
                                  //      indicates that the socket is
                                  //      unspecified.
#define NUM_PORTS           65536 // The number of ports on each IP address.
                                  //   As above, 0 is a special case that 
                                  //   indicates that the port is unassigned.
#define NUM_IP_INTERFACES   10    // The number of distinct IP addresses we can
                                  //   support (limitation is imposed to 
                                  //   constrain the size of the port array).
#define BIN_SIZE            65536 // The size of the input buffer.
#define BOUT_SIZE           65536 // The size of the output buffer/

#define UNSPECIFIED_SOCK    0

#endif
