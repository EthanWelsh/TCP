#ifndef minet_sockets
#define minet_sockets

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <cstdio>
#include <sys/poll.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef enum {MINET_KERNEL, MINET_USER} minet_socket_types;

EXTERNC int minet_error ();
  // Returns the error code for the last error.

EXTERNC int minet_perror (const char *s);
  // Prints an informative error message for the last error.

EXTERNC int minet_init (minet_socket_types type);

EXTERNC int minet_deinit ();
  // de-initialize minet.

EXTERNC int minet_socket (int type);
  // Create a socket.  Type must be SOCK_STREAM (TCP)
  // or SOCK_DGRAM (UDP)
  // or SOCK_ICMP (ICMP)

EXTERNC int minet_bind (int                 sockfd,
			struct sockaddr_in *myaddr);
  // Bind a socket to the IP address (AF_INET) and port
  // specified in myaddr.

EXTERNC int minet_listen (int sockfd,
			  int backlog);
  // Listen on a socket.

EXTERNC int minet_accept (int                 sockfd,
			  struct sockaddr_in *addr);
  // Accept a connection.

EXTERNC int minet_connect (int                 sockfd,
			   struct sockaddr_in *addr);
  // Connect to a remote socket.

EXTERNC int minet_read (int   fd,
			char *buf,
			int   len);
  // Read from a connected socket (returns # bytes actually read).

EXTERNC int minet_write (int   fd,
			 char *buf,
			 int   len);
  // Write to a connected socket (returns # bytes actually written).

EXTERNC int minet_recvfrom (int                 fd,
			    char               *buf,
			    int                 len,
			    struct sockaddr_in *from);
  // Read from an unconnected socket.

EXTERNC int minet_sendto (int                 fd,
			  char               *buf,
			  int                 len,
			  struct sockaddr_in *to);
  // Write to an unconnected socket.

EXTERNC int minet_close (int sockfd);
  // Close a socket.

EXTERNC int minet_select (int             minet_maxfd,
			  fd_set         *minet_read_fds,
			  fd_set         *minet_write_fds,
			  fd_set         *minet_except_fds,
			  struct timeval *timeout);
  // Does a select on whatever interface was selected using
  // minet_init.

EXTERNC int minet_select_ex (int             minet_maxfd,
			     fd_set         *minet_read_fds,
			     fd_set         *minet_write_fds,
			     fd_set         *minet_except_fds,
			     int             unix_maxfd,
			     fd_set         *unix_read_fds,
			     fd_set         *unix_write_fds,
			     fd_set         *unix_except_fds,
			     struct timeval *timeout);
  // Does a select on both interfaces.  Only possible if
  // MINET_USER sockets were selected using minet_init.

EXTERNC int minet_poll (struct pollfd *minet_fds,
			int            num_minet_fds,
			int            timeout);
  // Does a poll on whatever interface was selected using
  // minet_init.

EXTERNC int minet_poll_ex (struct pollfd *minet_fds,
			   int            num_minet_fds,
			   struct pollfd *unix_fds,
			   int            num_unix_fds,
			   int            timeout);
  // Does a poll on both interfaces.  Only possible if
  // MINET_USER sockets were selected using minet_init.

EXTERNC int minet_set_nonblocking (int sockfd);
  // Set a socket to be non-blocking.

EXTERNC int minet_set_blocking (int sockfd);
  // Set a socket to be blocking.

EXTERNC int minet_can_write_now (int sockfd);
  // Check if a socket is ready for writing.

EXTERNC int minet_can_read_now (int sockfd);
  // Check if a socket is ready for reading.

#endif
