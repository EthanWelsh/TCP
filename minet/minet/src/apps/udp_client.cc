#include <stdlib.h>
#include <iostream>
#include <ctype.h>
#include "minet_socket.h"

#define BUFLEN   15000
#define GENERATE 0

using std::cout;
using std::cerr;
using std::endl;

void usage()
{
  cerr << "udp_client k|u host port\n";
}

int main(int argc, char *argv[])
{
  char buf[BUFLEN];
  sockaddr_in server_sa, client_sa;
  struct hostent *he;
  int fd, rc;

  if (argc!=4) {
    usage();
    exit(-1);
  }

  if (toupper(argv[1][0])=='K') {
    cerr << "Using kernel stack.\n";
    if (minet_init(MINET_KERNEL)<0) {
      cerr << "Stack initialization failed.\n";
      goto err;
    } else {
      cerr << "Stack initialized.\n";
    }
  } else {
    cerr << "Using Minet User Level Stack.\n";
    if (minet_init(MINET_USER)<0) {
      cerr << "Stack initialization failed.\n";
      goto err;
    } else {
      cerr << "Stack initialized.\n";
    }
  }

  cerr << "Stack initialized.\n";

  he = gethostbyname(argv[2]);

  if (he==0) {
    cerr << "Unknown host.\n";
    goto err;
  }
  bzero(&server_sa,sizeof(server_sa));
  server_sa.sin_family=AF_INET;
  memcpy((void*)(&(server_sa.sin_addr)),he->h_addr,he->h_length);
  server_sa.sin_port=htons(atoi(argv[3]));

  fd = minet_socket(SOCK_DGRAM);

  if (fd<0) {
    cerr << "Can't create socket.\n";
    minet_perror("reason:");
    goto err;
  } else {
    cerr << "Socket created.\n";
  }

  bzero(&client_sa,sizeof(client_sa));
  client_sa.sin_family=AF_INET;
  client_sa.sin_addr.s_addr=htonl(INADDR_ANY);
  client_sa.sin_port=htons(0);

  if (minet_bind(fd,&client_sa)<0) {
    cerr << "Can't bind socket.\n";
    minet_perror("reason:");
    goto err;
  } else {
    cerr << "Socket bound.\n";
  }

  if (minet_connect(fd,&server_sa)<0) {
    cerr << "Can't connect socket.\n";
    minet_perror("reason:");
    goto err;
  } else {
    cerr << "Socket connected.\n";
  }

  cout << "Enter text to send.\n";

  while (1) {
#if GENERATE
    sleep(1);
    for (int i=0;i<BUFLEN;i++) {
      buf[i]='a'+i%26;
    }
    buf[BUFLEN-1]='\n';
    rc=BUFLEN;
#else
    if ((rc=read(fileno(stdin),buf,BUFLEN))<0) {
      cerr << "Read failed.\n";
      minet_perror("reason:");
      goto err;
    }
#endif
    if (rc==0) {
      cerr << "Done.\n";
      goto done;
    }
    if (minet_write(fd,buf,rc)<0) {
      cerr << "Write failed.\n";
      minet_perror("reason:");
      goto err;
    }
  }

 done:
 err:
  minet_close(fd);
  minet_deinit();
  return 0;
}

