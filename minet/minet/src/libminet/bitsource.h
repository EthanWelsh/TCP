#ifndef _bitsource
#define _bitsource

const int NUMUNIQUEBITS=8192*8;


// 0 1 2 3 4 5 6 7 8 9 10 11 12 13...
// Bit numbering as above

void InitBits();
int  GetBit(unsigned char byte, int num);
int  GetBit(unsigned char *bytearray, int num);
void SetBit(unsigned char &byte, int num, int val);
void SetBit(unsigned char *bytearray, int num, int val);
void ZeroBits(unsigned char *bitsout, int num, int offsetout);
void CopyBits(unsigned char *bitsin, int num, int offsetin, int offsetout, unsigned char *bitsout);
void GetNextBits(unsigned char *bits, int num, int offsetout);
void PrintBits(std::ostream &os,unsigned char *bits, int num, int offsetin);



struct hexme_t {
  unsigned int x;
  hexme_t(unsigned int x) : x(x) {}
};

inline std::ostream &operator<<(std::ostream &s, const hexme_t &hexme) {
  return s <<std::hex <<hexme.x <<std::dec;
}

inline hexme_t hexme(unsigned int x) {
  return hexme_t(x);
}




#endif
