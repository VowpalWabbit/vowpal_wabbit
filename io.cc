/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "io.h"

unsigned int buf_read(io_buf &i, char* &pointer, int n)
{//return a pointer to the next n bytes.  n must be smaller than 2^24.
  if (i.space.end + n <= i.space.end_array)
    {
      pointer = i.space.end;
      i.space.end += n;
      return n;
    }
  else if (n > (i.space.end_array - i.space.begin)) // the file is ending or ended.
    {
      pointer = i.space.end;
      int ret = i.space.end_array - i.space.end;
      i.space.end = i.space.end_array;
      return ret;
    }
  else // Change locations and refill buffer.
    {
      int left = i.space.end_array - i.space.end;
      memmove(i.space.begin, i.space.end, left);
      i.fill(left);
      i.space.end = i.space.begin;
      return buf_read(i,pointer,n);
    }
}

void buf_write(io_buf &o, char* &pointer, int n)
{//return a pointer to the next n bytes to write into.
  if (o.space.end + n <= o.space.end_array)
    {
      pointer = o.space.end;
      o.space.end += n;
    }
  else // Time to dump the file
    {
      o.flush();
      buf_write (o, pointer,n);
    }
}
