/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <sys/types.h>

#ifndef _WIN32
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/tcp.h>
#endif

#include <signal.h>

#include <fstream>

#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#include <io.h>
typedef int socklen_t;

int daemon(int a, int b)
{
	exit(0);
	return 0;
}
int getpid()
{
	return (int) ::GetCurrentProcessId();
}
#else
#include <netdb.h>
#endif
#include <boost/program_options.hpp>

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <assert.h>
namespace po = boost::program_options;

#include "parser.h"
#include "global_data.h"
#include "parse_example.h"
#include "cache.h"
#include "gd.h"
#include "comp_io.h"
#include "unique_sort.h"
#include "constant.h"
#include "example.h"
#include "simple_label.h"
#include "vw.h"

using namespace std;

void initialize_mutex(MUTEX * pm)
{
#ifndef _WIN32
  pthread_mutex_init(pm, NULL);
#else
	::InitializeCriticalSection(pm);
#endif
}

void delete_mutex(MUTEX * pm)
{
#ifndef _WIN32
	// no operation necessary here
#else
	::DeleteCriticalSection(pm);
#endif
}

void initialize_condition_variable(CV * pcv)
{
#ifndef _WIN32
  pthread_cond_init(pcv, NULL);
#else
	::InitializeConditionVariable(pcv);
#endif
}

void mutex_lock(MUTEX * pm)
{
#ifndef _WIN32
	pthread_mutex_lock(pm);
#else
	::EnterCriticalSection(pm);
#endif
}

void mutex_unlock(MUTEX * pm)
{
#ifndef _WIN32
	pthread_mutex_unlock(pm);
#else
	::LeaveCriticalSection(pm);
#endif
}

void condition_variable_wait(CV * pcv, MUTEX * pm)
{
#ifndef _WIN32
	pthread_cond_wait(pcv, pm);
#else
	::SleepConditionVariableCS(pcv, pm, INFINITE);
#endif
}

void condition_variable_signal(CV * pcv)
{
#ifndef _WIN32
	pthread_cond_signal(pcv);
#else
	::WakeConditionVariable(pcv);
#endif
}

void condition_variable_signal_all(CV * pcv)
{
#ifndef _WIN32
	pthread_cond_broadcast(pcv);
#else
	::WakeAllConditionVariable(pcv);
#endif
}

//This should not? matter in a library mode.
bool got_sigterm;

void handle_sigterm (int)
{
  got_sigterm = true;
}

bool is_test_only(uint32_t counter, uint32_t period, uint32_t after, bool holdout_off)
{
  if(holdout_off) return false;
  if (after == 0) // hold out by period
    return (counter % period == 0);
  else // hold out by position
    return (counter+1 >= after);
}

parser* new_parser()
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
  ret->input = new io_buf;
  ret->output = new io_buf;
  ret->local_example_number = 0;
  ret->in_pass_counter = 0;
  ret->ring_size = 1 << 8;
  ret->done = false;
  ret->used_index = 0;

  return ret;
}

void set_compressed(parser* par){
  finalize_source(par);
  par->input = new comp_io_buf;
  par->output = new comp_io_buf;
}

uint32_t cache_numbits(io_buf* buf, int filepointer)
{
  v_array<char> t;

  uint32_t v_length;
  buf->read_file(filepointer, (char*)&v_length, sizeof(v_length));
  if(v_length>29){
    cerr << "cache version too long, cache file is probably invalid" << endl;
    throw exception();
  }
  t.erase();
  if (t.size() < v_length)
    t.resize(v_length);
  
  buf->read_file(filepointer,t.begin,v_length);
  version_struct v_tmp(t.begin);
  if ( v_tmp != version )
    {
      cout << "cache has possibly incompatible version, rebuilding" << endl;
      t.delete_v();
      return 0;
    }

  char temp;
  if (buf->read_file(filepointer, &temp, 1) < 1) 
    {
      cout << "failed to read" << endl;
      throw exception();
    }
  if (temp != 'c')
    {
      cout << "data file is not a cache file" << endl;
      throw exception();
    }

  t.delete_v();
  
  const int total = sizeof(uint32_t);
  char* p[total];
  if (buf->read_file(filepointer, p, total) < total) 
    {
      return true;
    }

  uint32_t cache_numbits = *(uint32_t *)p;
  return cache_numbits;
}

bool member(v_array<int> ids, int id)
{
  for (size_t i = 0; i < ids.size(); i++)
    if (ids[i] == id)
      return true;
  return false;
}

void reset_source(vw& all, size_t numbits)
{
  io_buf* input = all.p->input;
  input->current = 0;
  if (all.p->write_cache)
    {
      all.p->output->flush();
      all.p->write_cache = false;
      all.p->output->close_file();
	  remove(all.p->output->finalname.begin);
      rename(all.p->output->currentname.begin, all.p->output->finalname.begin);
      while(input->files.size() > 0)
	{
	  int fd = input->files.pop();
	  if (!member(all.final_prediction_sink, (size_t) fd))
#ifdef _WIN32
	    _close(fd);
#else
	    close(fd);
#endif
	}
      input->open_file(all.p->output->finalname.begin, all.stdin_off, io_buf::READ); //pushing is merged into open_file
      all.p->reader = read_cached_features;
    }
  if ( all.p->resettable == true )
    {
      if (all.daemon)
	{
	  // wait for all predictions to be sent back to client
	  mutex_lock(&all.p->output_lock);
	  while (all.p->local_example_number != all.p->parsed_examples)
	    condition_variable_wait(&all.p->output_done, &all.p->output_lock);
	  mutex_unlock(&all.p->output_lock);
	  
	  // close socket, erase final prediction sink and socket
#ifdef _WIN32
	  _close(all.p->input->files[0]);
#else
	  close(all.p->input->files[0]);
#endif
	  all.final_prediction_sink.erase();
	  all.p->input->files.erase();
	  
	  sockaddr_in client_address;
	  socklen_t size = sizeof(client_address);
	  int f = (int)accept(all.p->bound_sock,(sockaddr*)&client_address,&size);
	  if (f < 0)
	    {
	      cerr << "bad client socket!" << endl;
	      throw exception();
	    }
	  
	  // note: breaking cluster parallel online learning by dropping support for id
	  
	  all.final_prediction_sink.push_back((size_t) f);
	  all.p->input->files.push_back(f);

	  if (isbinary(*(all.p->input))) {
	    all.p->reader = read_cached_features;
	    all.print = binary_print_result;
	  } else {
	    all.p->reader = read_features;
	    all.print = print_result;
	  }
	}
      else {
	for (size_t i = 0; i < input->files.size();i++)
	  {
	    input->reset_file(input->files[i]);
	    if (cache_numbits(input, input->files[i]) < numbits) {
	      cerr << "argh, a bug in caching of some sort!  Exiting\n" ;
	      throw exception();
	    }
	  }
      }
    }
}

void finalize_source(parser* p)
{
#ifdef _WIN32
  int f = _fileno(stdin);
#else
  int f = fileno(stdin);
#endif
  while (!p->input->files.empty() && p->input->files.last() == f)
    p->input->files.pop();
  p->input->close_files();

  delete p->input;
  p->output->close_files();
  delete p->output;
}

void make_write_cache(vw& all, string &newname, bool quiet)
{
  io_buf* output = all.p->output;
  if (output->files.size() != 0){
    cerr << "Warning: you tried to make two write caches.  Only the first one will be made." << endl;
    return;
  }

  string temp = newname+string(".writing");
  push_many(output->currentname,temp.c_str(),temp.length()+1);
  
  int f = output->open_file(temp.c_str(), all.stdin_off, io_buf::WRITE);
  if (f == -1) {
    cerr << "can't create cache file !" << endl;
    return;
  }

  uint32_t v_length = (uint32_t)version.to_string().length()+1;

  output->write_file(f, &v_length, sizeof(v_length));
  output->write_file(f,version.to_string().c_str(),v_length);
  output->write_file(f,"c",1);
  output->write_file(f, &all.num_bits, sizeof(all.num_bits));
  
  push_many(output->finalname,newname.c_str(),newname.length()+1);
  all.p->write_cache = true;
  if (!quiet)
    cerr << "creating cache_file = " << newname << endl;
}

void parse_cache(vw& all, po::variables_map &vm, string source,
		 bool quiet)
{
  vector<string> caches;
  if (vm.count("cache_file"))
    caches = vm["cache_file"].as< vector<string> >();
  if (vm.count("cache"))
    caches.push_back(source+string(".cache"));

  all.p->write_cache = false;

  for (size_t i = 0; i < caches.size(); i++)
    {
      int f = -1;
      if (!vm.count("kill_cache"))
        f = all.p->input->open_file(caches[i].c_str(), all.stdin_off, io_buf::READ);
      if (f == -1)
	make_write_cache(all, caches[i], quiet);
      else {
	uint32_t c = cache_numbits(all.p->input, f);
	if (all.default_bits)
	  all.num_bits = c;
	if (c < all.num_bits) {
          all.p->input->close_file();          
	  make_write_cache(all, caches[i], quiet);
	}
	else {
	  if (!quiet)
	    cerr << "using cache_file = " << caches[i].c_str() << endl;
	  all.p->reader = read_cached_features;
	  if (c == all.num_bits)
	    all.p->sorted_cache = true;
	  else
	    all.p->sorted_cache = false;
	  all.p->resettable = true;
	}
      }
    }
  
  all.parse_mask = (1 << all.num_bits) - 1;
  if (caches.size() == 0)
    {
      if (!quiet)
	cerr << "using no cache" << endl;
      all.p->output->space.delete_v();
    }
}

//For macs
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif


void parse_source_args(vw& all, po::variables_map& vm, bool quiet, size_t passes)
{
  all.p->input->current = 0;
  parse_cache(all, vm, all.data_filename, quiet);

  string hash_function("strings");
  if(vm.count("hash")) 
    hash_function = vm["hash"].as<string>();
  
  if (all.daemon || all.active)
    {
      all.p->bound_sock = (int)socket(PF_INET, SOCK_STREAM, 0);
      if (all.p->bound_sock < 0) {
	cerr << "can't open socket!" << endl;
	throw exception();
      }

      int on = 1;
      if (setsockopt(all.p->bound_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
	perror("setsockopt SO_REUSEADDR");

      sockaddr_in address;
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_ANY);
      short unsigned int port = 26542;
      if (vm.count("port"))
	port = (uint16_t)vm["port"].as<size_t>();
      address.sin_port = htons(port);

      // attempt to bind to socket
      if ( ::bind(all.p->bound_sock,(sockaddr*)&address, sizeof(address)) < 0 )
	{
	  cerr << "failure to bind!" << endl;
	  throw exception();
	}
      int source_count = 1;
      
      // listen on socket
      listen(all.p->bound_sock, source_count);

      // background process
      if (!all.active && daemon(1,1))
	{
	  cerr << "failure to background!" << endl;
	  throw exception();
	}
      // write pid file
      if (vm.count("pid_file"))
	{
	  ofstream pid_file;
	  pid_file.open(vm["pid_file"].as<string>().c_str());
	  if (!pid_file.is_open())
	    {
	      cerr << "error writing pid file" << endl;
	      throw exception();
	    }
	  pid_file << getpid() << endl;
	  pid_file.close();
	}

      if (all.daemon && !all.active)
	{
#ifdef _WIN32
		throw exception();
#else
	  // weights will be shared across processes, accessible to children
	  float* shared_weights = 
	    (float*)mmap(0,all.reg.stride * all.length() * sizeof(float), 
			 PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	  size_t float_count = all.reg.stride * all.length();
	  weight* dest = shared_weights;
	  memcpy(dest, all.reg.weight_vector, float_count*sizeof(float));
	  free(all.reg.weight_vector);
	  all.reg.weight_vector = dest;
	  
	  // learning state to be shared across children
	  shared_data* sd = (shared_data *)mmap(0,sizeof(shared_data),
			 PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	  memcpy(sd, all.sd, sizeof(shared_data));
	  free(all.sd);
	  all.sd = sd;

	  // create children
	  size_t num_children = all.num_children;
	  v_array<int> children;
	  children.resize(num_children);
	  for (size_t i = 0; i < num_children; i++)
	    {
	      // fork() returns pid if parent, 0 if child
	      // store fork value and run child process if child
	      if ((children[i] = fork()) == 0)
		goto child;
	    }

	  // install signal handler so we can kill children when killed
	  {
	    struct sigaction sa;
	    // specifically don't set SA_RESTART in sa.sa_flags, so that
	    // waitid will be interrupted by SIGTERM with handler installed
	    memset(&sa, 0, sizeof(sa));
	    sa.sa_handler = handle_sigterm;
	    sigaction(SIGTERM, &sa, NULL);
	  }

	  while (true)
	    {
	      // wait for child to change state; if finished, then respawn
	      int status;
	      pid_t pid = wait(&status);
	      if (got_sigterm)
		{
		  for (size_t i = 0; i < num_children; i++)
		    kill(children[i], SIGTERM);
		  exit(0);
		}
	      if (pid < 0)
		continue;
	      for (size_t i = 0; i < num_children; i++)
		if (pid == children[i])
		  {
		    if ((children[i]=fork()) == 0)
		      goto child;
		    break;
		  }
	    }

#endif
	}

#ifndef _WIN32
	child:
#endif
      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      all.p->max_fd = 0;
      if (!all.quiet)
	cerr << "calling accept" << endl;
      int f = (int)accept(all.p->bound_sock,(sockaddr*)&client_address,&size);
      if (f < 0)
	{
	  cerr << "bad client socket!" << endl;
	  throw exception();
	}
      
      all.p->label_sock = f;
      all.print = print_result;
      
      all.final_prediction_sink.push_back((size_t) f);
      
      all.p->input->files.push_back(f);
      all.p->max_fd = max(f, all.p->max_fd);
      if (!all.quiet)
	cerr << "reading data from port " << port << endl;

      all.p->max_fd++;
      if(all.active)
	{
	  all.p->reader = read_features;
	  all.p->hasher = getHasher(hash_function);
	}
      else {
	if (isbinary(*(all.p->input))) {
	  all.p->reader = read_cached_features;
	  all.print = binary_print_result;
	} else {
	  all.p->reader = read_features;
	  
	}
	all.p->hasher = getHasher(hash_function);
	all.p->sorted_cache = true;
      }

      all.p->resettable = all.p->write_cache || all.daemon;
    }
  
  else  // was: else if (vm.count("data"))
    {
      string hash_function("strings");
      if(vm.count("hash")) 
	hash_function = vm["hash"].as<string>();

      if (all.p->input->files.size() > 0)
	{
	  if (!quiet)
	    cerr << "ignoring text input in favor of cache input" << endl;
	}
      else
	{
	  string temp = all.data_filename;
	  if (!quiet)
	    cerr << "Reading datafile = " << temp << endl;
	  int f = all.p->input->open_file(temp.c_str(), all.stdin_off, io_buf::READ);
	  if (f == -1 && temp.size() != 0)
	    {
			cerr << "can't open '" << temp << "', sailing on!" << endl;
	    }
	  all.p->reader = read_features;
	  all.p->hasher = getHasher(hash_function);
	  all.p->resettable = all.p->write_cache;
	}
    }

  if (passes > 1 && !all.p->resettable)
    {
      cerr << all.program_name << ": need a cache file for multiple passes: try using --cache_file" << endl;  
      throw exception();
    }
  all.p->input->count = all.p->input->files.size();
  if (!quiet)
    cerr << "num sources = " << all.p->input->files.size() << endl;
}

bool parser_done(parser* p)
{
  if (p->done)
    {
      if (p->used_index != p->parsed_examples)
	return false;
      return true;
    }
  return false;
}

void addgrams(vw& all, size_t ngram, size_t skip_gram, v_array<feature>& atomics, v_array<audit_data>& audits,
	      size_t initial_length, v_array<size_t> &gram_mask, size_t skips)
{
  if (ngram == 0 && gram_mask.last() < initial_length)
    {
      size_t last = initial_length - gram_mask.last();
      for(size_t i = 0; i < last; i++)
	{
	  size_t new_index = atomics[i].weight_index;
	  for (size_t n = 1; n < gram_mask.size(); n++)
	    new_index = new_index*quadratic_constant + atomics[i+gram_mask[n]].weight_index;
	  feature f = {1.,(uint32_t)(new_index)};
	  atomics.push_back(f);
	  if ((all.audit || all.hash_inv) && audits.size() >= initial_length)
	    {
	      string feature_name(audits[i].feature);
	      for (size_t n = 1; n < gram_mask.size(); n++)
		{
		  feature_name += string("^");
		  feature_name += string(audits[i+gram_mask[n]].feature);
		}
	      string feature_space = string(audits[i].space);
	      
	      audit_data a_feature = {NULL,NULL,new_index, 1., true};
	      a_feature.space = (char*)malloc(feature_space.length()+1);
	      strcpy(a_feature.space, feature_space.c_str());
	      a_feature.feature = (char*)malloc(feature_name.length()+1);
	      strcpy(a_feature.feature, feature_name.c_str());
	      audits.push_back(a_feature);
	    }
	}
    }
  if (ngram > 0)
    {
      gram_mask.push_back(gram_mask.last()+1+skips);
      addgrams(all, ngram-1, skip_gram, atomics, audits, initial_length, gram_mask, 0);
      gram_mask.pop();
    }
  if (skip_gram > 0 && ngram > 0)
    addgrams(all, ngram, skip_gram-1, atomics, audits, initial_length, gram_mask, skips+1);
}

/**
 * This function adds k-skip-n-grams to the feature vector.
 * Definition of k-skip-n-grams:
 * Consider a feature vector - a, b, c, d, e, f
 * 2-skip-2-grams would be - ab, ac, ad, bc, bd, be, cd, ce, cf, de, df, ef
 * 1-skip-3-grams would be - abc, abd, acd, ace, bcd, bce, bde, bdf, cde, cdf, cef, def
 * Note that for a n-gram, (n-1)-grams, (n-2)-grams... 2-grams are also appended
 * The k-skip-n-grams are appended to the feature vector.
 * Hash is evaluated using the principle h(a, b) = h(a)*X + h(b), where X is a random no.
 * 32 random nos. are maintained in an array and are used in the hashing.
 */
void generateGrams(vw& all, example* &ex) {
  for(unsigned char* index = ex->indices.begin; index < ex->indices.end; index++)
    {
      size_t length = ex->atomics[*index].size();
      for (size_t n = 1; n < all.ngram[*index]; n++)
	{
	  all.p->gram_mask.erase();
	  all.p->gram_mask.push_back((size_t)0);
	  addgrams(all, n, all.skips[*index], ex->atomics[*index], 
		   ex->audit_features[*index], 
		   length, all.p->gram_mask, 0);
	}
    }
}

example* get_unused_example(vw& all)
{
  while (true)
    {
      mutex_lock(&all.p->examples_lock);
      if (all.p->examples[all.p->parsed_examples % all.p->ring_size].in_use == false)
	{
	  all.p->examples[all.p->parsed_examples % all.p->ring_size].in_use = true;
	  mutex_unlock(&all.p->examples_lock);
	  return all.p->examples + (all.p->parsed_examples % all.p->ring_size);
	}
      else 
	condition_variable_wait(&all.p->example_unused, &all.p->examples_lock);
      mutex_unlock(&all.p->examples_lock);
    }
}

bool parse_atomic_example(vw& all, example* ae, bool do_read = true)
{
  if (do_read && all.p->reader(&all, ae) <= 0)
    return false;

  if(all.p->sort_features && ae->sorted == false)
    unique_sort_features(all.audit, ae);

  if (all.p->write_cache) 
    {
      all.p->lp->cache_label(ae->ld,*(all.p->output));
      cache_features(*(all.p->output), ae, (uint32_t)all.parse_mask);
    }

  return true;
}

void end_pass_example(vw& all, example* ae)
{
  all.p->lp->default_label(ae->ld);
  ae->end_pass = true;
  all.p->in_pass_counter = 0;
}

void setup_example(vw& all, example* ae)
{
  ae->partial_prediction = 0.;
  ae->num_features = 0;
  ae->total_sum_feat_sq = 0;
  ae->loss = 0.;
  
  ae->example_counter = (size_t)(all.p->parsed_examples + 1);
  if ((!all.p->emptylines_separate_examples) || example_is_newline(ae))
    all.p->in_pass_counter++;

  ae->test_only = is_test_only(all.p->in_pass_counter, all.holdout_period, all.holdout_after, all.holdout_set_off);
  ae->global_weight = all.p->lp->get_weight(ae->ld);
  all.sd->t += ae->global_weight;
  ae->example_t = (float)all.sd->t;

  if (all.ignore_some)
    {
      if (all.audit || all.hash_inv)
	for (unsigned char* i = ae->indices.begin; i != ae->indices.end; i++)
	  if (all.ignore[*i])
	    ae->audit_features[*i].erase();
      
      for (unsigned char* i = ae->indices.begin; i != ae->indices.end; i++)
	if (all.ignore[*i])
	  {//delete namespace
	    ae->atomics[*i].erase();
	    memmove(i,i+1,(ae->indices.end - (i+1))*sizeof(*i));
	    ae->indices.end--;
	    i--;
	  }
    }

  if(all.ngram_strings.size() > 0)
    generateGrams(all, ae);    

  if (all.add_constant) {
    //add constant feature
    ae->indices.push_back(constant_namespace);
    feature temp = {1,(uint32_t) (constant * all.wpp)};
    ae->atomics[constant_namespace].push_back(temp);
    ae->total_sum_feat_sq++;
  }
  
  if(all.reg.stride != 1) //make room for per-feature information.
    {
      uint32_t stride = all.reg.stride;
      for (unsigned char* i = ae->indices.begin; i != ae->indices.end; i++)
	for(feature* j = ae->atomics[*i].begin; j != ae->atomics[*i].end; j++)
	  j->weight_index = j->weight_index*stride;
      if (all.audit || all.hash_inv)
	for (unsigned char* i = ae->indices.begin; i != ae->indices.end; i++)
	  for(audit_data* j = ae->audit_features[*i].begin; j != ae->audit_features[*i].end; j++)
	    j->weight_index = j->weight_index*stride;
    }
  
  for (unsigned char* i = ae->indices.begin; i != ae->indices.end; i++) 
    {
      ae->num_features += ae->atomics[*i].end - ae->atomics[*i].begin;
      ae->total_sum_feat_sq += ae->sum_feat_sq[*i];
    }

  if (all.rank == 0) {
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++)
      {
	ae->num_features 
	  += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin)
	  *(ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin);
	ae->total_sum_feat_sq += ae->sum_feat_sq[(int)(*i)[0]]*ae->sum_feat_sq[(int)(*i)[1]];
      }

    for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++)
      {
	ae->num_features 
	  += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin)
            *(ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin)
            *(ae->atomics[(int)(*i)[2]].end - ae->atomics[(int)(*i)[2]].begin);
	ae->total_sum_feat_sq += ae->sum_feat_sq[(int)(*i)[0]] * ae->sum_feat_sq[(int)(*i)[1]] * ae->sum_feat_sq[(int)(*i)[2]];
      }

  } else {
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++)
      {
	ae->num_features
	  += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin) * all.rank;
	ae->num_features
	  += (ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin) * all.rank;
      }
    for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++)
      {
	ae->num_features
	  += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin) * all.rank;
	ae->num_features
	  += (ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin) * all.rank;
	ae->num_features
	  += (ae->atomics[(int)(*i)[2]].end - ae->atomics[(int)(*i)[2]].begin) * all.rank;
      }
  }
}

namespace VW{
  example* new_unused_example(vw& all) { 
    example* ec = get_unused_example(all);
    all.p->lp->default_label(ec->ld);
    all.p->parsed_examples++;
    ec->example_counter = all.p->parsed_examples;
    return ec;
  }
  example* read_example(vw& all, char* example_line)
  {
    example* ret = get_unused_example(all);

    read_line(all, ret, example_line);
	parse_atomic_example(all,ret,false);
    setup_example(all, ret);
    all.p->parsed_examples++;

    return ret;
  }

  void add_constant_feature(vw& vw, example*ec) {
    uint32_t cns = constant_namespace;
    ec->indices.push_back(cns);
    feature temp = {1,(uint32_t) constant};
    ec->atomics[cns].push_back(temp);
    ec->total_sum_feat_sq++;
    ec->num_features++;
  }

  void add_label(example* ec, float label, float weight, float base)
  {
    label_data* l = (label_data*)ec->ld;
    l->label = label;
    l->weight = weight;
    l->initial = base;
  }

  example* import_example(vw& all, vector<feature_space> vf)
  {
    example* ret = get_unused_example(all);
    all.p->lp->default_label(ret->ld);
    for (size_t i = 0; i < vf.size();i++)
      {
	uint32_t index = vf[i].first;
	ret->indices.push_back(index);
	for (size_t j = 0; j < vf[i].second.size(); j++)
	  {	    
	    ret->sum_feat_sq[index] += vf[i].second[j].x * vf[i].second[j].x;
	    ret->atomics[index].push_back(vf[i].second[j]);
	  }
      }
	parse_atomic_example(all,ret,false);
    setup_example(all, ret);
    all.p->parsed_examples++;
    return ret;
  }

  example* import_example(vw& all, primitive_feature_space* features, size_t len)
  {
    example* ret = get_unused_example(all);
    all.p->lp->default_label(ret->ld);
    for (size_t i = 0; i < len;i++)
      {
	uint32_t index = features[i].name;
	ret->indices.push_back(index);
	for (size_t j = 0; j < features[i].len; j++)
	  {	    
	    ret->sum_feat_sq[index] += features[i].fs[j].x * features[i].fs[j].x;
	    ret->atomics[index].push_back(features[i].fs[j]);
	  }
      }
    parse_atomic_example(all,ret,false); // all.p->parsed_examples++;
    setup_example(all, ret);
    
    return ret;
  }

  primitive_feature_space* export_example(vw& all, example* ec, size_t& len)
  {
    len = ec->indices.size();
    primitive_feature_space* fs_ptr = new primitive_feature_space[len]; 
    
    int fs_count = 0;
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
      {
		fs_ptr[fs_count].name = *i;
		fs_ptr[fs_count].len = ec->atomics[*i].size();
		fs_ptr[fs_count].fs = new feature[fs_ptr[fs_count].len];
	
		int f_count = 0;
		for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
		  {
			feature t = *f;
			t.weight_index /= all.reg.stride;
			fs_ptr[fs_count].fs[f_count] = t;
			f_count++;
		  }
		fs_count++;
      }
    return fs_ptr;
  }
  
  void releaseFeatureSpace(primitive_feature_space* features, size_t len)
  {
    for (size_t i = 0; i < len;i++)
      delete features[i].fs;
    delete (features);
  }

  void parse_example_label(vw& all, example&ec, string label) {
    v_array<substring> words;
    char* cstr = (char*)label.c_str();
    substring str = { cstr, cstr+label.length() };
    words.push_back(str);
    all.p->lp->parse_label(all.p, all.sd, ec.ld, words);
    words.erase();
    words.delete_v();
  }

  void empty_example(vw& all, example* ec)
  {
	if (all.audit || all.hash_inv)
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
	{
	  for (audit_data* temp 
		 = ec->audit_features[*i].begin; 
	       temp != ec->audit_features[*i].end; temp++)
	    {
	      if (temp->alloced)
		{
		  free(temp->space);
		  free(temp->feature);
		  temp->alloced=false;
		}
	    }
	  ec->audit_features[*i].erase();
	}
    
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
      {  
	ec->atomics[*i].erase();
	ec->sum_feat_sq[*i]=0;
      }
    
    ec->indices.erase();
    ec->tag.erase();
    ec->sorted = false;
    ec->end_pass = false;
  }

  void finish_example(vw& all, example* ec)
  {
    mutex_lock(&all.p->output_lock);
    all.p->local_example_number++;
    condition_variable_signal(&all.p->output_done);
    mutex_unlock(&all.p->output_lock);
    
    empty_example(all, ec);
    
    mutex_lock(&all.p->examples_lock);
    assert(ec->in_use);
    ec->in_use = false;
    condition_variable_signal(&all.p->example_unused);
    if (all.p->done)
      condition_variable_signal_all(&all.p->example_available);
    mutex_unlock(&all.p->examples_lock);
  }
}

#ifdef _WIN32
DWORD WINAPI main_parse_loop(LPVOID in)
#else
void *main_parse_loop(void *in)
#endif
{
	vw* all = (vw*) in;
	size_t example_number = 0;  // for variable-size batch learning algorithms


	while(!all->p->done)
	  {
            example* ae = get_unused_example(*all);
	    if (!all->do_reset_source && example_number != all->pass_length && all->max_examples > example_number
		   && parse_atomic_example(*all, ae) )
	     {
	       setup_example(*all, ae);
	       example_number++;
	     }
	    else
	     {
	       reset_source(*all, all->num_bits);
	       all->do_reset_source = false;
	       all->passes_complete++;
	       end_pass_example(*all, ae);
	       if (all->passes_complete == all->numpasses && example_number == all->pass_length)
			 {
			   all->passes_complete = 0;
			   all->pass_length = all->pass_length*2+1;
			 }
	       if (all->passes_complete >= all->numpasses && all->max_examples >= example_number)
			 {
			   mutex_lock(&all->p->examples_lock);
			   all->p->done = true;
			   mutex_unlock(&all->p->examples_lock);
			 }
	       example_number = 0;
	     }
	   mutex_lock(&all->p->examples_lock);
	   all->p->parsed_examples++;
	   condition_variable_signal_all(&all->p->example_available);
	   mutex_unlock(&all->p->examples_lock);

	  }  
	return NULL;
}

namespace VW{
example* get_example(parser* p)
{
  mutex_lock(&p->examples_lock);
  if (p->parsed_examples != p->used_index) {
    size_t ring_index = p->used_index++ % p->ring_size;
    if (!(p->examples+ring_index)->in_use)
      cout << p->used_index << " " << p->parsed_examples << " " << ring_index << endl;
    assert((p->examples+ring_index)->in_use);
    mutex_unlock(&p->examples_lock);
    
    return p->examples + ring_index;
  }
  else {
    if (!p->done)
      {
	condition_variable_wait(&p->example_available, &p->examples_lock);
	mutex_unlock(&p->examples_lock);
	return get_example(p);
      }
    else {
      mutex_unlock(&p->examples_lock);
      return NULL;
    }
  }
}

label_data* get_label(example* ec)
{
	return (label_data*)(ec->ld);
}
}

void initialize_examples(vw& all)
{
  all.p->used_index = 0;
  all.p->parsed_examples = 0;
  all.p->done = false;

  all.p->examples = (example*)calloc(all.p->ring_size, sizeof(example));

  for (size_t i = 0; i < all.p->ring_size; i++)
    {
      all.p->examples[i].ld = calloc(1,all.p->lp->label_size);
      all.p->examples[i].in_use = false;
    }
}

void adjust_used_index(vw& all)
{
	all.p->used_index=all.p->parsed_examples;
}

void initialize_parser_datastructures(vw& all)
{
  initialize_examples(all);
  initialize_mutex(&all.p->examples_lock);
  initialize_condition_variable(&all.p->example_available);
  initialize_condition_variable(&all.p->example_unused);
  initialize_mutex(&all.p->output_lock);
  initialize_condition_variable(&all.p->output_done);
}

namespace VW {
void start_parser(vw& all, bool init_structures)
{
  if (init_structures)
	initialize_parser_datastructures(all);
  #ifndef _WIN32
  pthread_create(&all.parse_thread, NULL, main_parse_loop, &all);
  #else
  all.parse_thread = ::CreateThread(NULL, 0, static_cast<LPTHREAD_START_ROUTINE>(main_parse_loop), &all, NULL, NULL);
  #endif
}
}
void free_parser(vw& all)
{
  all.p->channels.delete_v();
  all.p->words.delete_v();
  all.p->name.delete_v();

  if(all.ngram_strings.size() > 0)
    all.p->gram_mask.delete_v();
  
  for (size_t i = 0; i < all.p->ring_size; i++) 
    {
      dealloc_example(all.p->lp->delete_label, all.p->examples[i]);
    }
  free(all.p->examples);
  
  io_buf* output = all.p->output;
  if (output != NULL)
    {
      output->finalname.delete_v();
      output->currentname.delete_v();
    }

  all.p->counts.delete_v();
}

void release_parser_datastructures(vw& all)
{
  delete_mutex(&all.p->examples_lock);
  delete_mutex(&all.p->output_lock);
}

namespace VW {
void end_parser(vw& all)
{
  #ifndef _WIN32
  pthread_join(all.parse_thread, NULL);
  #else
  ::WaitForSingleObject(all.parse_thread, INFINITE);
  ::CloseHandle(all.parse_thread);
  #endif
  release_parser_datastructures(all);
}
}
