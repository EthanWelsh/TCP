#include "sock_mod_structs.h"


SockRecord::SockRecord() :
  connection(Connection()),
  bin(Buffer()),
  //  bout(Buffer()),
  status(FREE),
  toApp(0),
  fromApp(0),
  blocking(1),
  forward_read_notification(0),
  forward_write_notification(0),
  forward_exception_notification(0)
{
  bin.Clear();
  //  bout.Clear();
}


SockRecord::SockRecord(const SockRecord &rhs) :
  connection(rhs.connection),
  bin(rhs.bin),
  //  bout(rhs.bout),
  status(rhs.status),
  toApp(rhs.toApp),
  fromApp(rhs.fromApp),
  blocking(rhs.blocking),
  forward_read_notification(rhs.forward_read_notification),
  forward_write_notification(rhs.forward_write_notification),
  forward_exception_notification(rhs.forward_exception_notification)
{}


SockRecord::SockRecord(const Connection &c,
		       const Buffer     &bi,
		       //		       const Buffer     &bo,
		       const Status     &s,
		       const int        &ta,
		       const int        &fa,
		       const int        &b,
		       const int        &frn,
		       const int        &fwn,
		       const int        &fen) :
  connection(c),
  bin(bi),
  //  bout(bo),
  status(s),
  toApp(ta),
  fromApp(fa),
  blocking(b),
  forward_read_notification(frn),
  forward_write_notification(fwn),
  forward_exception_notification(fwn)
{}


SockRecord & SockRecord::operator= (const SockRecord &rhs) {
  connection = rhs.connection;
  bin = rhs.bin;
  //  bout = rhs.bout;
  status = rhs.status;
  toApp = rhs.toApp;
  fromApp = rhs.fromApp;
  blocking = rhs.blocking;
  forward_read_notification = rhs.forward_read_notification;
  forward_write_notification = rhs.forward_write_notification;
  forward_exception_notification =
    rhs.forward_exception_notification;
  return *this;
}


std::ostream & SockRecord::Print(std::ostream &rhs) const {
  rhs << "SockRecord(connection=" << connection
      << ", bin=" << bin
    //      << ", bout=" << bout
      << ", status="
      << (status==FREE ? "FREE" :
	  status==UNBOUND ? "UNBOUND" :
	  status==BOUND ? "BOUND" :
	  status==LISTENING ? "LISTENING" :
	  status==ACCEPT_PENDING ? "ACCEPT_PENDING" :
	  status==CONNECT_PENDING ? "CONNECT_PENDING" :
	  status==CONNECTED ? "CONNECTED" :
	  status==READ_PENDING ? "READ_PENDING" :
	  status==WRITE_PENDING ? "WRITE_PENDING" :
	  "UNKNOWN")
      << ", toApp=" << toApp
      << ", fromApp=" << fromApp
      << ", blocking=" << blocking
      << ")";
  return rhs;
}


SockStatus::SockStatus(const SockStatus &rhs) {
  int i;
  for (i = 0; i < NUM_SOCKS; i++)
    sockArray[i] = rhs.sockArray[i];
}


SockStatus & SockStatus::operator=(const SockStatus &rhs) {
  int i;
  for (i = 0; i < NUM_SOCKS; i++)
    sockArray[i] = rhs.sockArray[i];
  return *this;
}


int SockStatus::FindFreeSock() {
  int i;
  for (i = 1; i < NUM_SOCKS; i++)
    if (sockArray[i].status == FREE)
      break;
  if (i < NUM_SOCKS)
    return i;
  return -1;
}


int SockStatus::FindConnection(const Connection & c) {
  int i;
  for (i = 1; i < NUM_SOCKS; i++)
    if (((c.src == sockArray[i].connection.src) &&
	 (c.dest == sockArray[i].connection.dest) &&
	 (c.srcport == sockArray[i].connection.srcport) &&
	 (c.destport == sockArray[i].connection.destport) &&
	 (c.protocol == sockArray[i].connection.protocol))
	||
	((c.src == sockArray[i].connection.src) &&
	 (IP_ADDRESS_ANY == sockArray[i].connection.dest) &&
	 (c.srcport == sockArray[i].connection.srcport) &&
	 (PORT_ANY == sockArray[i].connection.destport) &&
	 (c.protocol == sockArray[i].connection.protocol) &&
	 (ACCEPT_PENDING == sockArray[i].status)))
      break;
  if (i < NUM_SOCKS)
    return i;
  return -1;
}

int SockStatus::FindPendingConnection(const Connection & c) {
  int i;
  for (i = 1; i < NUM_SOCKS; i++)
    if ((c.src == sockArray[i].connection.src) &&
	((c.dest == sockArray[i].connection.dest) ||
	 (sockArray[i].connection.dest == IP_ADDRESS_ANY)) &&
	(c.srcport == sockArray[i].connection.srcport) &&
	((c.destport == sockArray[i].connection.destport) ||
	 (sockArray[i].connection.destport == PORT_ANY)) &&
	(c.protocol == sockArray[i].connection.protocol) &&
	((sockArray[i].status == CONNECT_PENDING) ||
	 (sockArray[i].status == ACCEPT_PENDING)))
      break;
  if (i < NUM_SOCKS)
    return i;
  return -1;
}


int SockStatus::SetStatus (unsigned sock, Status stat) {
  if ((sock < 1) || (sock >= NUM_SOCKS))
    return -1;
  sockArray[sock].status = stat;
  return 0;
}


int SockStatus::SetFifoToApp (unsigned sock, int fd) {
  if ((sock < 1) || (sock >= NUM_SOCKS) || (sockArray[sock].status == FREE))
    return -1;
  sockArray[sock].toApp = fd;
  return 0;
}


int SockStatus::SetFifoFromApp (unsigned sock, int fd) {
  if ((sock < 1) || (sock >= NUM_SOCKS) || (sockArray[sock].status == FREE))
    return -1;
  sockArray[sock].fromApp = fd;
  return 0;
}


int SockStatus::SetBlockingStatus (unsigned sock, int b) {
  if ((sock < 1) || (sock >= NUM_SOCKS) || (sockArray[sock].status == FREE))
    return -1;
  sockArray[sock].blocking = b;
  return 0;
}


int SockStatus::SetReadNotificationStatus (unsigned sock, int s) {
  if ((sock < 1) || (sock >= NUM_SOCKS) || (sockArray[sock].status == FREE))
    return -1;
  sockArray[sock].forward_read_notification = s;
  return 0;
}


int SockStatus::SetWriteNotificationStatus (unsigned sock, int s) {
  if ((sock < 1) || (sock >= NUM_SOCKS) || (sockArray[sock].status == FREE))
    return -1;
  sockArray[sock].forward_write_notification = s;
  return 0;
}


int SockStatus::SetExceptionNotificationStatus (unsigned sock, int s) {
  if ((sock < 1) || (sock >= NUM_SOCKS) || (sockArray[sock].status == FREE))
    return -1;
  sockArray[sock].forward_exception_notification = s;
  return 0;
}


PortStatus::PortStatus() {
  int i, j;
  for (i = 0; i < NUM_IP_INTERFACES; i++) {
    portArrayIndex[i] = IP_ADDRESS_ANY;
    for (j = 0; j < NUM_PORTS; j++)
      portArray[i][j] = 0;
  }
}


PortStatus::PortStatus(const PortStatus & rhs) {
  int i,j;
  for (i = 0; i < NUM_IP_INTERFACES; i++) {
    portArrayIndex[i] = rhs.portArrayIndex[i];
    for (j = 0; j < NUM_PORTS; j++)
      portArray[i][j] = rhs.portArray[i][j];
  }
}


PortStatus & PortStatus::operator=(const PortStatus & rhs) {
  int i,j;
  for (i = 0; i < NUM_IP_INTERFACES; i++) {
    portArrayIndex[i] = rhs.portArrayIndex[i];
    for (j = 0; j < NUM_PORTS; j++)
      portArray[i][j] = rhs.portArray[i][j];
  }
  return *this;
}

int PortStatus::FindFreePort(IPAddress ip, unsigned sockfd) {
  int i, j;
  if (sockfd >= NUM_SOCKS)
    return -1;
  for (i = 0; i < NUM_IP_INTERFACES; i++)
    if (portArrayIndex[i] == ip)
      break;
  if (i < NUM_IP_INTERFACES)
    for (j = 1; j < NUM_PORTS; j++)
      if (portArray[i][j] == 0) {
	portArray[i][j] = sockfd;
	return j;
      }
  return -1;
}

int PortStatus::Socket(IPAddress ip, unsigned port) {
  int i;
  if (port >= NUM_PORTS)
    return -1;
  for (i = 0; i < NUM_IP_INTERFACES; i++)
    if (portArrayIndex[i] == ip)
      break;
  if (i < NUM_IP_INTERFACES)
    return(portArray[i][port]);
  return -1;
}

int PortStatus::AssignPort(IPAddress ip, unsigned port, unsigned sockfd) {
  int i;
  if ((port >= NUM_PORTS) || (port < 1) || (sockfd >= NUM_SOCKS))
    return -1;
  for (i = 0; i < NUM_IP_INTERFACES; i++)
    if (portArrayIndex[i] == ip)
      break;
  if ((i < NUM_IP_INTERFACES) && (portArray[i][port] == 0)) {
    portArray[i][port] = sockfd;
    return sockfd;
  }
  return -1;
}


RequestRecord::RequestRecord (SockRequestResponse *s, int fd) :
    srr(s), sock(fd)
{}


RequestRecord::RequestRecord (const RequestRecord &rhs) :
    srr(rhs.srr), sock(rhs.sock)
{}


RequestRecord & RequestRecord::operator=(const RequestRecord & rhs) {
  srr = rhs.srr;
  sock = rhs.sock;
  return *this;
}


QueueElt::QueueElt(const QueueElt &rhs) :
  data(rhs.data),
  front(rhs.front),
  back(rhs.back)
{}


QueueElt::QueueElt(char *d, QueueElt *f, QueueElt *b) :
  data(d),
  front(f),
  back(b)
{}


QueueElt & QueueElt::operator=(const QueueElt & rhs) {
  data = rhs.data;
  front = rhs.front;
  back = rhs.back;
  return *this;
}


Queue::Queue() :
  front(NULL),
  back(NULL)
{}


Queue::Queue(const Queue &rhs) :
  front(rhs.front),
  back(rhs.back)
{}


Queue::~Queue() {
  QueueElt *t;
  while (front != NULL) {
    t = front;
    front = front->front;
    delete t;
  }
}


Queue & Queue::operator=(const Queue & rhs) {
  front = rhs.front;
  back = rhs.back;
  return *this;
}


void Queue::Insert(void * d) {
  QueueElt *q=new QueueElt;
  q->data = d;
  q->front = back;
  q->back = NULL;
  if (back != NULL)
    back->back = q;
  else
    front = q;
  back = q;
}


void * Queue::Remove() {
  if (front == NULL)
    return NULL;
  void * ret = front->data;
  QueueElt * d = front;
  front = front->back;
  if (front != NULL)
    front->front = NULL;
  else
    back = NULL;
  delete d;
  return ret;
}

