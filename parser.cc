/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <netdb.h>
#include <boost/program_options.hpp>
#include <netinet/tcp.h>
#include <errno.h>
#include <stdio.h>
namespace po = boost::program_options;

#include "parser.h"
#include "parse_example.h"
#include "cache.h"
#include "gd.h"
#include "multisource.h"
#include "comp_io.h"

parser* new_parser(const label_parser* lp)
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
  ret->lp = lp;
  ret->input = new io_buf;
  ret->output = new io_buf;
  return ret;
}

void set_compressed(parser* par){
  finalize_source(par);
  par->input = new comp_io_buf;
  par->output = new comp_io_buf;
}

size_t cache_numbits(io_buf* buf, int filepointer)
{
  size_t v_length;
  buf->read_file(filepointer, (char*)&v_length, sizeof(v_length));
  char t[v_length];
  buf->read_file(filepointer,t,v_length);
  if (strcmp(t,version.c_str()) != 0)
    {
      cout << "cache has possibly incompatible version, rebuilding" << endl;
      return 0;
    }
  
  int total = sizeof(size_t);
  char* p[total];
  if (buf->read_file(filepointer, p, total) < total) 
    return true;

  size_t cache_numbits = *(size_t *)p;
  return cache_numbits;
}

void close_files(v_array<int>& files)
{
  while(files.index() > 0)
    close(files.pop());
}

void reset_source(size_t numbits, parser* p)
{
  io_buf* input = p->input;
  input->current = 0;
  if (p->write_cache)
    {
      p->output->flush();
      p->write_cache = false;
      p->output->close_file();
      rename(p->output->currentname.begin, p->output->finalname.begin);
      input->close_files();
      input->open_file(p->output->finalname.begin,io_buf::READ); //pushing is merged into open_file
      p->reader = read_cached_features;
    }
  if ( p->resettable == true ){
    for (size_t i = 0; i < input->files.index();i++)
      {
        input->reset_file(input->files[i]);
        if (cache_numbits(input, input->files[i]) < numbits) {
          cerr << "argh, a bug in caching of some sort!  Exiting\n" ;
          exit(1);
        }
      }
  }
}

void finalize_source(parser* p)
{
  p->input->close_files();
  delete p->input;
  p->output->close_files();
  delete p->output;
}

void make_write_cache(size_t numbits, parser* par, string &newname, 
		      bool quiet)
{
  io_buf* output = par->output;
  if (output->files.index() != 0){
    cerr << "Warning: you tried to make two write caches.  Only the first one will be made." << endl;
    return;
  }

  string temp = newname+string(".writing");
  push_many(output->currentname,temp.c_str(),temp.length()+1);
  
  int f = output->open_file(temp.c_str(), io_buf::WRITE);
  if (f == -1) {
    cerr << "can't create cache file !" << endl;
    return;
  }

  size_t v_length = version.length()+1;

  output->write_file(f, &v_length, sizeof(size_t));
  output->write_file(f,version.c_str(),v_length);
  
  output->write_file(f, &numbits, sizeof(size_t));
  
  push_many(output->finalname,newname.c_str(),newname.length()+1);
  par->write_cache = true;
  if (!quiet)
    cerr << "creating cache_file = " << newname << endl;
}

void parse_cache(po::variables_map &vm, string source,
		 parser* par, bool quiet)
{
  vector<string> caches;
  if (vm.count("cache_file"))
    caches = vm["cache_file"].as< vector<string> >();
  if (vm.count("cache"))
    caches.push_back(source+string(".cache"));

  par->write_cache = false;

  for (size_t i = 0; i < caches.size(); i++)
    {
      int f = par->input->open_file(caches[i].c_str(),io_buf::READ);
      if (f == -1)
          make_write_cache(global.num_bits, par, caches[i], quiet);
      else {
	size_t c = cache_numbits(par->input, f);
	if (global.default_bits)
	  global.num_bits = c;
	if (c < global.num_bits) {
          par->input->close_file();          
	  make_write_cache(global.num_bits, par, caches[i], quiet);
	}
	else {
	  if (!quiet)
	    cerr << "using cache_file = " << caches[i].c_str() << endl;
	  if (c == global.num_bits)
	    par->reader = read_cached_features;
	  else
	    par->reader = read_and_order_cached_features;
	  par->resettable = true;
	}
      }
    }
  
  global.mask = (1 << global.num_bits) - 1;
  if (caches.size() == 0)
    {
      if (!quiet)
	cerr << "using no cache" << endl;
      reserve(par->output->space,0);
    }
}

bool member(v_array<size_t> ids, size_t id)
{
  for (size_t i = 0; i < ids.index(); i++)
    if (ids[i] == id)
      return true;
  return false;
}

void parse_source_args(po::variables_map& vm, parser* par, bool quiet, size_t passes)
{
  par->input->current = 0;
  parse_cache(vm, vm["data"].as<string>(), par, quiet);

  if (vm.count("daemon") || vm.count("multisource"))
    {
      int daemon = socket(PF_INET, SOCK_STREAM, 0);
      if (daemon < 0) {
	cerr << "can't open socket!" << endl;
	exit(1);
      }

      int on = 1;
      if (setsockopt(daemon, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
	perror("setsockopt SO_REUSEADDR");

      sockaddr_in address;
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_ANY);
      short unsigned int port = 39524;
      if (vm.count("port"))
	port = vm["port"].as<size_t>();
      address.sin_port = htons(port);
      
      if (bind(daemon,(sockaddr*)&address, sizeof(address)) < 0)
	{
	  cerr << "failure to bind!" << endl;
	  exit(1);
	}
      int source_count = 1;
      
      if (vm.count("multisource"))
	source_count = vm["multisource"].as<size_t>();

      listen(daemon, source_count);

      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      par->max_fd = 0;
      size_t min_id = INT_MAX;
      for (int i = 0; i < source_count; i++)
	{
	  if (!global.quiet)
	    cerr << "calling accept" << endl;
	  int f = accept(daemon,(sockaddr*)&client_address,&size);
	  if (f < 0)
	    {
	      cerr << "bad client socket!" << endl;
	      exit (1);
	    }

	  size_t id;
	  really_read(f, &id, sizeof(id));
	  if (!global.quiet)
	    cerr << "id read = " << id << endl;
	  min_id = min (min_id, (size_t)id);
	  if (id == 0)
	    {
	      par->label_sock = f;
	      global.final_prediction_sink = f;
	      global.print = binary_print_result;
	    }
	  if (member(par->ids, id))
	    {
	      cout << "error, two inputs with same id! Exiting.  Use --unique_id <n> next time." << endl;
	      exit(1);
	    }
	  push(par->ids,id);
	  push(par->input->files,f);
	  par->max_fd = max(f, par->max_fd);
	  if (!global.quiet)
	    cerr << "reading data from port " << port << endl;
	}
      global.unique_id = min_id;
      par->max_fd++;
      if(vm.count("multisource"))
	{
	  par->reader = receive_features;
	  calloc_reserve(par->pes,ring_size);
	  par->pes.end = par->pes.begin+ring_size;
	  calloc_reserve(par->counts,ring_size);
	  par->counts.end = par->counts.begin+ring_size;
	  par->finished_count = 0;
	}
      else 
	par->reader = read_cached_features;

      par->resettable = par->write_cache;
    }
  
  if (vm.count("data"))
    if (par->input->files.index() > 0)
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
            int f = par->input->open_file(temp.c_str(), io_buf::READ);
	    if (f == -1)
	      {
		cerr << "can't open " << temp << ", bailing!" << endl;
		exit(0);
	      }
	    par->reader = read_features;
	    par->resettable = par->write_cache;
	  }
      }
  if (par->input->files.index() == 0)// Default to stdin
    {
      if (!quiet)
	cerr << "Reading from stdin" << endl;
      if (vm.count("compressed")){
        cerr << "Compressed source can't be read from stdin." << endl << "Directly use the compressed source with -d option";
        exit(0);
      }
      push(par->input->files,fileno(stdin));
      par->reader = read_features;
      par->resettable = par->write_cache;
    }
  if (passes > 1 && !par->resettable)
    cerr << global.program_name << ": Warning only one pass will occur: try using --cache_file" << endl;  
  par->input->count = par->input->files.index();
  if (!quiet)
    cerr << "num sources = " << par->input->files.index() << endl;
}

example* examples;//A Ring of examples.
pthread_mutex_t examples_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t example_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t example_unused = PTHREAD_COND_INITIALIZER;
size_t parsed_index; // The index of the parsed example.
size_t* used_index; // The index of the example currently used by thread i.
bool done;

void print_update(example *ec)
{
  if (global.weighted_examples > global.dump_interval && !global.quiet)
    {
      label_data* ld = (label_data*) ec->ld;
      fprintf(stderr, "%-10.6f %-10.6f %8lld %8.1f   %8.4f %8.4f %8lu\n",
	      global.sum_loss/global.weighted_examples,
	      global.sum_loss_since_last_dump / (global.weighted_examples - global.old_weighted_examples),
	      global.example_number,
	      global.weighted_examples,
	      ld->label,
	      ec->final_prediction,
	      (long unsigned int)ec->num_features);
      
      global.sum_loss_since_last_dump = 0.0;
      global.old_weighted_examples = global.weighted_examples;
      global.dump_interval *= 2;
    }
}

void output_and_account_example(example* ec)
{
  global.example_number++;
  label_data* ld = (label_data*)ec->ld;
  global.weighted_examples += ld->weight;
  global.weighted_labels += ld->label * ld->weight;
  global.total_features += ec->num_features;
  global.sum_loss += ec->loss;
  global.sum_loss_since_last_dump += ec->loss;
  
  global.print(global.raw_prediction, ec->partial_prediction, ec->tag);
  
  global.print(global.final_prediction_sink, ec->final_prediction, ec->tag);
  
  print_update(ec);

}

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

bool parse_atomic_example(parser* p, example *ae)
{
  if (global.audit)
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
  ae->tag.erase();
  if (p->reader(p,ae) <= 0)
    return false;
  if (p->write_cache) 
    {
      p->lp->cache_label(ae->ld,*(p->output));
      cache_features(*(p->output), ae);
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

void setup_example(example* ae)
{
  size_t num_threads = global.num_threads();

  ae->partial_prediction = 0.;
  ae->num_features = 1;
  ae->threads_to_finish = num_threads;	
  ae->done = false;
  ae->example_counter = ++example_count;

  //Should loop through the features to determine the boundaries
  size_t length = global.mask + 1;
  size_t expert_size = length >> global.thread_bits; //#features/expert
  for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
    {
      ae->subsets[*i].erase();
      feature* f = ae->atomics[*i].begin;
      push(ae->subsets[*i],f);
      size_t current = expert_size;
      while (current <= length)
	{
	  feature* ret = f;
	  if (ae->atomics[*i].end > f)
	    ret = search(f, current, ae->atomics[*i].end);
	  push(ae->subsets[*i],ret);
	  f = ret;
	  current += expert_size;
	}
      ae->num_features += ae->atomics[*i].end - ae->atomics[*i].begin;
    }
  
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
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
	setup_example(ae);

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

void finish_example(example* ec)
{
  pthread_mutex_lock(&examples_lock);
  if (-- ec->threads_to_finish == 0)
    {
      output_and_account_example(ec);
      ec->in_use = false;
      pthread_cond_signal(&example_unused);
      if (done)
	pthread_cond_broadcast(&example_available);
    }

  pthread_mutex_unlock(&examples_lock);
}

example* get_example(size_t thread_num)
{
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
      if (examples[i].tag.end_array != examples[i].tag.begin)
	{
	  free(examples[i].tag.begin);
	  examples[i].tag.end_array = examples[i].tag.begin;
	}

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

  if (pf->pes.begin != NULL)
    {
      for (size_t i = 0; i < ring_size; i++)
	free(pf->pes[i].features.begin);
      free(pf->pes.begin);
    }
  if (pf->ids.begin != NULL)
    free(pf->ids.begin);
  if (pf->counts.begin != NULL)
    free(pf->counts.begin);
}

