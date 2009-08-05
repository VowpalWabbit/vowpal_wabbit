/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "source.h"
#include <fcntl.h>

bool inconsistent_cache(size_t numbits, io_buf &cache)
{
  size_t total = sizeof(numbits);
  char *p;
  if (buf_read(cache, p, total) < total) 
    return true;

  size_t cache_numbits = *(size_t *)p;

  if (cache_numbits != numbits)
    return true;

  return false;
}

#ifndef O_LARGEFILE //for OSX
#define O_LARGEFILE 0
#endif

void reset_source(size_t numbits, example_source &source)
{
  if (source.write_cache)
    {
      source.cache.flush();
      source.write_cache = false;
      close(source.cache.file);
      rename(source.cache.currentname.c_str(), source.cache.finalname.c_str());
      source.cache.file = open(source.cache.finalname.c_str(), O_RDONLY|O_LARGEFILE);
    }
  if (source.cache.file != -1)
    {
      lseek(source.cache.file, 0, SEEK_SET);
      source.cache.endloaded = source.cache.space.begin;
      source.cache.space.end = source.cache.space.begin;
      if (inconsistent_cache(numbits, source.cache)) {
	cout << "argh, a bug in caching of some sort!  Exitting\n" ;
	exit(1);
      }
    }
}

void finalize_source(example_source &source)
{
  if (source.input.file != -1) {
    close(source.input.file);
    free(source.input.space.begin);
  }
  if (source.cache.file != -1) {
    close(source.cache.file);  
    free(source.cache.space.begin);
  }
}

void setup_source(example_source& source, size_t numbits)
{
  source.global->mask = (1 << numbits) - 1;
  reserve(source.cache.space,0);
  source.cache.file=-1;
  source.write_cache = false;  
}

// sets the example file to read from standard in.
void stdin_source(example_source& source, size_t numbits)
{
  stdin_source(source, numbits, true);
}

void stdin_source(example_source& source, size_t numbits, bool quiet)
{
  setup_source(source,numbits);
  source.input.file = fileno(stdin);
  if (!quiet)
    cout << "reading from stdin" << endl;
}

// sets the example_source to read from a data file
void file_source(example_source& source, size_t numbits, string data_file_name)
{
  file_source(source, numbits, data_file_name, true);
}
void file_source(example_source& source, size_t numbits, string data_file_name, bool quiet)
{
  setup_source(source, numbits);
  if (!quiet)
    cout << "Reading from " << data_file_name << endl;
  FILE* t = fopen(data_file_name.c_str(), "r");
  if (t == NULL)
    {
      cerr << "can't open " << data_file_name << ", bailing!" << endl;
      exit(0);
    }
  source.input.file = fileno(t);
}

// sets the example_source to read from a cache file
void cache_source(example_source& source, size_t numbits, string cache_file_name)
{
  cache_source(source, numbits, cache_file_name, true);
}
void cache_source(example_source& source, size_t numbits, string cache_file_name,bool quiet)
{
  source.global->mask = (1 << numbits) - 1;
  source.cache.file = 
    open(cache_file_name.c_str(), O_RDONLY|O_LARGEFILE);  
  if (source.cache.file == -1)
    {
      cerr << "Cache file " << cache_file_name << " does not exist." << endl;
    }
  else {
    if (inconsistent_cache(numbits, source.cache)) {
      close(source.cache.file);
      source.cache.space.erase();
      cerr << "Cache file is invalid." << endl;
    }
    else {
      if (!quiet)
	cout << "using cache_file = " << cache_file_name << endl;
      source.write_cache = false;
    }
  }
}
