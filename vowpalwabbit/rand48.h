#pragma once
void msrand48(uint64_t initial);

#ifdef __APPLE__
float merand48(unsigned long& initial);
#endif

float merand48(uint64_t& initial);

float frand48();
float frand48_noadvance();

