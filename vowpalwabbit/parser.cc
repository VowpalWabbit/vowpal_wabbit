/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <signal.h>
#include <unistd.h>
#include <fstream>

#include <netdb.h>
#include <boost/program_options.hpp>

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

#include <netinet/tcp.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
namespace po = boost::program_options;

#include "parser.h"
#include "parse_example.h"
#include "cache.h"
#include "gd.h"
#include "comp_io.h"
#include "unique_sort.h"
#include "constant.h"

using namespace std;

example* examples;//A Ring of examples.
pthread_mutex_t examples_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t example_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t example_unused = PTHREAD_COND_INITIALIZER;
uint64_t used_index = 0; // The index of the example currently used by thread i.
bool done=false;
v_array<size_t> gram_mask;

bool got_sigterm = false;

void handle_sigterm (int)
{
  got_sigterm = true;
}

parser* new_parser()
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
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
  v_array<char> t;

  size_t v_length;
  buf->read_file(filepointer, (char*)&v_length, sizeof(v_length));
  if(v_length>29){
    cerr << "cache version too long, cache file is probably invalid" << endl;
    exit(1);
  }
  t.erase();
  if (t.index() < v_length)
    reserve(t,v_length);
  
  buf->read_file(filepointer,t.begin,v_length);
  if (strcmp(t.begin,version.c_str()) != 0)
    {
      cout << "cache has possibly incompatible version, rebuilding" << endl;
      free(t.begin);
      return 0;
    }
  free(t.begin);
  
  const int total = sizeof(size_t);
  char* p[total];
  if (buf->read_file(filepointer, p, total) < total) 
    {
      return true;
    }

  size_t cache_numbits = *(size_t *)p;
  return cache_numbits;
}

bool member(v_array<size_t> ids, size_t id)
{
  for (size_t i = 0; i < ids.index(); i++)
    if (ids[i] == id)
      return true;
  return false;
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
      while(input->files.index() > 0)
	{
	  int fd = input->files.pop();
	  if (!member(global.final_prediction_sink, (size_t) fd))
	    close(fd);
	}
      input->open_file(p->output->finalname.begin,io_buf::READ); //pushing is merged into open_file
      p->reader = read_cached_features;
    }
  if ( p->resettable == true )
    {
      if (global.daemon)
	{
	  // wait for all predictions to be sent back to client
	  pthread_mutex_lock(&output_lock);
	  while (global.local_example_number != global.parsed_examples)
	    pthread_cond_wait(&output_done, &output_lock);
	  pthread_mutex_unlock(&output_lock);
	  
	  // close socket, erase final prediction sink and socket
	  close(p->input->files[0]);
	  global.final_prediction_sink.erase();
	  p->input->files.erase();
	  
	  sockaddr_in client_address;
	  socklen_t size = sizeof(client_address);
	  int f = accept(p->bound_sock,(sockaddr*)&client_address,&size);
	  if (f < 0)
	    {
	      cerr << "bad client socket!" << endl;
	      exit (1);
	    }
	  
	  // note: breaking cluster parallel online learning by dropping support for id
	  
	  push(global.final_prediction_sink, (size_t) f);
	  push(p->input->files,f);

	  if (isbinary(*(p->input))) {
	    p->reader = read_cached_features;
	    global.print = binary_print_result;
	  } else {
	    p->reader = read_features;
	    global.print = print_result;
	  }
	}
      else {
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
	  par->reader = read_cached_features;
	  if (c == global.num_bits)
	    par->sorted_cache = true;
	  else
	    par->sorted_cache = false;
	  par->resettable = true;
	}
      }
    }
  
  global.parse_mask = (1 << global.num_bits) - 1;
  if (caches.size() == 0)
    {
      if (!quiet)
	cerr << "using no cache" << endl;
      reserve(par->output->space,0);
    }
}

//For macs
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif


void parse_source_args(po::variables_map& vm, parser* par, bool quiet, size_t passes)
{
  par->input->current = 0;
  parse_cache(vm, vm["data"].as<string>(), par, quiet);

  string hash_function("strings");
  if(vm.count("hash")) 
    hash_function = vm["hash"].as<string>();
  
  if (global.daemon)
    {
      par->bound_sock = socket(PF_INET, SOCK_STREAM, 0);
      if (par->bound_sock < 0) {
	cerr << "can't open socket!" << endl;
	exit(1);
      }

      int on = 1;
      if (setsockopt(par->bound_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
	perror("setsockopt SO_REUSEADDR");

      sockaddr_in address;
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_ANY);
      short unsigned int port = 26542;
      if (vm.count("port"))
	port = vm["port"].as<size_t>();
      address.sin_port = htons(port);

      // attempt to bind to socket
      if ( ::bind(par->bound_sock,(sockaddr*)&address, sizeof(address)) < 0 )
	{
	  cerr << "failure to bind!" << endl;
	  exit(1);
	}
      int source_count = 1;
      
      // listen on socket
      listen(par->bound_sock, source_count);

      // background process
      if (daemon(1,1))
	{
	  cerr << "failure to background!" << endl;
	  exit(1);
	}
      // write pid file
      if (vm.count("pid_file"))
	{
	  ofstream pid_file;
	  pid_file.open(vm["pid_file"].as<string>().c_str());
	  if (!pid_file.is_open())
	    {
	      cerr << "error writing pid file" << endl;
	      exit(1);
	    }
	  pid_file << getpid() << endl;
	  pid_file.close();
	}

      if (global.daemon)
	{
	  // weights will be shared across processes, accessible to children
	  float* shared_weights = 
	    (float*)mmap(0,global.stride * global.length() * sizeof(float), 
			 PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	  size_t float_count = global.stride * global.length();
	  weight* dest = shared_weights;
	  memcpy(dest, global.reg.weight_vectors, float_count*sizeof(float));
	  free(global.reg.weight_vectors);
	  global.reg.weight_vectors = dest;
	  
	  // learning state to be shared across children
	  shared_data* sd = (shared_data *)mmap(0,sizeof(shared_data),
			 PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	  memcpy(sd, global.sd, sizeof(shared_data));
	  free(global.sd);
	  global.sd = sd;

	  // create children
	  size_t num_children = global.num_children;
	  v_array<int> children;
	  reserve(children, num_children);
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

	}

    child:
      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      par->max_fd = 0;
      if (!global.quiet)
	cerr << "calling accept" << endl;
      int f = accept(par->bound_sock,(sockaddr*)&client_address,&size);
      if (f < 0)
	{
	  cerr << "bad client socket!" << endl;
	  exit (1);
	}
      
      par->label_sock = f;
      global.print = print_result;
      
      push(global.final_prediction_sink, (size_t) f);
      
      push(par->input->files,f);
      par->max_fd = max(f, par->max_fd);
      if (!global.quiet)
	cerr << "reading data from port " << port << endl;

      par->max_fd++;
      if(global.active)
	{
	  par->reader = read_features;
	  par->hasher = getHasher(hash_function);
	}
      else {
	if (isbinary(*(par->input))) {
	  par->reader = read_cached_features;
	  global.print = binary_print_result;
	} else {
	  par->reader = read_features;
	  
	}
	par->hasher = getHasher(hash_function);
	par->sorted_cache = true;
      }

      par->resettable = par->write_cache || global.daemon;
    }
  
  else if (vm.count("data"))
    {
      string hash_function("strings");
      if(vm.count("hash")) 
	hash_function = vm["hash"].as<string>();
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
	      par->hasher = getHasher(hash_function);
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
	  par->hasher = getHasher(hash_function);
	  par->resettable = par->write_cache;
	}
    }

  if (passes > 1 && !par->resettable)
    {
      cerr << global.program_name << ": need a cache file for multiple passes: try using --cache_file" << endl;  
      exit(1);
    }
  par->input->count = par->input->files.index();
  if (!quiet)
    cerr << "num sources = " << par->input->files.index() << endl;
}

bool parser_done()
{
  if (done)
    {
      if (used_index != global.parsed_examples)
	return false;
      return true;
    }
  return false;
}

void addgrams(size_t ngram, size_t skip_gram, v_array<feature>& atomics, v_array<audit_data>& audits,
	      size_t initial_length, v_array<size_t> &gram_mask, size_t skips)
{
  if (ngram == 0 && gram_mask.last() < initial_length)
    {
      size_t last = initial_length - gram_mask.last();
      for(size_t i = 0; i < last; i++)
	{
	  size_t new_index = atomics[i].weight_index;
	  for (size_t n = 1; n < gram_mask.index(); n++)
	    new_index = new_index*quadratic_constant + atomics[i+gram_mask[n]].weight_index;
	  feature f = {1.,(uint32_t)(new_index & global.parse_mask)};
	  push(atomics,f);
	  if (global.audit && audits.index() >= initial_length)
	    {
	      string feature_name(audits[i].feature);
	      for (size_t n = 1; n < gram_mask.index(); n++)
		{
		  feature_name += string("^");
		  feature_name += string(audits[i+gram_mask[n]].feature);
		}
	      string feature_space = string(audits[i].space);
	      
	      audit_data a_feature = {NULL,NULL,new_index & global.parse_mask, 1., true};
	      a_feature.space = (char*)malloc(feature_space.length()+1);
	      strcpy(a_feature.space, feature_space.c_str());
	      a_feature.feature = (char*)malloc(feature_name.length()+1);
	      strcpy(a_feature.feature, feature_name.c_str());
	      push(audits, a_feature);
	    }
	}
    }
  if (ngram > 0)
    {
      push(gram_mask,gram_mask.last()+1+skips);
      addgrams(ngram-1, skip_gram, atomics, audits, initial_length, gram_mask, 0);
      gram_mask.pop();
    }
  if (skip_gram > 0 && ngram > 0)
    addgrams(ngram, skip_gram-1, atomics, audits, initial_length, gram_mask, skips+1);
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
void generateGrams(size_t ngram, size_t skip_gram, example * &ex) {
  for(size_t *index = ex->indices.begin; index < ex->indices.end; index++)
    {
      size_t length = ex->atomics[*index].index();
      for (size_t n = 1; n < ngram; n++)
	{
	  gram_mask.erase();
	  push(gram_mask,(size_t)0);
	  addgrams(n, skip_gram, ex->atomics[*index], 
		   ex->audit_features[*index], 
		   length, gram_mask, 0);
	}
    }
}

example* get_unused_example()
{
  while (true)
    {
      pthread_mutex_lock(&examples_lock);
      if (examples[global.parsed_examples % global.ring_size].in_use == false)
	{
	  examples[global.parsed_examples % global.ring_size].in_use = true;
	  pthread_mutex_unlock(&examples_lock);
	  return examples + (global.parsed_examples % global.ring_size);
	}
      else 
	{
	  pthread_cond_wait(&example_unused, &examples_lock);
	}
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
  {  
    ae->atomics[*i].erase();
    ae->sum_feat_sq[*i]=0;
  }

  ae->indices.erase();
  ae->tag.erase();
  ae->sorted = false;
  if (p->reader(p,ae) <= 0)
    return false;

  if(p->sort_features && ae->sorted == false)
    unique_sort_features(ae);

  if (p->write_cache) 
    {
      global.lp->cache_label(ae->ld,*(p->output));
      cache_features(*(p->output), ae);
    }

  if(global.ngram > 1)
    generateGrams(global.ngram, global.skips, ae);
    
  return true;
}

void setup_example(parser* p, example* ae)
{
  ae->pass = global.passes_complete;
  ae->partial_prediction = 0.;
  ae->num_features = 0;
  ae->total_sum_feat_sq = 0;
  ae->done = false;
  ae->example_counter = global.parsed_examples + 1;
  ae->global_weight = global.lp->get_weight(ae->ld);
  global.sd->t += ae->global_weight;
  ae->example_t = global.sd->t;

  if (global.ignore_some)
    {
      for (size_t* i = ae->indices.begin; i != ae->indices.end; i++)
	if (global.ignore[*i])
	  {//delete namespace
	    ae->atomics[*i].erase();
	    memmove(i,i+1,(ae->indices.end - (i+1))*sizeof(size_t));
	    ae->indices.end--;
	    i--;
	  }
    }

  if (global.add_constant) {
    //add constant feature
    push(ae->indices,constant_namespace);
    feature temp = {1,(uint32_t) (constant & global.parse_mask)};
    push(ae->atomics[constant_namespace], temp);
    ae->total_sum_feat_sq++;
  }
  
  if(global.stride != 1) //make room for per-feature information.
    {
      size_t stride = global.stride;
      for (size_t* i = ae->indices.begin; i != ae->indices.end; i++)
	for(feature* j = ae->atomics[*i].begin; j != ae->atomics[*i].end; j++)
	  j->weight_index = j->weight_index*stride;
      if (global.audit)
	for (size_t* i = ae->indices.begin; i != ae->indices.end; i++)
	  for(audit_data* j = ae->audit_features[*i].begin; j != ae->audit_features[*i].end; j++)
	    j->weight_index = j->weight_index*stride;
    }
  
  for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
    {
      ae->num_features += ae->atomics[*i].end - ae->atomics[*i].begin;
      ae->total_sum_feat_sq += ae->sum_feat_sq[*i];
    }

  if (global.rank == 0)                                                                        
    for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++)
      {
	ae->num_features 
	  += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin)
	  *(ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin);
	ae->total_sum_feat_sq += ae->sum_feat_sq[(int)(*i)[0]]*ae->sum_feat_sq[(int)(*i)[1]];
      }
  else
    for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++)
      {
	ae->num_features
	  += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin) * global.rank;
	ae->num_features
	  += (ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin)
	  *global.rank;
      }                                                                 
}

void *main_parse_loop(void *in)
{
  parser* p = (parser*) in;
  
  global.passes_complete = 0;
  size_t example_number = 0;  // for variable-size batch learning algorithms
  while(!done)
    {
      example* ae=get_unused_example();

      if (example_number != global.pass_length && parse_atomic_example(p,ae)) {	
	setup_example(p,ae);
	example_number++;
	pthread_mutex_lock(&examples_lock);
	global.parsed_examples++;
	pthread_cond_broadcast(&example_available);
	pthread_mutex_unlock(&examples_lock);
      }
      else
	{
	  reset_source(global.num_bits, p);
	  global.passes_complete++;
	  if (global.passes_complete == global.numpasses && example_number == global.pass_length)
	    {
	      global.passes_complete = 0;
	      global.pass_length = global.pass_length*2+1;
	    }
	  example_number = 0;
	  if (global.passes_complete >= global.numpasses)
	    {
	      pthread_mutex_lock(&examples_lock);
	      done = true;
	      pthread_mutex_unlock(&examples_lock);
	    }
	  pthread_mutex_lock(&examples_lock);
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

void free_example(example* ec)
{
  pthread_mutex_lock(&output_lock);
  global.local_example_number++;
  pthread_cond_signal(&output_done);
  pthread_mutex_unlock(&output_lock);

  pthread_mutex_lock(&examples_lock);
  assert(ec->in_use);
  ec->in_use = false;
  pthread_cond_signal(&example_unused);
  if (done)
    pthread_cond_broadcast(&example_available);
  pthread_mutex_unlock(&examples_lock);
}

example* get_example()
{
  pthread_mutex_lock(&examples_lock);

  if (global.parsed_examples != used_index) {
    size_t ring_index = used_index++ % global.ring_size;
    if (!(examples+ring_index)->in_use)
      cout << used_index << " " << global.parsed_examples << " " << ring_index << endl;
    assert((examples+ring_index)->in_use);
    pthread_mutex_unlock(&examples_lock);
    
    return examples + ring_index;
  }
  else {
    if (!done)
      {
	pthread_cond_wait(&example_available, &examples_lock);
	pthread_mutex_unlock(&examples_lock);
	return get_example();
      }
    else {
      pthread_mutex_unlock(&examples_lock);
      return NULL;
    }
  }
}

pthread_t parse_thread;

void start_parser(parser* pf)
{
  used_index = 0;
  global.parsed_examples = 0;
  done = false;

  examples = (example*)calloc(global.ring_size, sizeof(example));

  for (size_t i = 0; i < global.ring_size; i++)
    {
      examples[i].ld = calloc(1,global.lp->label_size);
      examples[i].in_use = false;
    }
  pthread_create(&parse_thread, NULL, main_parse_loop, pf);
}

void end_parser(parser* pf)
{
  pthread_join(parse_thread, NULL);

  if(global.ngram > 1)
    {
      if(gram_mask.begin != NULL) reserve(gram_mask,0);
    }

  for (size_t i = 0; i < global.ring_size; i++) 
    {
      global.lp->delete_label(examples[i].ld);
      if (examples[i].tag.end_array != examples[i].tag.begin)
	{
	  free(examples[i].tag.begin);
	  examples[i].tag.end_array = examples[i].tag.begin;
	}
      
      if (global.lda > 0)
	free(examples[i].topic_predictions.begin);

      free(examples[i].ld);
      for (size_t j = 0; j < 256; j++)
	{
	  if (examples[i].atomics[j].begin != examples[i].atomics[j].end_array)
	    free(examples[i].atomics[j].begin);

	  if (examples[i].audit_features[j].begin != examples[i].audit_features[j].end_array)
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
	}
      free(examples[i].indices.begin);
    }
  free(examples);
  
  io_buf* output = pf->output;
  if (output != NULL)
    {
      if (output->finalname.begin != NULL)
	free(output->finalname.begin);
      if (output->currentname.begin != NULL)
	free(output->currentname.begin);
    }

  if (pf->counts.begin != NULL)
    free(pf->counts.begin);
}

