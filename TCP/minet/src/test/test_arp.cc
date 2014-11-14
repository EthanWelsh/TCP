#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "ethernet.h"
#include "ip.h"
#include "arp.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

void usage()
{
  cerr << "test_arp\n";
}


int main(int argc, char *argv[])
{
  int toarp=open(ip2arp_fifo_name,O_WRONLY);
  int fromarp=open(arp2ip_fifo_name,O_RDONLY);

  if (toarp<0 || fromarp<0) {
    cerr << "Can't connect.\n";
    exit(-1);
  }


  cerr << "test_arp: ready for queries\n";

  while (1) {
    char buf[1024];
    cin >> buf;

    ARPRequestResponse r;

    r.ipaddr = IPAddress(buf);
    r.ethernetaddr = ETHERNET_BLANK_ADDR;
    r.flag = ARPRequestResponse::REQUEST;

    cout << "Request:  " << r << "\n";

    r.Serialize(toarp);
    r.Unserialize(fromarp);

    cout << "Response: " << r << "\n";
  }

  return 0;
}
