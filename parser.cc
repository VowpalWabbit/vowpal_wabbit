/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <fcntl.h>
#include <netdb.h>
#include <boost/program_options.hpp>
#include <netinet/tcp.h>
#include <errno.h>
namespace po = boost::program_options;

#include "parser.h"
#include "parse_example.h"
#include "cache.h"
#include "gd.h"
#include "multisource.h"

parser* new_parser(const label_parser* lp)
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
  ret->lp = lp;
  ret->input = io_buf();
  ret->output = io_buf();
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

void close_files(v_array<int>& files)
{
  while(files.index() > 0)
    close(files.pop());
}

void reset_source(size_t numbits, parser* p)
{
  p->input.current = 0;
  if (p->write_cache)
    {
      p->output.flush();
      p->write_cache = false;
      close(p->output.files.pop());
      rename(p->output.currentname.begin, p->output.finalname.begin);
      close_files(p->input.files);
      push(p->input.files,open(p->output.finalname.begin, O_RDONLY|O_LARGEFILE));
      p->reader = read_cached_features;
    }
  if ( p->resettable == true )
    for (size_t i = 0; i < p->input.files.index();i++)
      {
	lseek(p->input.files[i], 0, SEEK_SET);
	p->input.endloaded = p->input.space.begin;
	p->input.space.end = p->input.space.begin;
	if (inconsistent_cache(numbits, p->input.files[i])) {
	  cerr << "argh, a bug in caching of some sort!  Exiting\n" ;
	  exit(1);
	}
      }
}

void finalize_source(parser* p)
{
  close_files(p->input.files);
  free(p->input.files.begin);
  free(p->input.space.begin);

  close_files(p->output.files);
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
		 parser* par, bool quiet)
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
	{
	  if (par->output.files.index() == 0)
	    make_write_cache(numbits, par, caches[i], quiet);
	  else 
	    cerr << "Warning: you tried to make two write caches.  Only the first one will be made." << endl;
	}
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
	  par->resettable = true;
	}
      }
    }
  
  if (caches.size() == 0)
    {
      if (!quiet)
	cerr << "using no cache" << endl;
      reserve(par->output.space,0);
    }
}

void parse_source_args(po::variables_map& vm, parser* par, bool quiet, size_t passes)
{
  par->input.current = 0;
  parse_cache(vm, par->global->num_bits, vm["data"].as<string>(), par, quiet);

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
      int source_count = 1;
      
      if (vm.count("multisource"))
	source_count = vm["multisource"].as<int>();

      listen(daemon, source_count);

      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      int max_fd = 0;
      for (int i = 0; i < source_count; i++)
	{
	  int f = accept(daemon,(sockaddr*)&client_address,&size);
	  if (f < 0)
	    {
	      cerr << "bad client socket!" << endl;
	      exit (1);
	    }
	  int flag = 1;
	  if (setsockopt(f,IPPROTO_TCP,TCP_NODELAY,(char*) &flag, sizeof(int)) == -1)
	    cerr << strerror(errno) << " " << errno << " " << IPPROTO_TCP << endl;

	  size_t id;
	  read(f, &id, sizeof(id));
	  if (id == 0)
	    {
	      par->label_sock = f;
	      par->global->network_prediction = f;
	    }

	  push(par->ids,id);
	  push(par->input.files,f);
	  max_fd = max(f, max_fd);
	  cerr << "reading data from port 39523" << endl;
	}
      max_fd++;
      if(source_count > 1)
	par->reader = receive_features;
      else 
	par->reader = read_cached_features;

      par->resettable = par->write_cache;
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
	    par->resettable = par->write_cache;
	  }
      }
  if (par->input.files.index() == 0)// Default to stdin
    {
      if (!quiet)
	cerr << "Reading from stdin" << endl;
      push(par->input.files,fileno(stdin));
      par->reader = read_features;
      par->resettable = par->write_cache;
    }
  if (passes > 1 && !par->resettable)
    cerr << par->global->program_name << ": Warning only one pass will occur: try using --cache_file" << endl;  
  cout << "num sources = " << par->input.files.index() << endl;
}

const size_t ring_size = 20;
example* examples;//A Ring of examples.
pthread_mutex_t examples_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t example_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t example_unused = PTHREAD_COND_INITIALIZER;
size_t parsed_index; // The index of the parsed example.
size_t* used_index; // The index of the example currently used by thread i.
bool done;

example* get_unused_example()
{
  while (true)
    {
      pthread_mutex_lock(&examples_lock);
      if (examples[parsed_index % ring_size].in_use == false)
	{
	  examples[parsed_index % ring_size].in_use = true;
	  pthread_mutex_unlock(&examples_lock);
	  return examples + (parsed_index % ring_size);
	}
      else 
	pthread_cond_wait(&example_unused, &examples_lock);
      pthread_mutex_unlock(&examples_lock);
    }
}

int order_features(const void* first, const void* second)
{
  return ((feature*)first)->weight_index - ((feature*)second)->weight_index;
}

int order_audit_features(const void* first, const void* second)
{
  return ((audit_data*)first)->weight_index - ((audit_data*)second)->weight_index;
}

void unique_features(v_array<feature> &features)
{
  if (features.empty())
    return;
  feature* last = features.begin;
  for (feature* current = features.begin+1; 
       current != features.end; current++)
    if (current->weight_index != last->weight_index) 
      *(++last) = *current;
  features.end = ++last;
}

void unique_audit_features(v_array<audit_data> &features)
{
  if (features.empty())
    return;
  audit_data* last = features.begin;
  for (audit_data* current = features.begin+1; 
       current != features.end; current++)
    if (current->weight_index != last->weight_index) 
      *(++last) = *current;
  features.end = ++last;
}

void unique_sort_features(parser* p, example* ae)
{
  bool audit = p->global->audit;
  for (size_t* b = ae->indices.begin; b != ae->indices.end; b++)
    {
      qsort(ae->atomics[*b].begin, ae->atomics[*b].index(), sizeof(feature), 
	    order_features);
      unique_features(ae->atomics[*b]);
      
      if (audit)
	{
	  qsort(ae->audit_features[*b].begin, ae->audit_features[*b].index(), sizeof(audit_data), 
		order_audit_features);
	  unique_audit_features(ae->audit_features[*b]);
	}
    }
}

bool parse_atomic_example(parser* p, example *ae)
{
  if (p->global->audit)
    for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
      {
	for (audit_data* temp 
	       = ae->audit_features[*i].begin; 
	     temp != ae->audit_features[*i].end; temp++)
	  {
	    if (temp->alloced)
	      {
		free(temp->space);
		free(temp->feature);
		temp->alloced=false;
	      }
	  }
	ae->audit_features[*i].erase();
      }

  for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
    ae->atomics[*i].erase();

  ae->indices.erase();
  if (p->reader(p,ae) <= 0)
      return false;
  unique_sort_features(p,ae);
  if (p->write_cache) 
    {
      p->lp->cache_label(ae->ld,p->output);
      cache_features(p->output, ae);
    }
  return true;
}

feature* search(feature* begin, size_t value, feature* end)
{//return the smallest position >= value, never referencing end.
  size_t diff = end-begin;
  if (diff <= 1)
    if (begin->weight_index >= value)
      return begin;
    else
      return end;
  else
    {
      feature* middle = begin + (diff >> 1);
      if (middle->weight_index >= value)
	return search(begin, value, middle);
      else
	return search(middle, value, end);
    }
}

size_t example_count = 0;

void setup_example(example* ae, global_data* global)
{
  size_t num_threads = global->num_threads();

  ae->partial_prediction = 0.;
  ae->num_features = 1;
  ae->threads_to_finish = num_threads;	
  ae->done = false;
  ae->example_counter = ++example_count;

  //Should loop through the features to determine the boundaries
  size_t length = global->mask + 1;
  size_t expert_size = length >> global->thread_bits; //#features/expert
  for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
    {
      ae->subsets[*i].erase();
      feature* f = ae->atomics[*i].begin;
      push(ae->subsets[*i],f);
      size_t current = expert_size;
      while (current <= length)
	{
	  feature* ret = search(f, current, ae->atomics[*i].end);
	  push(ae->subsets[*i],ret);
	  f = ret;
	  current += expert_size;
	}
      
      ae->num_features += ae->atomics[*i].end - ae->atomics[*i].begin;
    }
  
  for (vector<string>::iterator i = global->pairs.begin(); i != global->pairs.end();i++) 
    {
    ae->num_features 
      += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin)
      *(ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin);
    }
}

void *main_parse_loop(void *in)
{
  parser* p = (parser*) in;
  
  while(!done)
    {
      example* ae=get_unused_example();

      int output = parse_atomic_example(p,ae);
      if (output) {	
	setup_example(ae,p->global);

	pthread_mutex_lock(&examples_lock);
	parsed_index++;
	pthread_cond_broadcast(&example_available);
	pthread_mutex_unlock(&examples_lock);

      }
      else
	{
	  pthread_mutex_lock(&examples_lock);
	  done = true;
	  ae->in_use = false;
	  pthread_cond_broadcast(&example_available);
	  pthread_mutex_unlock(&examples_lock);
	}
    }  

  free(p->channels.begin);
  p->channels.begin = p->channels.end = p->channels.end_array = NULL;
  free(p->words.begin);
  p->words.begin = p->words.end = p->words.end_array = NULL;
  free(p->name.begin);
  p->name.begin = p->name.end = p->name.end_array = NULL;

  return NULL;
}

bool examples_to_finish()
{
  for(size_t i = 0; i < ring_size; i++)
    if (examples[i].in_use)
      return true;
  return false;
}

inline void finish_example(example* ec)
{
  pthread_mutex_lock(&examples_lock);
  if (-- ec->threads_to_finish == 0)
    {
      ec->in_use = false;
      pthread_cond_signal(&example_unused);
      if (done)
	pthread_cond_broadcast(&example_available);
    }
  pthread_mutex_unlock(&examples_lock);
}

example* get_example(example* ec, size_t thread_num)
{
  if (ec != NULL)
    finish_example(ec);
  
  while (true) // busy wait until an example is acquired.
    {
      pthread_mutex_lock(&examples_lock);
      
      if (parsed_index != used_index[thread_num]) {
	size_t ring_index = used_index[thread_num]++ % ring_size;
	pthread_mutex_unlock(&examples_lock);
	return examples + ring_index;
      }
      else {
	if (!done || examples_to_finish()) {
	  pthread_cond_wait(&example_available, &examples_lock);
	  pthread_mutex_unlock(&examples_lock);
	}
	else 
	  { 
	    pthread_mutex_unlock(&examples_lock);
	    return NULL;
	  }
      }
    }
}

pthread_t parse_thread;

void start_parser(size_t num_threads, parser* pf)
{
  used_index = (size_t*) calloc(num_threads, sizeof(size_t));
  parsed_index = 0;
  done = false;

  examples = (example*)calloc(ring_size, sizeof(example));
  
  for (size_t i = 0; i < ring_size; i++)
    {
      examples[i].lock = examples_lock;
      examples[i].ld = calloc(1,pf->lp->label_size);
    }
  pthread_create(&parse_thread, NULL, main_parse_loop, pf);
}

void end_parser(parser* pf)
{
  pthread_join(parse_thread, NULL);
  free(used_index);
  
  for (size_t i = 0; i < ring_size; i++) 
    {
      pf->lp->delete_label(examples[i].ld);
      free(examples[i].ld);
      for (size_t j = 0; j < 256; j++)
	{
	  if (examples[i].atomics[j].begin != examples[i].atomics[j].end_array)
	    free(examples[i].atomics[j].begin);
	  if (examples[i].audit_features[j].begin != examples[i].audit_features[j].end)
	    {
	      for (audit_data* temp = examples[i].audit_features[j].begin; 
		   temp != examples[i].audit_features[j].end; temp++)
		if (temp->alloced) {
		  free(temp->space);
		  free(temp->feature);
		  temp->alloced = false;
		}
	      free(examples[i].audit_features[j].begin);
	    }
	  if (examples[i].subsets[j].begin != examples[i].subsets[j].end_array)
	    free(examples[i].subsets[j].begin);
	}
      free(examples[i].indices.begin);
    }
  free(examples);
}

