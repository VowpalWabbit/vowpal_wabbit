/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
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
	  ret.push_back(temp);
	}
      last = s.begin+1;
    }
  }
  if (s.begin != last)
    {
      substring final = {last, s.begin};
      ret.push_back(final);
    }
}

