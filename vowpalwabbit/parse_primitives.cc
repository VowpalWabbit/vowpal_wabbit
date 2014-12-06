/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <iostream>
#ifndef WIN32
#include <strings.h>
#endif

#include "parse_primitives.h"
#include "hash.h"

using namespace std;

void tokenize(char delim, substring s, v_array<substring>& ret, bool allow_empty)
{
  ret.erase();
  char *last = s.begin;
  for (; s.begin != s.end; s.begin++) {
    if (*s.begin == delim) {
      if (allow_empty || (s.begin != last))
	{
	  substring temp = {last, s.begin};
	  ret.push_back(temp);
	}
      last = s.begin+1;
    }
  }
  if (allow_empty || (s.begin != last))
    {
      substring final = {last, s.begin};
      ret.push_back(final);
    }
}

size_t hashstring (substring s, uint32_t h)
{
  //trim leading whitespace but not UTF-8
  for(; s.begin < s.end && *(s.begin) <= 0x20 && (int)*(s.begin)>= 0; s.begin++);
  //trim trailing white space but not UTF-8
  for(; s.end > s.begin && *(s.end-1) <= 0x20 && (int)*(s.end-1) >=0; s.end--);

  size_t ret = 0;
  char *p = s.begin;
  while (p != s.end)
    if (*p >= '0' && *p <= '9')
      ret = 10*ret + *(p++) - '0';
    else
      return uniform_hash((unsigned char *)s.begin, s.end - s.begin, h);

  return ret + h;
}

size_t hashall (substring s, uint32_t h)
{
  return uniform_hash((unsigned char *)s.begin, s.end - s.begin, h);
}

hash_func_t getHasher(const string& s){
  if (s=="strings")
    return hashstring;
  else if(s=="all")
    return hashall;
  else{
    cerr << "Unknown hash function: " << s.c_str() << ". Exiting " << endl;
    throw exception();
  }
}
