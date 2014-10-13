#ifndef _sock_mod_structs
#define _sock_mod_structs

#include <iostream>
#include "sockint.h"

enum Status {FREE, UNBOUND, BOUND, LISTENING, ACCEPT_PENDING,
	     CONNECT_PENDING, CONNECTED, READ_PENDING,
	     WRITE_PENDING};

struct SockRecord {
  Connection    connection;
  Buffer        bin;
  //  Buffer        bout;
  Status        status;
  int           toApp;
  int           fromApp;
  int           blocking;
  int           forward_read_notification;
  int           forward_write_notification;
  int           forward_exception_notification;

  SockRecord();
  SockRecord(const SockRecord &rhs);
  SockRecord(const Connection &c,
	     const Buffer     &bi,
	     //	     const Buffer     &bo,
	     const Status     &s,
	     const int        &ta,
	     const int        &fa,
	     const int        &b,
	     const int        &frn,
	     const int        &fwn,
	     const int        &fen);
  virtual ~SockRecord() {}
  SockRecord & operator=(const SockRecord &rhs);
  std::ostream & Print(std::ostream &rhs) const;
  friend std::ostream &operator<<(std::ostream &os, const SockRecord& L) {
    return L.Print(os);
  }
};

struct SockStatus {
  SockRecord sockArray[NUM_SOCKS];

  int FindFreeSock();                          // Return the first free socket,
                                               //   or -1 if no socket is
                                               //   available.

  void CloseSocket(unsigned sock) {            // Close the specified socket.
    sockArray[sock] = SockRecord(); }

  int FindConnection(const Connection & c);    // Find a socket which matches
                                               //   this connection, return
                                               //   its fd or -1 if no socket
                                               //   matches.
  int FindPendingConnection(const Connection & c);
                                               // Find a socket which matches
                                               //   this connection and is
                                               //   waiting to complete its
                                               //   connection.  Return its fd
                                               //   or -1 if no socket matches.
  Connection *GetConnection (unsigned sock) {  // Get the connection for the
    return (&sockArray[sock].connection); }    //   specified socket.

  Buffer *GetBin (unsigned sock) {             // Get the input buffer for the
    return (&sockArray[sock].bin); }           //   specified socket.

  //Buffer *GetBout (unsigned sock) {            // Get the output buffer for
  //  return (&sockArray[sock].bout); }          //   the specified socket.

  Status GetStatus (unsigned sock) {           // Get the status of the
    return (sockArray[sock].status); }         //   specified socket.
  int SetStatus (unsigned sock, Status stat);  // Set the status of the
                                               //   specified socket.

  int GetFifoToApp (unsigned sock) {           // Get the fd of the fifo to the
    return (sockArray[sock].toApp); }          //   application layer for the
                                               //   specified socket.
  int SetFifoToApp (unsigned sock, int fd);    // Set the fd of the fifo to the
                                               //   application layer for the
                                               //   specified socket.

  int GetFifoFromApp (unsigned sock) {         // Get the fd of the fifo from
    return (sockArray[sock].fromApp); }        //   the application layer for
                                               //   the specified socket.
  int SetFifoFromApp (unsigned sock, int fd);  // Set the fd of the fifo from
                                               //   the application layer for
                                               //   the specified socket.

  int GetBlockingStatus (unsigned sock) {      // Return 0 if the socket is
    return (sockArray[sock].blocking); }       //   set non-blocking, 1 if it
                                               //   is set to block.
  int SetBlockingStatus (unsigned sock, int b);// Set the blocking status for
                                               //   the specified socket.

  int GetReadNotificationStatus (unsigned sock) {
    return (sockArray[sock].forward_read_notification); }
                                               // Returns 1 if we have been
                                               //   asked to notify the
                                               //   application layer the next
                                               //   time this socket is
                                               //   available for reading, 0
                                               //   otherwise.
  int SetReadNotificationStatus (unsigned sock,// Set the read notification
				 int s);       //   status.

  int GetWriteNotificationStatus (unsigned sock) {
    return (sockArray[sock].forward_write_notification); }
                                               // Returns 1 if we have been
                                               //   asked to notify the
                                               //   application layer the next
                                               //   time this socket is
                                               //   available for writing, 0
                                               //   otherwise.
  int SetWriteNotificationStatus (unsigned sock, int s);
                                               // Set the write notification
			                       //   status.


  int GetExceptionNotificationStatus (unsigned sock) {
    return (sockArray[sock].forward_exception_notification); }
                                               // Returns 1 if we have been
                                               //   asked to notify the
                                               //   application layer the next
                                               //   time this socket has an
                                               //   exception, 0 otherwise.
  int SetExceptionNotificationStatus (unsigned sock, int s);
                                               // Set the exception
                                               //   notification status

  SockStatus() {}
  SockStatus(const SockStatus &rhs);
  virtual ~SockStatus() {}
  SockStatus & operator=(const SockStatus &rhs);
};

struct PortStatus {
  IPAddress portArrayIndex[NUM_IP_INTERFACES];
  unsigned portArray[NUM_IP_INTERFACES][NUM_PORTS];

  int FindFreePort(IPAddress ip,               // Return the port number of an
		   unsigned sockfd);           //   available port for the
                                               //   specified ip.  Assign the
                                               //   port to the specified
                                               //   socket.  Return -1 on
                                               //   failure.
  int Socket(IPAddress ip, unsigned port);     // Return the socket to which
                                               //   a given port on the
                                               //   specified ip is assigned.
                                               //   Returns 0 if the port is
                                               //   unassigned.  Return -1 on
                                               //   failure
  int AssignPort(IPAddress ip, unsigned port,  // Assign the specified port to
		 unsigned sockfd);             //   the specified ip.  Return
                                               //   -1 on failure.

  PortStatus();
  PortStatus(const PortStatus &rhs);
  virtual ~PortStatus() {}
  PortStatus & operator=(const PortStatus & rhs);
};


struct RequestRecord {
  SockRequestResponse *srr;
  int sock;

  RequestRecord() {}
  RequestRecord(SockRequestResponse *s, int fd);
  RequestRecord(const RequestRecord &rhs);
  ~RequestRecord() {
    delete srr; }
  RequestRecord & operator=(const RequestRecord & rhs);
};


struct QueueElt {
  void *data;
  QueueElt *front;
  QueueElt *back;

  QueueElt() {}
  QueueElt(const QueueElt &rhs);
  QueueElt(char *d, QueueElt *f, QueueElt *b);
  virtual ~QueueElt() {}
  QueueElt & operator=(const QueueElt & rhs);
};

struct Queue {
  QueueElt *front;
  QueueElt *back;

  void Insert(void * d);
  void * Remove();

  Queue();
  Queue(const Queue &rhs);
  Queue & operator=(const Queue & rhs);
  ~Queue();
};

#endif
