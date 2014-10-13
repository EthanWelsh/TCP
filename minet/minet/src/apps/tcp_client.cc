#include <stdlib.h>
#include <iostream>
#include <ctype.h>
#include "minet_socket.h"


using namespace std;


void usage() {
    cerr << "tcp_client k|u host port" << endl;
}

void die(int fd) {
    minet_close(fd);
    minet_deinit();
    exit(0);
}

int main(int argc, char * argv[]) {
    char buf[512];
    sockaddr_in server_sa;
    sockaddr_in client_sa;
    struct hostent * he = NULL;
    int fd = 0;
    int rc = 0;

    if (argc != 4) {
	usage();
	exit(-1);
    }

    if (toupper(argv[1][0]) == 'K') {
	cerr << "Using kernel stack." << endl;

	if (minet_init(MINET_KERNEL) < 0) {
	    cerr << "Stack initialization failed." << endl;
	    die(fd);
	} 
    } else {
	cerr << "Using Minet User Level Stack." << endl;
	if (minet_init(MINET_USER) < 0) {
	    cerr << "Stack initialization failed." << endl;
	    die(fd);
	} 
    }

   cerr << "Stack initialized." << endl;

    he = gethostbyname(argv[2]);

    if (he == 0) {
	cerr << "Unknown host." << endl;
	die(fd);
    }

    bzero(&server_sa, sizeof(server_sa));
    server_sa.sin_family = AF_INET;
    memcpy((void *)(&(server_sa.sin_addr)), he->h_addr, he->h_length);
    server_sa.sin_port = htons(atoi(argv[3]));

    fd = minet_socket(SOCK_STREAM);

    if (fd < 0) {
	cerr << "Can't create socket." << endl;
	minet_perror("reason:");
	die(fd);
    } 
	
    cerr << "Socket created." << endl;

    bzero(&client_sa, sizeof(client_sa));
    client_sa.sin_family = AF_INET;
    client_sa.sin_addr.s_addr = htonl(INADDR_ANY);
    client_sa.sin_port = htons(0);

    if (minet_bind(fd, &client_sa) < 0) {
	cerr << "Can't bind socket." << endl;
	minet_perror("reason:");
	die(fd);
    }

    cerr << "Socket bound." << endl;

    if (minet_connect(fd, &server_sa) < 0) {
	cerr << "Can't connect socket." << endl;
	minet_perror("reason:");
	die(fd);
    }

    cerr << "Socket connected." << endl;
    cout << "Enter text to send." << endl;

    while (1) {

	if ((rc = read(fileno(stdin), buf, 512)) < 0) {
	    cerr << "Read failed." << endl;
	    minet_perror("reason:");
	    break;
	}

	if (rc == 0) {
	    cerr << "Done." << endl;
	    break;
	}

	if (minet_write(fd, buf, rc) < 0) {
	    cerr << "Write failed." << endl;
	    minet_perror("reason:");
	    break;
	}
    }

    die(fd);

    return 0;
}

