/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "source.h"
#include "parse_example.h"
#include "cache.h"
#include <fcntl.h>

parser* new_parser(const label_parser* lp)
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
  ret->lp = lp;
  io_buf i,o;
  ret->input = i;
  ret->output = o;
  return ret;
}

bool inconsistent_cache(size_t numbits, io_buf &cache)
{
  size_t total = sizeof(numbits);
  char *p;
  if (buf_read(cache, p, total) < total) 
    return true;

  size_t cache_numbits = *(size_t *)p;

  if (cache_numbits < numbits)
    return true;

  return false;
}

#ifndef O_LARGEFILE //for OSX
#define O_LARGEFILE 0
#endif

void reset_source(size_t numbits, parser* p)
{
  if (p->write_cache)
    {
      p->output.flush();
      p->write_cache = false;
      close(p->output.file);
      rename(p->output.currentname.begin, p->output.finalname.begin);
      p->input.file = open(p->output.finalname.begin, O_RDONLY|O_LARGEFILE);
    }
  if (p->input.file != -1)
    {
      lseek(p->input.file, 0, SEEK_SET);
      p->input.endloaded = p->input.space.begin;
      p->input.space.end = p->input.space.begin;
      if (inconsistent_cache(numbits, p->input)) {
	cout << "argh, a bug in caching of some sort!  Exitting\n" ;
	exit(1);
      }
      p->reader = read_cached_features;
    }
}

void finalize_source(parser* p)
{
  if (p->input.file != -1) {
    close(p->input.file);
    free(p->input.space.begin);
  }
  if (p->output.file != -1) {
    close(p->output.file);  
    free(p->output.space.begin);
  }
}

