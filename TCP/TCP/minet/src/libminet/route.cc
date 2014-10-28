#include "route.h"


#define	NET		0
#define GATEWAY		1
#define MASK		2
#define FLAGS		3
#define METRIC		4
#define REF		5
#define USE		6
#define IFACE		7
#define BUFFER		100

using std::cout;
using std::endl;


// Initializing a route table
route_table_t *make_route_table(void)
{
  route_table_t *route_q = (route_table_t *)malloc(sizeof(route_table_t));
  route_q->first = NULL;
  route_q->last = NULL;
  route_q->deflt = NULL;

  return route_q;
}



// Making a route
route_t *make_route(char *net, char *mask, char *iface, char *gateway,        \
		    char *flags, char *metric, char *ref, char *use)
{
  route_t *route_entry = (route_t *)malloc(sizeof(route_t));

  route_entry->net = (char *)malloc(BUFFER*sizeof(char));
  route_entry->mask = (char *)malloc(BUFFER*sizeof(char));
  route_entry->iface = (char *)malloc(BUFFER*sizeof(char));
  route_entry->gateway = (char *)malloc(BUFFER*sizeof(char));
  route_entry->flags = (char *)malloc(BUFFER*sizeof(char));
  route_entry->metric = (char *)malloc(BUFFER*sizeof(char));
  route_entry->ref = (char *)malloc(BUFFER*sizeof(char));
  route_entry->use = (char *)malloc(BUFFER*sizeof(char));

  strcpy(route_entry->net, net);
  strcpy(route_entry->mask, mask);
  strcpy(route_entry->iface, iface);
  strcpy(route_entry->gateway, gateway);
  strcpy(route_entry->flags, flags);
  strcpy(route_entry->metric, metric);
  strcpy(route_entry->ref, ref);
  strcpy(route_entry->use, use);
  route_entry->next = NULL;
  route_entry->previous = NULL;

  return route_entry;
}



// Initial loading of route table from a given file
void load_routes(route_table_t *table, const char *filename)
{
  FILE		*f;
  int		type = 0;
  int		not_entry = 1;
  char		net[BUFFER];
  char		mask[BUFFER];
  char		iface[BUFFER];
  char		gateway[BUFFER];
  char		flags[BUFFER];
  char		metric[BUFFER];
  char		ref[BUFFER];
  char		use[BUFFER];


  cout << "Loading route table from " << filename << endl;
  if((f = fopen(filename, "r")) == NULL)
    cout << filename << " cannot be opened" << endl;

  fseek(f, 0, SEEK_SET);

  while(!feof(f)) {
    if(not_entry) {
      fscanf(f, "%s", net);
      if((strchr(net, '.') != NULL) || (strcmp(net, "default") == 0)) {
        not_entry = 0;
        type++;
      }
    }
    else {
      switch(type) {			// Once the pointer gets into the table
        case NET: {
          fscanf(f, "%s", net);
        } break;
        case GATEWAY: {
          fscanf(f, "%s", gateway);
        } break;
        case MASK: {
          fscanf(f, "%s", mask);
        } break;
        case FLAGS: {
          fscanf(f, "%s", flags);
        } break;
        case METRIC: {
          fscanf(f, "%s", metric);
        } break;
        case REF: {
          fscanf(f, "%s", ref);
        } break;
        case USE: {
          fscanf(f, "%s", use);
        } break;
        case IFACE: {
          fscanf(f, "%s", iface);
        } break;
        default: {}
      }

      type++;
      if(type == 8) {  			// Finish reading one row
        type = 0;
        add_route(table, net, gateway, mask, flags, 				\
		  metric, ref, use, iface);
      }
    }
  }

  fclose(f);
  return;
}



// Checking if a route_table is empty
// if empty return 1
// if not empty return 0
int is_empty(route_table_t *table)
{
  if(table->first == NULL)
    return 1;

  return 0;
}



// Adding a route to the route table
void add_route(route_table_t *table, char *net, char *gateway, char *mask,	\
	       char *flags, char *metric, char *ref,				\
       	       char *use, char *iface)
{
  route_t *current = (route_t *)malloc(sizeof(route_t));
  current = make_route(net, mask, iface, gateway, flags, metric, ref, use);

  if(strcmp(current->net, "default") == 0)
    table->deflt = current;

  if(is_empty(table)) {
    table->first = current;
    table->last = current;
  }
  else {
    current->previous = table->last;
    table->last->next = current;
	table->last = current;
  }
  return;
}



// Looking for a match
// Criteria is longest match
// If no match use default
// If two routes have the same match length just take the first occurance
route_t *match_route(route_table_t *table, char *net_addr)
{
  int match_count = 0;
  int max_match = 0;
  route_t *matched = (route_t *)malloc(sizeof(route_t));
  route_t *current = (route_t *)malloc(sizeof(route_t));

  current = table->first;
  while(current->next != NULL) {
    if(strcmp(current->net, net_addr) == 0) {
      matched = current;
      free(current);
      return matched;
    }
    else {
      match_count = match_func(current->net, net_addr);
      if(match_count > max_match) {
	max_match = match_count;
	matched = current;
      }
    }
    current = current->next;
  }

  if(max_match == 0)
    matched = table->deflt;
  return matched;
}



// Deleting an entry from the route table
void del_route(route_table_t *table, char *net_addr)
{
  route_t *current = (route_t *)malloc(sizeof(route_t));

  current = table->first;

  while(current != NULL) {
    if(strcmp(net_addr, current->net) == 0) {
      if(current == table->first) {
        table->first = table->first->next;
        table->first->previous = NULL;
      }
      else if (current == table->last) {
        table->last = table->last->previous;
        table->last->next = NULL;
      }
      else {
        current->previous->next = current->next;
	current->next->previous = current->previous;
      }
      free(current);
      return;
    }

    current = current->next;
  }
  cout << "Network address " << net_addr << " not in the routing table" << endl;
  return;
}



// Printing routes in the route table
void print_route(route_table_t *table)
{
  route_t *current = (route_t *)malloc(sizeof(route_t));

  current = table->first;
  if(current == NULL) {
    cout << "Route table is empty" << endl;
    free(current);
    return;
  }

  cout << "\nNet\t\t\tGateway\t\tMask\t\tFlag Met Ref Use Iface" << endl;
  while(current != NULL) {
    printf("%15s %18s %18s %5s %3s %3s %3s %5s\n", current->net,	        \
	   current->gateway, current->mask, current->flags, 			\
	   current-> metric, current->ref, current->use, current->iface);

    current = current->next;
  }

  free(current);
  return;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

// Matching function to be used by match_route
// Count the number of dots in the network address
// up to which the addresses still match
// As soon as a mismatch occur, stop
int match_func(const char *entry_net, const char *net)
{
  int i = 0;
  int match_count = 0;
  int str_len = (((strlen(entry_net)) < (strlen(net))) ?			\
		  (strlen(entry_net)) : (strlen(net)));

  while(i <str_len) {
    if(entry_net[i] == '.') {
      match_count++;
      i++;
    }
    else if(entry_net[i] != net[i])
      return match_count;

    i++;
  }

  return match_count;
}


char *ipToString(IPAddress ip)
{
  struct in_addr addr;
  addr.s_addr = htonl(ip.addr);
  return inet_ntoa(addr);
}


char *ethToString(EthernetAddr eth)
{
  EthernetAddrString *s = (EthernetAddrString *)malloc(sizeof(EthernetAddrString));
  eth.GetAsString(*s);

  return (char *)s;
}
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

if_list_t *make_if_list(void)
{
  if_list_t *if_list = (if_list_t *)malloc(sizeof(if_list_t));
  if_list->first = NULL;
  if_list->last = NULL;

  return if_list;
}


intface_t *make_intface(char *name, char *status, char *IPAddr,    \
			char *NetAddr)
{
  intface_t *intface = (intface_t *)malloc(sizeof(intface_t));
  intface->name = (char *)malloc(BUFFER*sizeof(char));
  intface->status = (char *)malloc(BUFFER*sizeof(char));
  intface->IPAddr = (char *)malloc(BUFFER*sizeof(char));
  intface->NetAddr = (char *)malloc(BUFFER*sizeof(char));

  strcpy(intface->name, name);
  strcpy(intface->status, status);
  strcpy(intface->IPAddr, IPAddr);
  strcpy(intface->NetAddr, NetAddr);

  return intface;
}


void add_intface(if_list_t *if_list, char *name, char *status, char *IPAddr,    \
			char *NetAddr)
{
  intface_t *current = (intface_t *)malloc(sizeof(intface_t));
  current = make_intface(name, status, IPAddr, NetAddr);

  if(is_empty_list(if_list)) {
    if_list->first = current;
    if_list->last = current;
  }
  else {
    current->previous = if_list->last;
    if_list->last->next = current;
    if_list->last = current;
  }
  return;
}


void del_intface(if_list_t *if_list, char *name)
{
  intface_t *current = (intface_t *)malloc(sizeof(intface_t));

  current = if_list->first;

  while(current != NULL) {
    if(strcmp(name, current->name) == 0) {
      if(current == if_list->first) {
	if_list->first = if_list->first->next;
        if_list->first->previous = NULL;
      }
      else if (current == if_list->last) {
        if_list->last = if_list->last->previous;
        if_list->last->next = NULL;
      }
      else {
        current->previous->next = current->next;
	current->next->previous = current->previous;
      }
      free(current);
      return;
    }

    current = current->next;
  }
  cout << "interface " << name << " not in the list" << endl;
  return;
}


int is_empty_list(if_list_t *if_list)
{
  if(if_list->first == NULL)
    return 1;

  return 0;
}


void print_if_list(if_list_t *if_list)
{
  intface_t *current = (intface_t *)malloc(sizeof(intface_t));

  current = if_list->first;
  if(current == NULL) {
    cout << "Interface list is empty" << endl;
    free(current);
    return;
  }

  cout << "\n\nInterface\tStatus\t\tIPAddr\t\tNetAddr" << endl;
  while(current != NULL) {
    printf("%10s %10s %18s %25s\n", current->name, current->status, current->IPAddr,  \
	   current->NetAddr);

    current = current->next;
  }

  free(current);
  return;
}
