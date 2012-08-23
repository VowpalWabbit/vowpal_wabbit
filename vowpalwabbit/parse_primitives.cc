/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef WIN32
#include <strings.h>
#endif
#include "parse_primitives.h"
#include <iostream>

using namespace std;

void tokenize(char delim, substring s, v_array<substring>& ret)
{
  ret.erase();
  char *last = s.begin;
  for (; s.begin != s.end; s.begin++) {
    if (*s.begin == delim) {
      if (s.begin != last)
	{
	  substring temp = {last, s.begin};
	  push(ret, temp);
	}
      last = s.begin+1;
    }
  }
  if (s.begin != last)
    {
      substring final = {last, s.begin};
      push(ret, final);
    }
}

