#ifndef _util
#define _util

#include <cstdio>
#include <functional>

//template <class T>
//std::ostream & operator << (std::ostream & os, const T &obj)
//{
//  return obj::operator<<(os);
//};

int readall(const int fd, char *buf, const int len, const int oneshot=0, const int awaitblock=1);
int writeall(const int fd, const char *buf, const int len, const int oneshot=0, const int awaitblock=1);

void printhexnybble(FILE *out,const char lower);
void printhexbyte(FILE *out,const char h);
void printhexshort(FILE *out,const short h);
void printhexint(FILE *out,const int h);
void printhexbuffer(FILE *out, const char *buf, const int len);

void hexbytetobyte(const char hexbyte[2], char *byte);
void bytetohexbyte(const char byte, char hexbyte[2]);

void IPAddressToString(unsigned adx, char buf[16]);


bool CanReadNow(const int fd);
bool CanWriteNow(const int fd);
bool WaitForRead(const int fd);
bool WaitForWrite(const int fd);

unsigned short OnesComplementSum(unsigned short *buf, int len);

struct SerializationException {
};


#include <iostream>
#include <algorithm>
#include <functional>

// MIGHT REGRET: commenting these next two lines out.
//template <class T>
//inline std::ostream & operator<< (std::ostream &lhs, const T &rhs) { return rhs.Print(lhs);} ;

// MIGHT REGRET: commenting this out
// gcc is broken
//inline std::ostream & operator<< (std::ostream &lhs, char *rhs) { return lhs << ((const char*) rhs);}

// TODO: Find out all the places where this is used.
template <class T>
struct PrintFunc : public std::unary_function<T, void>
{
  std::ostream& os;
  PrintFunc(std::ostream& out) : os(out) {}
  void operator() (T x) { os << x ; }
};

template <typename T>
T MAX(const T &lhs, const T &rhs)
{
  return (lhs>rhs) ? lhs : rhs;
};

template <typename T>
T MIN(const T &lhs, const T &rhs)
{
  return (lhs<rhs) ? lhs : rhs;
};


// TODO / MIGHT REGRET / TEMP: Since ideally we don't want to be using macros, I'm going to rename the MIN and MAX
// macros to something painfully obvious (and painful to use).  Later, we'll want to find all the places
// that use this and see if we can find a better alternative...

#define MIN_MACRO(x,y) ((x)<(y) ? (x) : (y))
#define MAX_MACRO(x,y) ((x)>(y) ? (x) : (y))

// TODO: Do a project-wide search for MIN_MACRO and MAX_MACRO and see if you can find better solutions. This is TEMP.

/**
 * Prints a tab character on an output stream.
 * Allows cout << tab << "text";
 * Slightly easier readability than using "\t" inside the message.
 * @see tab()
 */
inline std::ostream &tab(std::ostream& os)
{
	os << "\t";
	return os;
}


/**
 * @internal
 * @see tab()
 */
struct TabStream
{
	unsigned int num_tabs;

	TabStream(int _num_tabs) : num_tabs(_num_tabs) {}

	friend std::ostream& operator<<(std::ostream& os, const TabStream& tab_stream)
	{
		for(unsigned int i = 0; i < tab_stream.num_tabs; ++i)
			os << "\t";
		return os;
	}
};


/**
 * Prints multiple tabs on an output stream.  Example:
 * 		cout << tab(3) << "This text will be tabbed over thrice.";
 */
inline TabStream tab(const unsigned int num_tabs)
{
	return TabStream(num_tabs);
}

#endif
