/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <fcntl.h>
#include <netdb.h>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include "source.h"
#include "parse_example.h"
#include "cache.h"
#include "gd.h"

parser* new_parser(const label_parser* lp)
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
  ret->lp = lp;
  io_buf i,o;
  ret->input = i;
  ret->output = o;
  return ret;
}

#ifndef O_LARGEFILE //for OSX
#define O_LARGEFILE 0
#endif

bool inconsistent_cache(size_t numbits, int filepointer)
{
  int total = sizeof(numbits);
  char* p[sizeof(numbits)];
  if (read(filepointer, p, total) < total) 
    return true;

  size_t cache_numbits = *(size_t *)p;

  if (cache_numbits < numbits)
    return true;

  return false;
}

void reset_source(size_t numbits, parser* p)
{
  if (p->write_cache)
    {
      p->output.flush();
      p->write_cache = false;
      close(p->output.files[0]);
      rename(p->output.currentname.begin, p->output.finalname.begin);
      push(p->input.files,open(p->output.finalname.begin, O_RDONLY|O_LARGEFILE));
    }
  if (p->input.files.index() > 0)
    {
      lseek(p->input.files[0], 0, SEEK_SET);
      p->input.endloaded = p->input.space.begin;
      p->input.space.end = p->input.space.begin;
      if (inconsistent_cache(numbits, p->input.files.last())) {
	cerr << "argh, a bug in caching of some sort!  Exiting\n" ;
	exit(1);
      }
      p->reader = read_cached_features;
    }
}

void finalize_source(parser* p)
{
  for (size_t i = 0; i < p->input.files.index();i++)
    close(p->input.files[i]);
  free(p->input.files.begin);
  free(p->input.space.begin);

  for (size_t i = 0; i < p->output.files.index();i++)
    close(p->output.files[i]);  
  free(p->output.files.begin);
  free(p->output.space.begin);
}

void make_write_cache(size_t numbits, parser* par, string &newname, 
		      bool quiet)
{
  string temp = newname+string(".writing");
  push_many(par->output.currentname,temp.c_str(),temp.length()+1);
  
  int f = open(temp.c_str(), O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
  if (f == -1) {
    cerr << "can't create cache file !" << endl;
    return;
  }
  push(par->output.files,f);
  char *p;
  buf_write(par->output, p, sizeof(size_t));
  
  *(size_t *)p = numbits;
  
  push_many(par->output.finalname,newname.c_str(),newname.length()+1);
  par->write_cache = true;
  if (!quiet)
    cerr << "creating cache_file = " << newname << endl;
}

void parse_cache(po::variables_map &vm, size_t numbits, string source,
		 parser* par, bool quiet, size_t passes)
{
  par->global->mask = (1 << numbits) - 1;
  vector<string> caches;
  if (vm.count("cache_file"))
    caches = vm["cache_file"].as< vector<string> >();
  if (vm.count("cache"))
    caches.push_back(source+string(".cache"));

  par->write_cache = false;

  for (size_t i = 0; i < caches.size(); i++)
    {
      int f = open(caches[i].c_str(), O_RDONLY|O_LARGEFILE);
      if (f == -1)
	make_write_cache(numbits, par, caches[i], quiet);
      else {
	if (inconsistent_cache(numbits, f)) {
	  close(f);
	  make_write_cache(numbits, par, caches[i], quiet);
	}
	else {
	  if (!quiet)
	    cerr << "using cache_file = " << caches[i].c_str() << endl;
	  push(par->input.files,f);
	  par->reader = read_cached_features;
	}
      }
    }
  
  if (caches.size() == 0)
    {
      if (!quiet)
	cerr << "using no cache" << endl;
      if (passes > 1)
	cerr << par->global->program_name << ": Warning only one pass will occur: try using --cache_file" << endl;
      reserve(par->output.space,0);
    }
}

void parse_source_args(po::variables_map& vm, parser* par, bool quiet, size_t passes)
{
  parse_cache(vm, par->global->num_bits, vm["data"].as<string>(), par, quiet, passes);

  if (vm.count("daemon"))
    {
      int daemon = socket(PF_INET, SOCK_STREAM, 0);
      if (daemon < 0) {
	cerr << "can't open socket!" << endl;
	exit(1);
      }
      sockaddr_in address;
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_ANY);
      address.sin_port = htons(39523);
      
      if (bind(daemon,(sockaddr*)&address, sizeof(address)) < 0)
	{
	  cerr << "failure to bind!" << endl;
	  exit(1);
	}
      listen(daemon,1);
      
      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      int f = accept(daemon,(sockaddr*)&client_address,&size);
      if (f < 0)
	{
	  cerr << "bad client socket!" << endl;
	  exit (1);
	}
      cerr << "reading data from port 39523" << endl;
      push(par->input.files,f);
      par->reader = read_cached_features;
    }
  
  if (vm.count("data"))
    if (par->input.files.index() > 0)
      {
	if (!quiet)
	  cerr << "ignoring text input in favor of cache input" << endl;
      }
    else
      {
	string temp = vm["data"].as< string >();
	if (temp.length() != 0)
	  {
	    if (!quiet)
	      cerr << "Reading from " << temp << endl;
	    int f = open(temp.c_str(), O_RDONLY);
	    if (f == -1)
	      {
		cerr << "can't open " << temp << ", bailing!" << endl;
		exit(0);
	      }
	    push(par->input.files, f);
	    par->reader = read_features;
	  }
      }
  if (par->input.files.index() == 0)// Default to stdin
    {
      if (!quiet)
	cerr << "Reading from stdin" << endl;
      push(par->input.files,fileno(stdin));
      par->reader = read_features;
    }
}
