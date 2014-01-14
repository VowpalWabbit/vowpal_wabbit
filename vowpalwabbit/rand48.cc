//A quick implementation similar to drand48 for cross-platform compatibility
#include <stdint.h>
#include <iostream>
using namespace std;

//
// NB: the 'ULL' suffix is not part of the constant it is there to
// prevent truncation of constant to (32-bit long) when compiling
// in a 32-bit env: warning: integer constant is too large for "long" type
//
uint64_t a = 0xeece66d5deece66dULL;
uint64_t c = 2147483647;

int bias = 127 << 23;

float merand48(uint64_t& initial)
{
  initial = a * initial + c;
  int32_t temp = ((initial >> 25) & 0x7FFFFF) | bias;
  return *(float *)&temp - 1;
}

uint64_t v = c;

void msrand48(uint64_t initial)
{
	v = initial;
}

float frand48()
{
	return merand48(v);
}

/*
//int mantissa = 128 << 15;

int main(int argc, char *argv[])
{
  for (size_t i = 0 ; i < 100000; i++) {
    msrand48(i);
    cout << frand48() << endl;
  }
}

*/
