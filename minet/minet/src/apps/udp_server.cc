#include <stdlib.h>
#include <iostream>
#include <ctype.h>
#include "minet_socket.h"

using std::cout;
using std::cerr;
using std::endl;

void usage()
{
  cerr << "udp_server k|u port\n";
}

int main(int argc, char *argv[])
{
  char buf[512];
  sockaddr_in server_sa;
  int fd, rc;

  if (argc!=3) {
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

  bzero(&server_sa,sizeof(server_sa));
  server_sa.sin_family=AF_INET;
  server_sa.sin_addr.s_addr=htonl(INADDR_ANY);
  server_sa.sin_port=htons(atoi(argv[2]));

  fd = minet_socket(SOCK_DGRAM);

  if (fd<0) {
    cerr << "Can't create socket.\n";
    minet_perror("reason:");
    goto err;
  } else {
    cerr << "Socket created.\n";
  }

  if (minet_bind(fd,&server_sa)<0) {
    cerr << "Can't bind socket.\n";
    minet_perror("reason:");
    goto err;
  } else {
    cerr << "Socket bound.\n";
  }

  cout << "Beginning to echo text.\n";

  while (1) {
    if ((rc=minet_read(fd,buf,512))<0) {
      cerr << "Read failed.\n";
      minet_perror("reason:");
      goto err;
    }
    if (rc==0) {
      cerr << "Done.\n";
      goto done;
    }
    if (write(fileno(stdout),buf,rc)<0) {
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

