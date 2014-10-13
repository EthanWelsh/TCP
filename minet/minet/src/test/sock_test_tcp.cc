#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sockint.h"
#include "config.h"
#include <iostream>
#include "ip.h"
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

int fromsock, tosock;     //fds for fifos

int listenForSRR(SockRequestResponse &s, int timeout) {
  fd_set read_fds;
  int rc, maxfd;
  struct timeval tv;

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  FD_ZERO(&read_fds);
  FD_SET(fromsock, &read_fds);
  maxfd = fromsock;

  rc = select(maxfd+1, &read_fds, 0, 0, &tv);
  if (rc<0) {
    if (errno!=EINTR);
    return -1;
  }
  else if (rc==0)
    return 0;
  else if (FD_ISSET(fromsock, &read_fds)) {
    s.Unserialize(fromsock);
    return 1;
  }
  return -1;
}

void buildSRR(SockRequestResponse &s) {
  int menu_ret = 0;
  int submenu;
  char stemp[32], dtemp[32];
  unsigned short sport, dport;
  unsigned char proto;
  std::string data;
  int length, errcode;
  IPAddress srcip, destip;
  Connection temp;
  int test;
  char testc;

  cout << "this is the current outbound srr: " << endl;
  cout << s << endl;

  while (!menu_ret) {

    cout << endl << "pick:\n1) modify type\n2) modify connection\n3) modify data\n"
	 << "4) modify bytes (danger!)\n5) modify error\n6) exit submenu" << endl << "?: ";

    test = cin.peek();
    //    cerr << endl << "test is " << test << endl;

    if (test < 48 || test > 57)
      test = cin.get();

    //    cerr << endl << "test 2 is " << test << endl;
    cin >> menu_ret;

    if (menu_ret<1 || menu_ret>6) {
      test = cin.peek();
      //      cerr << endl << "test is " << test;
      if (test == -1)
	cin >> testc;
      menu_ret = 0;
      sleep(2);
    }
  }

  switch (menu_ret) {

  case 1:
    submenu = 0;
    while (!submenu) {
    cout << "modify type:\n"
	 << "1) ACCEPT\n2) CONNECT\n3) WRITE\n4) FORWARD\n5) CLOSE\n6) STATUS" << endl
	 << "?: ";
    cin >> submenu;
    if (submenu<1 || submenu>6)
      submenu = 0;
    }
    switch (submenu) {
    case 1:
      s.type = ACCEPT;
      break;
    case 2:
      s.type = CONNECT;
      break;
    case 3:
      s.type = WRITE;
      break;
    case 4:
      s.type = FORWARD;
      break;
    case 5:
      s.type = CLOSE;
      break;
    case 6:
      s.type = STATUS;
      break;
    }
    buildSRR(s);
    break;

  case 2:
    cout << "enter new connection data:" << endl;
    cout << "source IP: ";
    cin >> stemp;
    cout << "source port: ";
    cin >> sport;
    cout << "destination IP: ";
    cin >> dtemp;
    cout << "destination port: ";
    cin >> dport;

    submenu = 0;
    while (!submenu) {
      cout << "pick a protocol:\n1) IP\n2) UDP\n3) TCP\n4) ICMP\n5) RAW\n" << endl << "?: ";
      cin >> submenu;
      if (submenu<1 || submenu>5)
	submenu = 0;
    }

    switch (submenu) {
    case 1:
      proto = IP_PROTO_IP;
      break;
    case 2:
      proto = IP_PROTO_UDP;
      break;
    case 3:
      proto = IP_PROTO_TCP;
      break;
    case 4:
      proto = IP_PROTO_ICMP;
      break;
    case 5:
      proto = IP_PROTO_RAW;
      break;
    }
    srcip = IPAddress(stemp);
    destip = IPAddress(dtemp);
    temp = Connection(srcip, destip, sport, dport, proto);
    s.connection = temp;
    buildSRR(s);
    break;

  case 3:
    cout << "enter new data payload: ";
    test = cin.peek();
    if (test==10)
      cin.get();
    getline(cin, data);
    s.data.Clear();
    s.data.SetData(data.c_str(), data.length(), 0);
    s.bytes = s.data.GetSize();
    buildSRR(s);
    break;
  case 4:
    cout << "enter new length: ";
    cin >> length;
    s.bytes = length;
    buildSRR(s);
    break;
  case 5:
    cout << "enter new error code: ";
    cin >> errcode;
    s.error = errcode;
    buildSRR(s);
    break;
  case 6:
    return;
  }
}

int main(int argc, char *argv[]) {

  SockRequestResponse inbound;

  Buffer garbage("this is garbage text", 21);
  Connection con(IPAddress("10.10.10.10"), IP_ADDRESS_ANY, 1999, 1999, IP_PROTO_TCP);
  SockRequestResponse outbound(STATUS, con, garbage, garbage.GetSize(), 0);

  tosock=open(tcp2sock_fifo_name,O_WRONLY);
  fromsock=open(sock2tcp_fifo_name,O_RDONLY);

  cerr << "opened fifos" << endl;

  if (tosock<0 || fromsock<0) {
    cerr << "Can't open connection to sock module\n";
    return -1;
  }

  cout << "socket module tester: tcp dummy" << endl;

  while (1) {

    int menu_select = 0;
    while (!menu_select) {
      cout << "select option:\n"
	   << "1) send srr\n2) listen for srr"
	   << endl << "?: ";
      cin >> menu_select;
      if (menu_select<1 || menu_select>2)
	menu_select = 0;

    }

    int in = -1;
    int rc;

    switch (menu_select) {

    case 1:
      while (in<0) {
	cout << endl << "1) edit current srr or\n2) send current srr" << endl << "?: ";
	cin >> in;
	if (in<1 || in>2)
	  in = -1;
      }
      if (in==1)
	buildSRR(outbound);
      else {
	cout << "this is being serialized to the socket module:" << endl;
	outbound.Print(cout);
	outbound.Serialize(tosock);
	cout << endl;
      }
      break;

    case 2:
      while (in<0) {
	cout << "enter a timeout value in (s), or 0 for no timeout:" << endl << "?: ";
	cin >> in;
	if (in<0)
	  in = -1;
      }
      cout << endl << "listening to sock_mod..." << endl;
      rc = listenForSRR(inbound, in);
      if (rc<0)
	cerr << "unexpected error\n";
      else if (rc==0)
	cout << "timeout occured, no srr received" << endl;
      else {
	  cout << "got this from sock_mod: " << endl;
	  inbound.Print(cout);
	}
      break;
    }
  }
}
