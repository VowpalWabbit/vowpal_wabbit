//A quick implementation similar to drand48 for cross-platform compatibility
#include <stdint.h>

uint64_t a = 0x5DEECE66D;
uint64_t c = 2147483647;

uint64_t v = c;

uint64_t mask = (((uint64_t)1) << 48) - 1;

void msrand48(uint64_t initial)
{
  v = initial;
}

int bias = 127 << 23;

float frand48()
{
  v = a * v + c;
  int32_t temp = (v >> 41) | bias;
  return *(float *)&temp - 1;
}

/*#include <iostream>
using namespace std;

int mantissa = 128 << 15;

int main(int argc, char *argv[])
{
  for (size_t i = 0 ; i < 100000; i++)
    cout << frand48() << endl;
}
*/
