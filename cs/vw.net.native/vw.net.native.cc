#include "vw.net.native.h"
#include "hash.h"
#include <cstdlib>

API void FreeDupString(char* str)
{
  free(str);
}

API uint64_t VwUniformHash(char* key, size_t len, uint64_t seed)
{
  return uniform_hash(key, len, seed);
}