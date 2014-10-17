#pragma once
//A quick implementation similar to drand48 for cross-platform compatibility
#include <stdint.h>

namespace PRG {
const uint64_t a = 0xeece66d5deece66dULL;
const uint64_t c = 2147483647;

const int bias = 127 << 23;

 struct prg {
 private:
   uint64_t v;
 public:
   prg() {v = c;}
   prg(uint64_t initial) 
   { 
	   v = initial; 
	   merand48(v);
   }

   float merand48(uint64_t& initial)
   {
     initial = a * initial + c;
     int32_t temp = ((initial >> 25) & 0x7FFFFF) | bias;
     return *(float *)&temp - 1;
   }
   
   float Uniform_Unit_Interval()
   {
     return merand48(v);
   }

   uint32_t Uniform_Int(uint32_t low, uint32_t high)
   {
	   merand48(v);
	   uint32_t ret = low + ((v >> 25) % (high - low + 1));
	   return ret;
   }
 };
}
