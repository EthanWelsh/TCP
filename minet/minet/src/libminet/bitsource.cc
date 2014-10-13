#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "bitsource.h"

using namespace std;

template <typename T>
static T MIN(T lhs, T rhs) {
    return (lhs < rhs) ? lhs : rhs;
}


static unsigned char bitstore[NUMUNIQUEBITS / 8];
static int nextbit = 0;


void InitBits() {
    srand(time(0));

    for (int i = 0; i < NUMUNIQUEBITS / 8; i++) {
	bitstore[i] = rand();
    }

    nextbit = 0;
}


// 0 1 2 3 4 5 6 7 8 9 10 11 12 13...
int GetBit(unsigned char byte, int num) {
    return (byte >> (7 - num)) & 0x1;
}


int GetBit(unsigned char * bytearray, int num) {
    int byte = num / 8;
    int bit = num % 8;

    return GetBit(bytearray[byte], bit);
}

void SetBit(unsigned char &byte, int num, int val) {
    unsigned char mask = ~(0x1 << (7 - num));
    val &= 0x1;

    byte &= mask;
    byte |= val << (7 - num);
}

void SetBit(unsigned char *bytearray, int num, int val)
{
  int byte = num/8;
  int bit = num%8;
  SetBit(bytearray[byte],bit,val);
}


void ZeroBits(unsigned char *bitsout, int num, int offsetout)
{
  int i;
  for (i=0;i<num;i++) {
    SetBit(bitsout,i+offsetout,0);
  }
}
  

void CopyBits(unsigned char *bitsin, int num, int offsetin, int offsetout, unsigned char *bitsout)
{
  int i;
  for (i=0;i<num;i++) {
    SetBit(bitsout,i+offsetout,GetBit(bitsin,i+offsetin));
  }
}

void PrintBits(ostream &os,unsigned char *bits, int num, int offsetin)
{
  int i;

  for (i=0;i<num;i++) {
    os << GetBit(bits,i+offsetin);
  }
}


void GetNextBits(unsigned char *bits, int n, int offsetout)
{
  int first, remainder;

  first=MIN(NUMUNIQUEBITS-nextbit,n);
  remainder=n-first;
  CopyBits(bitstore,first,nextbit,offsetout,bits);
  if (remainder>0) {
    CopyBits(bitstore,remainder,0,offsetout+first,bits);
    nextbit=remainder;
  } else {
    nextbit+=first;
  }
#if 0
  cerr << " Returning bits: ";
  for (int i=0;i<(n/8 + (n%8 !=0));i++) {
    cerr << hex << (unsigned) bits[i] << dec;
  }
  cerr << endl;
#endif
    

}

