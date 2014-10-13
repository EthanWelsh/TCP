#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <ctype.h> 
#include <netinet/in.h>



int readall(const int fd, char *buf, const int len, const int oneshot, const int awaitblock)
{
  int rc;
  int left;

  left=len;
  while (left>0) {
    rc=read(fd,&(buf[len-left]),left);
    if (oneshot) { 
      return rc;
    }
    if (rc==0) {
      return len-left;
    }
    if (rc<0) { 
      if (errno==EINTR) {
	continue;
      }
      if (errno==EWOULDBLOCK && awaitblock) {
	continue;
      }
      return rc;
    } else {
      left-=rc;
    }
  }
  return len;
}


int writeall(const int fd, const char *buf, const int len, const int oneshot, const int awaitblock)
{
  int rc;
  int left;

  left=len;
  while (left>0) {
    rc=write(fd,&(buf[len-left]),left);
    if (oneshot) { 
      return rc;
    }
    if (rc==0) { 
      return len-left;
    }
    if (rc<0) { 
      if (errno==EINTR) {
	continue;
      }
      if (errno==EWOULDBLOCK && awaitblock) {
	continue;
      }
      return rc;
    } else {
      left-=rc;
    }
  }
  return len;
}


void printhexnybble(FILE *out,const char lower)
{
  fputc(lower>=10 ? lower-10 + 'A' : lower+'0', out);
}

void printhexbyte(FILE *out,const char h)
{
  char upper=(h>>4)&0xf;
  char lower=h&0xf;

  printhexnybble(out,upper); printhexnybble(out,lower);
}

void printhexbuffer(FILE *out, const char *buf, const int len)
{
  int i;
  for (i=0;i<len;i++) { 
    printhexbyte(out,buf[i]);
  }
}

void printhexshort(FILE *out, const short s)
{
  printhexbuffer(out, (char*)&s,2);
}

void printhexint(FILE *out, const int i)
{
  printhexbuffer(out,(char*)&i,4);
}


char hexnybbletonybble(const char hexnybble)
{
  char x = toupper(hexnybble);
  if (x>='0' && x<='9') {
    return x-'0';
  } else {
    return 10+(x-'A');
  }
}

void hexbytetobyte(const char hexbyte[2], char *byte)
{
  *byte=(hexnybbletonybble(hexbyte[0])<<4)+(hexnybbletonybble(hexbyte[1])&0xf);
}

char nybbletohexnybble(const char nybble)
{
  return nybble>=10 ? nybble-10 + 'A' : nybble+'0';
}

void bytetohexbyte(const char byte, char hexbyte[2])
{
  hexbyte[0]=nybbletohexnybble((byte>>4)&0xf);
  hexbyte[1]=nybbletohexnybble(byte&0xf);
}

void IPAddressToString(unsigned adx, char buf[16])
{
  snprintf(buf,16,"%3d.%3d.%3d.%3d", (adx>>24)&0xff,
	  (adx>>16)&0xff, (adx>>8)&0xff, (adx)&0xff);
}


#include <sys/poll.h>
#include <errno.h>

bool CanWriteNow(const int fd)
{
#if 1
  return true;
#else
  struct pollfd pfd;
  int rc;

  pfd.fd=fd;
  pfd.events=POLLOUT;
  pfd.revents=0;

  while (1) { 
    rc=poll(&pfd,1,0);
    if (rc<0 && errno!=EINTR) {
      perror("poll");
      return false;
    }
    if (rc==0) { 
      return false;
    }
    if (rc>0) { 
      return (pfd.revents & POLLOUT)!=0;;
    }
  } 
  return false;
#endif
}

bool CanReadNow(const int fd)
{
#if 1
  return true;
#else
  struct pollfd pfd;
  int rc;

  pfd.fd=fd;
  pfd.events=POLLIN;
  pfd.revents=0;

  while (1) { 
    rc=poll(&pfd,1,0);
    if (rc<0 && errno!=EINTR) {
      perror("poll");
      return false;
    }
    if (rc==0) { 
      return false;
    }
    if (rc>0) { 
      return (pfd.revents & POLLIN)!=0;;
    }
  } 
  return false;
#endif
}


bool WaitForRead(const int fd)
{
  struct pollfd pfd;
  int rc;

  pfd.fd=fd;
  pfd.events=POLLIN;
  pfd.revents=0;

  while (1) { 
    rc=poll(&pfd,1,-1);
    if (rc<0) { 
      if (errno!=EINTR) { 
	perror("poll");
	return false;
      } else {
	continue;
      }
    } else if (rc==0) {
      return false;
    } else {
      return true;
    }
  } 
  return false;
}


bool WaitForWrite(const int fd)
{
  struct pollfd pfd;
  int rc;

  pfd.fd=fd;
  pfd.events=POLLOUT;
  pfd.revents=0;

  while (1) { 
    rc=poll(&pfd,1,-1);
    if (rc<0) { 
      if (errno!=EINTR) { 
	perror("poll");
	return false;
      } else {
	continue;
      }
    } else if (rc==0) {
      return false;
    } else {
      return true;
    }
  } 
  return false;
}


unsigned short OnesComplementSum(unsigned short *buf, int len)
{
  unsigned int sum, sum2, sum3;
  unsigned short realsum;
  int i;

  sum=0;
  for (i=0;i<len;i++) {
    sum+=ntohs(buf[i]);
  }
  // assume there is no carry out, so now...

  sum2 = (sum&0x0000ffff) + ((sum&0xffff0000)>>16);

  sum3 = (sum2&0x0000ffff) +((sum2&0xffff0000)>>16);

  realsum=sum3;

  return realsum;
}  
