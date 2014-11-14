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
#include <arpa/inet.h>
#include <sys/socket.h>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

int fromsock, tosock;     //fds for fifos

int listenForSLRR(SockLibRequestResponse &s, int timeout) {
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

void buildSLRR(SockLibRequestResponse &s) {
  int menu_ret = 0;
  int submenu;
  char stemp[32], dtemp[32];
  unsigned short sport, dport;
  unsigned sock;
  unsigned char proto;
  std::string data;
  int length, errcode;
  IPAddress srcip, destip;
  Connection temp;
  int test;
  char testc;

  cout << "this is the current outbound slrr: " << endl;
  cout << s << endl;

  while (!menu_ret) {

    cout << endl << "pick:\n1) modify type\n2) modify connection\n3) modify data\n"
	 << "4) modify bytes (danger!)\n5) modify error\n6) modify sock\n"
	 << "7) exit submenu" << endl << "?: ";

    test = cin.peek();
    //    cerr << endl << "test is " << test << endl;

    if (test < 48 || test > 57)
      test = cin.get();

    //    cerr << endl << "test 2 is " << test << endl;
    cin >> menu_ret;

    if (menu_ret<1 || menu_ret>7) {
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
	   << " 1) mSOCKET            2) mBIND             3) mLISTEN\n"
	   << " 4) mACCEPT            5) mCONNECT          6) mREAD\n"
	   << " 7) mWRITE             8) mRECVFROM         9) mSENDTO\n"
	   << "10) mCLOSE            11) mSELECT          12) mPOLL\n"
	   << "13) mSET_BLOCKING     14) mSET_NONBLOCKING 15) mCAN_WRITE_NOW\n"
	   << "16) mCAN_READ_NOW     17) mSTATUS";
      cin >> submenu;
      if (submenu<1 || submenu>17)
	submenu = 0;
    }
    s.type = (slrrType)(submenu - 1);
    buildSLRR(s);
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
    buildSLRR(s);
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
    buildSLRR(s);
    break;
  case 4:
    cout << "enter new length: ";
    cin >> length;
    s.bytes = length;
    buildSLRR(s);
    break;
  case 5:
    cout << "enter new error code: ";
    cin >> errcode;
    s.error = errcode;
    buildSLRR(s);
    break;
  case 6:
    cout << "enter new sock: ";
    cin >> sock;
    s.sockfd = sock;
    buildSLRR(s);
  case 7:
    return;
  }
}

int main(int argc, char *argv[]) {

  SockLibRequestResponse inbound;

  Buffer garbage("this is garbage text", 21);
  Connection con(IPAddress("10.10.10.10"), IP_ADDRESS_ANY, 1999, 1999, IP_PROTO_TCP);
  SockLibRequestResponse outbound(mSTATUS, con, 0, garbage, garbage.GetSize(), 0);

  fromsock=open(sock2app_fifo_name,O_RDONLY);
  tosock=open(app2sock_fifo_name,O_WRONLY);

  if (tosock<0 || fromsock<0) {
    cerr << "Can't open connection to sock module\n";
    return -1;
  }
  cout << "socket module tester: app dummy" << endl;

  while (1) {

    int menu_select = 0;
    while (!menu_select) {
      cout << "select option:\n"
	   << "1) send slrr\n2) listen for slrr"
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
	cout << endl << "1) edit current slrr or\n2) send current slrr" << endl << "?: ";
	cin >> in;
	if (in<1 || in>2)
	  in = -1;
      }
      if (in==1)
	buildSLRR(outbound);
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
      rc = listenForSLRR(inbound, in);
      if (rc<0)
	cerr << "unexpected error\n";
      else if (rc==0)
	cout << "timeout occured, no slrr received" << endl;
      else {
	  cout << "got this from sock_mod: " << endl;
	  inbound.Print(cout);
	}
      break;
    }
  }
}
