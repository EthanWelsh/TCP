#include <iostream>
#include "minet_socket.h"

#include "Minet.h"

using std::cerr;

int main() {
    MinetInit(MINET_SOCKLIB_MODULE);
    MinetConnect(MINET_SOCK_MODULE);

    while (1) {
	cerr << "la ";
	sleep(1);
    }
}

