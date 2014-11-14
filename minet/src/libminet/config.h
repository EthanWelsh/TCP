#ifndef _config
#define _config

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

//Ethernet configuration 
#define ETHERNET_READER_ON 1
#define ETHERNET_WRITER_ON 1

#define ETHERNET_DEVICE     "eth0" 
#define ETHERNET_HEADER_LEN 14
#define ETHERNET_DATA_MIN   46
#define ETHERNET_DATA_MAX   1500

#define ETHERNET_PACKET_LEN (ETHERNET_HEADER_LEN + ETHERNET_DATA_MAX)

#define ETHERNET_SIGNAL SIGUSR1

extern char * READER_BINARY;
extern char * READER_ARGS[];

extern char * WRITER_BINARY;
extern char * WRITER_ARGS[];

#define WRITER_OK   0

#define LIBNET_ERRORBUF_SIZE 256

#define PACKET_BUFFER_SIZE 256

#define BUFFER_EXCEPTION_SIZE 80

const int DEFAULT_BUFFER_SIZE = 1024;

const char ether2mux_fifo_name[] = "./fifos/ether2mux";
const char mux2ether_fifo_name[] = "./fifos/mux2ether";

const char mux2ip_fifo_name[] = "./fifos/mux2ip";
const char ip2mux_fifo_name[] = "./fifos/ip2mux";

const char mux2arp_fifo_name[] = "./fifos/mux2arp";
const char arp2mux_fifo_name[] = "./fifos/arp2mux";

const char mux2other_fifo_name[] = "./fifos/mux2other";
const char other2mux_fifo_name[] = "./fifos/other2mux";

const char ip2arp_fifo_name[] = "./fifos/ip2arp";
const char arp2ip_fifo_name[] = "./fifos/arp2ip";

const char ip2ipmux_fifo_name[] = "./fifos/ip2ipmux";
const char ipmux2ip_fifo_name[] = "./fifos/ipmux2ip";

const char udp2ipmux_fifo_name[] = "./fifos/udp2ipmux";
const char ipmux2udp_fifo_name[] = "./fifos/ipmux2udp";

const char tcp2ipmux_fifo_name[] = "./fifos/tcp2ipmux";
const char ipmux2tcp_fifo_name[] = "./fifos/ipmux2tcp";

const char icmp2ipmux_fifo_name[] = "./fifos/icmp2ipmux";
const char ipmux2icmp_fifo_name[] = "./fifos/ipmux2icmp";

const char other2ipmux_fifo_name[] = "./fifos/other2ipmux";
const char ipmux2other_fifo_name[] = "./fifos/ipmux2other";

const char udp2sock_fifo_name[] = "./fifos/udp2sock";
const char sock2udp_fifo_name[] = "./fifos/sock2udp";

const char tcp2sock_fifo_name[] = "./fifos/tcp2sock";
const char sock2tcp_fifo_name[] = "./fifos/sock2tcp";

const char icmp2sock_fifo_name[] = "./fifos/icmp2sock";
const char sock2icmp_fifo_name[] = "./fifos/sock2icmp";

const char ipother2sock_fifo_name[] = "./fifos/ipother2sock";
const char sock2ipother_fifo_name[] = "./fifos/sock2ipother";

const char other2sock_fifo_name[] = "./fifos/other2sock";
const char sock2other_fifo_name[] = "./fifos/sock2other";

const char app2sock_fifo_name[] = "./fifos/app2sock";
const char sock2app_fifo_name[] = "./fifos/sock2app";

const char sock2socklib_fifo_name[] = "./fifos/sock2socklib";
const char socklib2sock_fifo_name[] = "./fifos/socklib2sock";

const char reader2mon_fifo_name[]   = "./fifos/reader2mon";
const char writer2mon_fifo_name[]   = "./fifos/writer2mon";
const char ether2mon_fifo_name[]    = "./fifos/ether2mon";
const char ethermux2mon_fifo_name[] = "./fifos/ethermux2mon";
const char arp2mon_fifo_name[]      = "./fifos/arp2mon";
const char ip2mon_fifo_name[]       = "./fifos/ip2mon";
const char other2mon_fifo_name[]    = "./fifos/other2mon";
const char ipmux2mon_fifo_name[]    = "./fifos/ipmux2mon";
const char udp2mon_fifo_name[]      = "./fifos/udp2mon";
const char tcp2mon_fifo_name[]      = "./fifos/tcp2mon";
const char icmp2mon_fifo_name[]     = "./fifos/icmp2mon";
const char ipother2mon_fifo_name[]  = "./fifos/ipother2mon";
const char sock2mon_fifo_name[]     = "./fifos/sock2mon";
const char socklib2mon_fifo_name[]  = "./fifos/socklib2mon";
const char app2mon_fifo_name[]      = "./fifos/app2mon";


// shims
// the rule with shims is that the upper module to shim connection
// uses the fifos that it would usually use to talk to the lower module
// the lower module to shim connection uses the shim fifos
// Conceptually, then, the tcp_shim "belongs to" tcp_module
//
// The rule is that when connecting to an upper module, the
// lower module should check to see if the datapath is named in
// MINET_SHIMS if it is, then it should connect to the shim instead of to the
// upper module
//
// the shim fifo name is the standard fifo name with shim_ prefixed
// 
// ./fifos/tcp2sock -> ./fifos/tcp2sock_shim
//


#endif
