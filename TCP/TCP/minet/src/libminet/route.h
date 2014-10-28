#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ip.h"
#include "ethernet.h"


struct route_t
{
  char   	*net;
  char		*mask;
  char		*iface;
  char		*gateway;
  char		*flags;
  char		*metric;
  char		*ref;
  char		*use;
  route_t	*next;
  route_t	*previous;
};


struct route_table_t
{
  route_t *first;
  route_t *deflt;
  route_t *last;
};


struct intface_t
{
  char      *name;
  char      *status;
  char      *IPAddr;
  char      *NetAddr;
  intface_t *next;
  intface_t *previous;
};


struct if_list_t
{
  intface_t *first;
  intface_t *last;
};


// Function prototypes relating to route_tables
route_table_t *make_route_table(void);
//static route_t *make_route(char *net, char *mask, char *iface, char *gateway,
//			   char *flags, char *metric, char *ref, char *use);
void load_routes(route_table_t *table, const char *filename);
void add_route(route_table_t *table, char *net, char *gateway, char* mask,
		      char * flags, char *mtric, char *ref, char *use, char *iface);
void del_route(route_table_t *table, char *net_addr);
void print_route(route_table_t *table);
route_t	*match_route(route_table_t *table, char *net_addr);
int is_empty(route_table_t *table);



// helper functions
int match_func(const char* entry_net, const char* net);
char *ipToString(IPAddress ip);
char *ethToString(EthernetAddr eth);


// Function prototypes relating to intface
if_list_t *make_if_list(void);
void add_intface(if_list_t *if_list, char *name, char *status, char *IPAddr, char *NetAddr);
void del_intface(if_list_t *if_list, char *name);
int is_empty_list(if_list_t *if_list);
intface_t *make_intface(char *name, char *status, char *IPAddr,                         \
			char *NetAddr);
void print_if_list(if_list_t *if_list);
