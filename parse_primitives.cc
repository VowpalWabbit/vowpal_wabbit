/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "parse_primitives.h"

void tokenize(char delim, substring s, v_array<substring>& ret)
{
  ret.erase();
  char *last = s.start;
  for (; s.start != s.end; s.start++) {
    if (*s.start == delim) {
      if (s.start != last)
	{
	  substring temp = {last,s.start};
	  push(ret, temp);
	}
      last = s.start+1;
    }
  }
  if (s.start != last)
    {
      substring final = {last, s.start};
      push(ret, final);
    }
}

