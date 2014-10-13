#include <iostream>
#include "bitsource.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
  unsigned char bit;
  unsigned char bitarray[20];
  int i;


  InitBits();

  ZeroBits(&bit,sizeof(bit)*8,0);

  for (i=0;i<13;i++) {
    GetNextBits(&bit,1,0);
    cout << "got bit ";
    PrintBits(cout,&bit,1,0);
    cout <<endl;
  }

  for (i=0;i<8;i++) {
    GetNextBits(bitarray,32,0);
    cout << "got bits ";
    PrintBits(cout,bitarray,32,0);
    cout << endl;
  }
}


