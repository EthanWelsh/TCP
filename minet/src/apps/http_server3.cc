#include "minet_socket.h"
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#define FILENAMESIZE 100
#define BUFSIZE 1024

typedef enum { NEW,
	       READING_HEADERS,
	       WRITING_RESPONSE,
	       READING_FILE,
	       WRITING_FILE,
	       CLOSED } states;

typedef struct connection_s connection;

struct connection_s {
    int sock;
    int fd;
    char filename[FILENAMESIZE + 1];
    char buf[BUFSIZE + 1];
    char * endheaders;
    bool ok;
    long filelen;
    states state;

    int headers_read;
    int response_written;
    int file_read;
    int file_written;

    connection * next;
};

void read_headers(connection * con);
void write_response(connection * con);
void read_file(connection * con);
void write_file(connection * con);

int main(int argc, char * argv[]) {
    int server_port = -1;

    /* parse command line args */
    if (argc != 3) {
	fprintf(stderr, "usage: http_server3 k|u port\n");
	exit(-1);
    }

    server_port = atoi(argv[2]);

    if (server_port < 1500) {
	fprintf(stderr, "INVALID PORT NUMBER: %d; can't be < 1500\n", server_port);
	exit(-1);
    }
    
    /* initialize and make socket */

    /* set server address*/

    /* bind listening socket */

    /* start listening */

    /* connection handling loop */

    while (1) {
	/* create read and write lists */
	
	/* do a select */
	
	/* process sockets that are ready */
	
    }
}

void read_headers(connection * con) {

    /* first read loop -- get request and headers*/

    /* parse request to get file name */
    /* Assumption: this is a GET request and filename contains no spaces*/

    /* get file name and size, set to non-blocking */

    /* get name */

    /* try opening the file */
    
    /* set to non-blocking, get size */

    write_response(con);
}

void write_response(connection * con) {
    char * ok_response_f = "HTTP/1.0 200 OK\r\n"	\
	"Content-type: text/plain\r\n"			\
	"Content-length: %d \r\n\r\n";
    
    char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"	\
	"Content-type: text/html\r\n\r\n"			\
	"<html><body bgColor=black text=white>\n"		\
	"<h2>404 FILE NOT FOUND</h2>\n"				\
	"</body></html>\n";
    
    /* send response */
  
    /* send headers */
}

void read_file(connection * con) {

}

void write_file(connection * con) {
 
}
