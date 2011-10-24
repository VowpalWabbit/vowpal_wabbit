/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <string.h>

#include "io.h"

unsigned int buf_read(io_buf &i, char* &pointer, int n)
{//return a pointer to the next n bytes.  n must be smaller than the maximum size.
  if (i.space.end + n <= i.endloaded)
    {
      pointer = i.space.end;
      i.space.end += n;
      return n;
    }
  else // out of bytes, so refill.
    {
      if (i.space.end != i.space.begin) //There exists room to shift.
	{ // Out of buffer so swap to beginning.
	  int left = i.endloaded - i.space.end;
	  memmove(i.space.begin, i.space.end, left);
	  i.space.end = i.space.begin;
	  i.endloaded = i.space.begin+left;
	}
      if (i.fill(i.files[i.current]) > 0)
	return buf_read(i,pointer,n);// more bytes are read.
      else if (++i.current < i.files.index()) 
	return buf_read(i,pointer,n);// No more bytes, so go to next file and try again.
      else
	{//no more bytes to read, return all that we have left.
	  pointer = i.space.end;
	  i.space.end = i.endloaded;
	  return i.endloaded - pointer;
	}
    }
}

bool isbinary(io_buf &i) {
  if (i.endloaded == i.space.end)
    if (i.fill(i.files[i.current]) <= 0)
      return false;

  bool ret = (*i.space.end == 0);
  if (ret)
    i.space.end++;

  return ret;
}

size_t readto(io_buf &i, char* &pointer, char terminal)
{//Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = i.space.end;
  while (pointer != i.endloaded && *pointer != terminal)
    pointer++;
  if (pointer != i.endloaded)
    {
      size_t n = pointer - i.space.end;
      i.space.end = pointer+1;
      pointer -= n;
      return n;
    }
  else
    {
      if (i.endloaded == i.space.end_array)
	{
	  size_t left = i.endloaded - i.space.end;
	  memmove(i.space.begin, i.space.end, left);
	  i.space.end = i.space.begin;
	  i.endloaded = i.space.begin+left;
	  pointer = i.endloaded;
	}
      if (i.fill(i.files[i.current]) > 0)// more bytes are read.
	return readto(i,pointer,terminal);
      else if (++i.current < i.files.index())  //no more bytes, so go to next file.
	return readto(i,pointer,terminal);
      else //no more bytes to read, return nothing.
	return 0;
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
      if (o.space.end != o.space.begin)
	o.flush();
      else // Array is short, so increase size.
	{
	  reserve(o.space, 2*(o.space.end_array - o.space.begin));
	  o.endloaded = o.space.begin;
	}
      buf_write (o, pointer,n);
    }
}
