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
{ exit(0);
  return 0;
}
int getpid()
{ return (int) ::GetCurrentProcessId();
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

#include "parse_example.h"
#include "cache.h"
#include "unique_sort.h"
#include "constant.h"
#include "vw.h"
#include "interactions.h"
#include "vw_exception.h"
#include "parse_example_json.h"

using namespace std;

void initialize_mutex(MUTEX * pm)
{
#ifndef _WIN32
  pthread_mutex_init(pm, nullptr);
#else
  ::InitializeCriticalSection(pm);
#endif
}

#ifndef _WIN32
void delete_mutex(MUTEX *) { /* no operation necessary here*/ }
#else
void delete_mutex(MUTEX * pm)
{ ::DeleteCriticalSection(pm);
}
#endif

void initialize_condition_variable(CV * pcv)
{
#ifndef _WIN32
  pthread_cond_init(pcv, nullptr);
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
{ got_sigterm = true;
}

bool is_test_only(uint32_t counter, uint32_t period, uint32_t after, bool holdout_off, uint32_t target_modulus)  // target should be 0 in the normal case, or period-1 in the case that emptylines separate examples
{ if(holdout_off) return false;
  if (after == 0) // hold out by period
    return (counter % period == target_modulus);
  else // hold out by position
    return (counter >= after);
}

parser* new_parser()
{ parser& ret = calloc_or_throw<parser>();
  ret.input = new io_buf;
  ret.output = new io_buf;
  ret.local_example_number = 0;
  ret.in_pass_counter = 0;
  ret.ring_size = 1 << 8;
  ret.done = false;
  ret.used_index = 0;
  ret.jsonp = nullptr;

  return &ret;
}

void set_compressed(parser* par)
{ finalize_source(par);
  par->input = new comp_io_buf;
  par->output = new comp_io_buf;
}

uint32_t cache_numbits(io_buf* buf, int filepointer)
{ v_array<char> t = v_init<char>();

  try
  { size_t v_length;
    buf->read_file(filepointer, (char*)&v_length, sizeof(v_length));
    if (v_length > 61)
      THROW("cache version too long, cache file is probably invalid");

    if (v_length == 0)
      THROW("cache version too short, cache file is probably invalid");

    t.erase();
    if (t.size() < v_length)
      t.resize(v_length);

    buf->read_file(filepointer,t.begin(),v_length);
    version_struct v_tmp(t.begin());
    if ( v_tmp != version )
    { cout << "cache has possibly incompatible version, rebuilding" << endl;
      t.delete_v();
      return 0;
    }

    char temp;
    if (buf->read_file(filepointer, &temp, 1) < 1)
      THROW("failed to read");

    if (temp != 'c')
      THROW("data file is not a cache file");
  }
  catch(...)
  { t.delete_v();
  }

  t.delete_v();

  uint32_t cache_numbits;
  if (buf->read_file(filepointer, &cache_numbits, sizeof(cache_numbits)) < (int)sizeof(cache_numbits))
  { return true;
  }

  return cache_numbits;
}

bool member(v_array<int> ids, int id)
{ for (size_t i = 0; i < ids.size(); i++)
    if (ids[i] == id)
      return true;
  return false;
}

void reset_source(vw& all, size_t numbits)
{ io_buf* input = all.p->input;
  input->current = 0;
  if (all.p->write_cache)
  { all.p->output->flush();
    all.p->write_cache = false;
    all.p->output->close_file();
    remove(all.p->output->finalname.begin());
    rename(all.p->output->currentname.begin(), all.p->output->finalname.begin());
    while(input->num_files() > 0)
      if (input->compressed())
        input->close_file();
      else
      { int fd = input->files.pop();
        if (!member(all.final_prediction_sink, (size_t) fd))
          io_buf::close_file_or_socket(fd);
      }
    input->open_file(all.p->output->finalname.begin(), all.stdin_off, io_buf::READ); //pushing is merged into open_file
    all.p->reader = read_cached_features;
  }
  if ( all.p->resettable == true )
  { if (all.daemon)
    { // wait for all predictions to be sent back to client
      mutex_lock(&all.p->output_lock);
      while (all.p->local_example_number != all.p->end_parsed_examples)
        condition_variable_wait(&all.p->output_done, &all.p->output_lock);
      mutex_unlock(&all.p->output_lock);

      // close socket, erase final prediction sink and socket
      io_buf::close_file_or_socket(all.p->input->files[0]);
      all.final_prediction_sink.erase();
      all.p->input->files.erase();

      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      int f = (int)accept(all.p->bound_sock,(sockaddr*)&client_address,&size);
      if (f < 0)
        THROW("accept: " << strerror(errno));

      // note: breaking cluster parallel online learning by dropping support for id

      all.final_prediction_sink.push_back((size_t) f);
      all.p->input->files.push_back(f);

      if (isbinary(*(all.p->input)))
      { all.p->reader = read_cached_features;
        all.print = binary_print_result;
      }
      else
      { all.p->reader = read_features_string;
        all.print = print_result;
      }
    }
    else
    { for (size_t i = 0; i < input->files.size(); i++)
      { input->reset_file(input->files[i]);
        if (cache_numbits(input, input->files[i]) < numbits)
          THROW("argh, a bug in caching of some sort!");
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
  if (p->jsonp)
    delete p->jsonp;
}

void make_write_cache(vw& all, string &newname, bool quiet)
{ io_buf* output = all.p->output;
  if (output->files.size() != 0)
  { all.trace_message << "Warning: you tried to make two write caches.  Only the first one will be made." << endl;
    return;
  }

  string temp = newname+string(".writing");
  push_many(output->currentname,temp.c_str(),temp.length()+1);

  int f = output->open_file(temp.c_str(), all.stdin_off, io_buf::WRITE);
  if (f == -1)
  { all.trace_message << "can't create cache file !" << endl;
    return;
  }

  size_t v_length = (uint64_t)version.to_string().length()+1;

  output->write_file(f, &v_length, sizeof(v_length));
  output->write_file(f,version.to_string().c_str(),v_length);
  output->write_file(f,"c",1);
  output->write_file(f, &all.num_bits, sizeof(all.num_bits));

  push_many(output->finalname,newname.c_str(),newname.length()+1);
  all.p->write_cache = true;
  if (!quiet)
    all.trace_message << "creating cache_file = " << newname << endl;
}

void parse_cache(vw& all, po::variables_map &vm, string source,
                 bool quiet)
{ vector<string> caches;
  if (vm.count("cache_file"))
    caches = vm["cache_file"].as< vector<string> >();
  if (vm.count("cache"))
    caches.push_back(source+string(".cache"));

  all.p->write_cache = false;

  for (size_t i = 0; i < caches.size(); i++)
  { int f = -1;
    if (!vm.count("kill_cache"))
      try
      { f = all.p->input->open_file(caches[i].c_str(), all.stdin_off, io_buf::READ);
      }
      catch (exception e) { f = -1; }
    if (f == -1)
      make_write_cache(all, caches[i], quiet);
    else
    { uint64_t c = cache_numbits(all.p->input, f);
      if (c < all.num_bits)
      { if (!quiet)
		  all.trace_message << "WARNING: cache file is ignored as it's made with less bit precision than required!" << endl;
        all.p->input->close_file();
        make_write_cache(all, caches[i], quiet);
      }
      else
      { if (!quiet)
          all.trace_message << "using cache_file = " << caches[i].c_str() << endl;
        all.p->reader = read_cached_features;
        if (c == all.num_bits)
          all.p->sorted_cache = true;
        else
          all.p->sorted_cache = false;
        all.p->resettable = true;
      }
    }
  }

  all.parse_mask = ((uint64_t)1 << all.num_bits) - 1;
  if (caches.size() == 0)
  { if (!quiet)
      all.trace_message << "using no cache" << endl;
    all.p->output->space.delete_v();
  }
}

//For macs
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif

void enable_sources(vw& all, bool quiet, size_t passes)
{ all.p->input->current = 0;
  parse_cache(all, all.vm, all.data_filename, quiet);

  if (all.daemon || all.active)
  {
#ifdef _WIN32
    WSAData wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    int lastError = WSAGetLastError();
#endif
    all.p->bound_sock = (int)socket(PF_INET, SOCK_STREAM, 0);
    if (all.p->bound_sock < 0)
    { stringstream msg;
      msg << "socket: " << strerror(errno);
      all.trace_message << msg.str() << endl;
      THROW(msg.str().c_str());
    }

    int on = 1;
    if (setsockopt(all.p->bound_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0)
      all.trace_message << "setsockopt SO_REUSEADDR: " << strerror(errno) << endl;

    // Enable TCP Keep Alive to prevent socket leaks
    int enableTKA = 1;
    if (setsockopt(all.p->bound_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&enableTKA, sizeof(enableTKA)) < 0)
      all.trace_message << "setsockopt SO_KEEPALIVE: " << strerror(errno) << endl;

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    short unsigned int port = 26542;
    if (all.vm.count("port"))
      port = (uint16_t)all.vm["port"].as<size_t>();
    address.sin_port = htons(port);

    // attempt to bind to socket
    if ( ::bind(all.p->bound_sock,(sockaddr*)&address, sizeof(address)) < 0 )
      THROWERRNO("bind");

    // listen on socket
    if (listen(all.p->bound_sock, 1) < 0)
      THROWERRNO("listen");

    // write port file
    if (all.vm.count("port_file"))
    { socklen_t address_size = sizeof(address);
      if (getsockname(all.p->bound_sock, (sockaddr*)&address, &address_size) < 0)
      { all.trace_message << "getsockname: " << strerror(errno) << endl;
      }
      ofstream port_file;
      port_file.open(all.vm["port_file"].as<string>().c_str());
      if (!port_file.is_open())
        THROW("error writing port file: " << all.vm["port_file"].as<string>());

      port_file << ntohs(address.sin_port) << endl;
      port_file.close();
    }

    // background process (if foreground is not set)
    if (!all.vm.count("foreground"))
    {
      if (!all.active && daemon(1,1))
        THROWERRNO("daemon");
    }

    // write pid file
    if (all.vm.count("pid_file"))
    { ofstream pid_file;
      pid_file.open(all.vm["pid_file"].as<string>().c_str());
      if (!pid_file.is_open())
        THROW("error writing pid file");

      pid_file << getpid() << endl;
      pid_file.close();
    }

    if (all.daemon && !all.active)
    {
#ifdef _WIN32
      THROW("not supported on windows");
#else
      fclose(stdin);
      // weights will be shared across processes, accessible to children
      all.weights.share(all.length());

      // learning state to be shared across children
      shared_data* sd = (shared_data *)mmap(0,sizeof(shared_data),
                                            PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
      memcpy(sd, all.sd, sizeof(shared_data));
      free(all.sd);
      all.sd = sd;

      // create children
      size_t num_children = all.num_children;
      v_array<int> children = v_init<int>();
      children.resize(num_children);
      for (size_t i = 0; i < num_children; i++)
      { // fork() returns pid if parent, 0 if child
        // store fork value and run child process if child
        if ((children[i] = fork()) == 0)
        { all.quiet |= (i > 0);
          goto child;
        }
      }

      // install signal handler so we can kill children when killed
      { struct sigaction sa;
        // specifically don't set SA_RESTART in sa.sa_flags, so that
        // waitid will be interrupted by SIGTERM with handler installed
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handle_sigterm;
        sigaction(SIGTERM, &sa, nullptr);
      }

      while (true)
      { // wait for child to change state; if finished, then respawn
        int status;
        pid_t pid = wait(&status);
        if (got_sigterm)
        { for (size_t i = 0; i < num_children; i++)
            kill(children[i], SIGTERM);
          VW::finish(all);
          exit(0);
        }
        if (pid < 0)
          continue;
        for (size_t i = 0; i < num_children; i++)
          if (pid == children[i])
          { if ((children[i]=fork()) == 0)
            { all.quiet |= (i > 0);
              goto child;
            }
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
      all.trace_message << "calling accept" << endl;
    int f = (int)accept(all.p->bound_sock,(sockaddr*)&client_address,&size);
    if (f < 0)
      THROWERRNO("accept");

    all.p->label_sock = f;
    all.print = print_result;

    all.final_prediction_sink.push_back((size_t) f);

    all.p->input->files.push_back(f);
    all.p->max_fd = max(f, all.p->max_fd);
    if (!all.quiet)
      all.trace_message << "reading data from port " << port << endl;

    all.p->max_fd++;
    if(all.active)
      all.p->reader = read_features_string;
    else
    { if (isbinary(*(all.p->input)))
      { all.p->reader = read_cached_features;
        all.print = binary_print_result;
      }
      else
      { all.p->reader = read_features_string;
      }
      all.p->sorted_cache = true;
    }
    all.p->resettable = all.p->write_cache || all.daemon;
  }
  else
  { if (all.p->input->files.size() > 0)
    { if (!quiet)
        all.trace_message << "ignoring text input in favor of cache input" << endl;
    }
    else
    { string temp = all.data_filename;
      if (!quiet)
        all.trace_message << "Reading datafile = " << temp << endl;
      try
      { all.p->input->open_file(temp.c_str(), all.stdin_off, io_buf::READ);
      }
      catch (exception const& ex)
      { // when trying to fix this exception, consider that an empty temp is valid if all.stdin_off is false
        if (temp.size() != 0)
        { all.trace_message << "can't open '" << temp << "', sailing on!" << endl;
        }
        else
        { throw ex;
        }
      }

      if (all.vm.count("json"))
      { 
		  // TODO: change to class with virtual method
		  if (all.audit)
		  {
			  all.p->reader = &read_features_json<true>;
			  all.p->jsonp = new json_parser<true>;
		  }
		  else
		  {
			  all.p->reader = &read_features_json<false>;
			  all.p->jsonp = new json_parser<false>;
		  }
      }
      else
        all.p->reader = read_features_string;

      all.p->resettable = all.p->write_cache;
    }
  }

  if (passes > 1 && !all.p->resettable)
    THROW("need a cache file for multiple passes : try using --cache_file");

  all.p->input->count = all.p->input->files.size();
  if (!quiet && !all.daemon)
    all.trace_message << "num sources = " << all.p->input->files.size() << endl;
}

void set_done(vw& all)
{ all.early_terminate = true;
  mutex_lock(&all.p->examples_lock);
  all.p->done = true;
  mutex_unlock(&all.p->examples_lock);
}

void addgrams(vw& all, size_t ngram, size_t skip_gram, features& fs,
              size_t initial_length, v_array<size_t> &gram_mask, size_t skips)
{ if (ngram == 0 && gram_mask.last() < initial_length)
  { size_t last = initial_length - gram_mask.last();
    for(size_t i = 0; i < last; i++)
    { uint64_t new_index = fs.indicies[i];
      for (size_t n = 1; n < gram_mask.size(); n++)
        new_index = new_index*quadratic_constant + fs.indicies[i+gram_mask[n]];

      fs.push_back(1.,new_index);
      if (fs.space_names.size() > 0)
      { string feature_name(fs.space_names[i].get()->second);
        for (size_t n = 1; n < gram_mask.size(); n++)
        { feature_name += string("^");
          feature_name += string(fs.space_names[i+gram_mask[n]].get()->second);
        }
        fs.space_names.push_back(audit_strings_ptr(new audit_strings(fs.space_names[i].get()->first, feature_name)));
      }
    }
  }
  if (ngram > 0)
  { gram_mask.push_back(gram_mask.last()+1+skips);
    addgrams(all, ngram-1, skip_gram, fs, initial_length, gram_mask, 0);
    gram_mask.pop();
  }
  if (skip_gram > 0 && ngram > 0)
    addgrams(all, ngram, skip_gram-1, fs, initial_length, gram_mask, skips+1);
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
void generateGrams(vw& all, example* &ex)
{ for(namespace_index index : ex->indices)
  { size_t length = ex->feature_space[index].size();
    for (size_t n = 1; n < all.ngram[index]; n++)
    { all.p->gram_mask.erase();
      all.p->gram_mask.push_back((size_t)0);
      addgrams(all, n, all.skips[index], ex->feature_space[index],
               length, all.p->gram_mask, 0);
    }
  }
}

void end_pass_example(vw& all, example* ae)
{ all.p->lp.default_label(&ae->l);
  ae->end_pass = true;
  all.p->in_pass_counter = 0;
}

void feature_limit(vw& all, example* ex)
{ for(namespace_index index : ex->indices)
    if (all.limit[index] < ex->feature_space[index].size())
    { features& fs = ex->feature_space[index];
      fs.sort(all.parse_mask);
      unique_features(fs, all.limit[index]);
    }
}

namespace VW
{
example& get_unused_example(vw* all)
{ parser* p = all->p;
  while (true)
  { mutex_lock(&p->examples_lock);
    if (p->examples[p->begin_parsed_examples % p->ring_size].in_use == false)
    { example& ret = p->examples[p->begin_parsed_examples++ % p->ring_size];
      ret.in_use = true;
      mutex_unlock(&p->examples_lock);
      return ret;
    }
    else
      condition_variable_wait(&p->example_unused, &p->examples_lock);
    mutex_unlock(&p->examples_lock);
  }
}

void setup_examples(vw& all, v_array<example*>& examples)
{ for (example* ae : examples)
    setup_example(all, ae);
}

void setup_example(vw& all, example* ae)
{ if (all.p->sort_features && ae->sorted == false)
    unique_sort_features(all.parse_mask, ae);

  if (all.p->write_cache)
  { all.p->lp.cache_label(&ae->l, *(all.p->output));
    cache_features(*(all.p->output), ae, all.parse_mask);
  }

  ae->partial_prediction = 0.;
  ae->num_features = 0;
  ae->total_sum_feat_sq = 0;
  ae->loss = 0.;

  ae->example_counter = (size_t)(all.p->end_parsed_examples);
  if (!all.p->emptylines_separate_examples)
    all.p->in_pass_counter++;

  ae->test_only = is_test_only(all.p->in_pass_counter, all.holdout_period, all.holdout_after, all.holdout_set_off, all.p->emptylines_separate_examples ? (all.holdout_period-1) : 0);

  if (all.p->emptylines_separate_examples && example_is_newline(*ae))
    all.p->in_pass_counter++;

  ae->weight = all.p->lp.get_weight(&ae->l);

  if (all.ignore_some)
    for (unsigned char* i = ae->indices.begin(); i != ae->indices.end(); i++)
      if (all.ignore[*i])
      { //delete namespace
        ae->feature_space[*i].erase();
        memmove(i, i + 1, (ae->indices.end() - (i + 1))*sizeof(*i));
        ae->indices.end()--;
        i--;
      }

  if(all.ngram_strings.size() > 0)
    generateGrams(all, ae);

  if (all.add_constant)//add constant feature
    VW::add_constant_feature(all,ae);

  if(all.limit_strings.size() > 0)
    feature_limit(all,ae);

  uint64_t multiplier = all.wpp << all.weights.stride_shift();

  if(multiplier != 1) //make room for per-feature information.
    for (features& fs : *ae)
      for (auto& j : fs.indicies)
        j *= multiplier;
  ae->num_features = 0;
  ae->total_sum_feat_sq = 0;
  for (features& fs : *ae)
  { ae->num_features += fs.size();
    ae->total_sum_feat_sq += fs.sum_feat_sq;
  }

  size_t new_features_cnt;
  float new_features_sum_feat_sq;
  INTERACTIONS::eval_count_of_generated_ft(all, *ae, new_features_cnt, new_features_sum_feat_sq);
  ae->num_features += new_features_cnt;
  ae->total_sum_feat_sq += new_features_sum_feat_sq;
}
}

namespace VW
{
example* new_unused_example(vw& all)
{ example* ec = &get_unused_example(&all);
  all.p->lp.default_label(&ec->l);
  all.p->begin_parsed_examples++;
  ec->example_counter = (size_t)all.p->begin_parsed_examples;
  return ec;
}
example* read_example(vw& all, char* example_line)
{ example* ret = &get_unused_example(&all);

  VW::read_line(all, ret, example_line);
  setup_example(all, ret);
  all.p->end_parsed_examples++;

  return ret;
}

example* read_example(vw& all, string example_line) { return read_example(all, (char*)example_line.c_str()); }

void add_constant_feature(vw& vw, example*ec)
{ ec->indices.push_back(constant_namespace);
  ec->feature_space[constant_namespace].push_back(1,constant);
  ec->total_sum_feat_sq++;
  ec->num_features++;
  if (vw.audit || vw.hash_inv) ec->feature_space[constant_namespace].space_names.push_back(audit_strings_ptr(new audit_strings("","Constant")));
}

void add_label(example* ec, float label, float weight, float base)
{ ec->l.simple.label = label;
  ec->l.simple.initial = base;
  ec->weight = weight;
}

example* import_example(vw& all, string label, primitive_feature_space* features, size_t len)
{ example* ret = &get_unused_example(&all);
  all.p->lp.default_label(&ret->l);

  if (label.length() > 0)
    parse_example_label(all, *ret, label);

  for (size_t i = 0; i < len; i++)
  { unsigned char index = features[i].name;
    ret->indices.push_back(index);
    for (size_t j = 0; j < features[i].len; j++)
      ret->feature_space[index].push_back(features[i].fs[j].x, features[i].fs[j].weight_index);
  }

  setup_example(all, ret);
  all.p->end_parsed_examples++;
  return ret;
}

primitive_feature_space* export_example(vw& all, example* ec, size_t& len)
{ len = ec->indices.size();
  primitive_feature_space* fs_ptr = new primitive_feature_space[len];

  int fs_count = 0;
  for (namespace_index i : ec->indices)
  { fs_ptr[fs_count].name = i;
    fs_ptr[fs_count].len = ec->feature_space[i].size();
    fs_ptr[fs_count].fs = new feature[fs_ptr[fs_count].len];

    uint32_t stride_shift = all.weights.stride_shift();
    int f_count = 0;
    for (features::iterator& f : ec->feature_space[i])
      { feature t = {f.value(), f.index()};
        t.weight_index >>= stride_shift;
        fs_ptr[fs_count].fs[f_count] = t;
        f_count++;
      }
    fs_count++;
  }
  return fs_ptr;
}

void releaseFeatureSpace(primitive_feature_space* features, size_t len)
{ for (size_t i = 0; i < len; i++)
    delete features[i].fs;
  delete (features);
}

void parse_example_label(vw& all, example&ec, string label)
{ v_array<substring> words = v_init<substring>();
  char* cstr = (char*)label.c_str();
  substring str = { cstr, cstr+label.length() };
  tokenize(' ', str, words);
  all.p->lp.parse_label(all.p, all.sd, &ec.l, words);
  words.erase();
  words.delete_v();
}

void empty_example(vw& all, example& ec)
{ for (features& fs : ec)
    fs.erase();

  ec.indices.erase();
  ec.tag.erase();
  ec.sorted = false;
  ec.end_pass = false;
}

void finish_example(vw& all, example* ec)
{ // only return examples to the pool that are from the pool and not externally allocated
  if (!is_ring_example(all, ec))
    return;

  mutex_lock(&all.p->output_lock);
  all.p->local_example_number++;
  condition_variable_signal(&all.p->output_done);
  mutex_unlock(&all.p->output_lock);

  empty_example(all, *ec);

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
{ v_array<example*> examples = v_init<example*>();
  vw* all = (vw*)in;
  size_t example_number = 0;  // for variable-size batch learning algorithms

  try {
    size_t examples_available;
    while(!all->p->done)
    { examples.push_back(&VW::get_unused_example(all)); // need at least 1 example
      if (!all->do_reset_source && example_number != all->pass_length && all->max_examples > example_number
          && all->p->reader(all, examples) > 0)
      { VW::setup_examples(*all, examples);
        example_number+=examples.size();
        examples_available=examples.size();
      }
      else
      { reset_source(*all, all->num_bits);
        all->do_reset_source = false;
        all->passes_complete++;
    
        end_pass_example(*all, examples[0]);
        if (all->passes_complete == all->numpasses && example_number == all->pass_length)
        { all->passes_complete = 0;
          all->pass_length = all->pass_length*2+1;
        }
        if (all->passes_complete >= all->numpasses && all->max_examples >= example_number)
        { mutex_lock(&all->p->examples_lock);
          all->p->done = true;
          mutex_unlock(&all->p->examples_lock);
        }
        example_number = 0;
        examples_available=1;
      }
      mutex_lock(&all->p->examples_lock);
      all->p->end_parsed_examples+=examples_available;
      condition_variable_signal_all(&all->p->example_available);
      mutex_unlock(&all->p->examples_lock);
      examples.erase();
    }
  }
  catch (VW::vw_exception& e)
  { cerr << "vw example #" << example_number << "(" << e.Filename() << ":" << e.LineNumber() << "): " << e.what() << endl;
  }
  catch (exception& e)
  { cerr << "vw: example #" << example_number << e.what() << endl;
  }

  if (!all->p->done)
  { mutex_lock(&all->p->examples_lock);
	all->p->done = true;
	mutex_unlock(&all->p->examples_lock);
  }

  examples.delete_v();
  return 0L;
}

namespace VW
{
example* get_example(parser* p)
{ mutex_lock(&p->examples_lock);
  if (p->end_parsed_examples != p->used_index)
  { size_t ring_index = p->used_index++ % p->ring_size;
    if (!(p->examples+ring_index)->in_use)
      cout << "error: example should be in_use " << p->used_index << " " << p->end_parsed_examples << " " << ring_index << endl;
    assert((p->examples+ring_index)->in_use);
    mutex_unlock(&p->examples_lock);

    return p->examples + ring_index;
  }
  else
  { if (!p->done)
    { condition_variable_wait(&p->example_available, &p->examples_lock);
      mutex_unlock(&p->examples_lock);
      return get_example(p);
    }
    else
    { mutex_unlock(&p->examples_lock);
      return nullptr;
    }
  }
}

float get_topic_prediction(example* ec, size_t i)
{ return ec->pred.scalars[i]; }

float get_label(example* ec)
{ return ec->l.simple.label; }

float get_importance(example* ec)
{ return ec->weight; }

float get_initial(example* ec)
{ return ec->l.simple.initial; }

float get_prediction(example* ec)
{ return ec->pred.scalar; }

float get_cost_sensitive_prediction(example* ec)
{ return (float)ec->pred.multiclass; }

v_array<float>& get_cost_sensitive_prediction_confidence_scores(example* ec)
{ return ec->pred.scalars;
}

uint32_t* get_multilabel_predictions(example* ec, size_t& len)
{ MULTILABEL::labels labels = ec->pred.multilabels;
  len = labels.label_v.size();
  return labels.label_v.begin();
}

size_t get_tag_length(example* ec)
{ return ec->tag.size();
}

const char* get_tag(example* ec)
{ return ec->tag.begin();
}

size_t get_feature_number(example* ec)
{ return ec->num_features;
}

float get_confidence(example* ec)
{ return ec->confidence;
}
}

void initialize_examples(vw& all)
{ all.p->used_index = 0;
  all.p->begin_parsed_examples = 0;
  all.p->end_parsed_examples = 0;
  all.p->done = false;

  all.p->examples = calloc_or_throw<example>(all.p->ring_size);

  for (size_t i = 0; i < all.p->ring_size; i++)
  { memset(&all.p->examples[i].l, 0, sizeof(polylabel));
    all.p->examples[i].in_use = false;
  }
}

void adjust_used_index(vw& all)
{ all.p->used_index=all.p->begin_parsed_examples;
}

void initialize_parser_datastructures(vw& all)
{ initialize_examples(all);
  initialize_mutex(&all.p->examples_lock);
  initialize_condition_variable(&all.p->example_available);
  initialize_condition_variable(&all.p->example_unused);
  initialize_mutex(&all.p->output_lock);
  initialize_condition_variable(&all.p->output_done);
}

namespace VW
{
void start_parser(vw& all)
{
#ifndef _WIN32
  pthread_create(&all.parse_thread, nullptr, main_parse_loop, &all);
#else
  all.parse_thread = ::CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(main_parse_loop), &all, 0L, nullptr);
#endif
}
}
void free_parser(vw& all)
{ all.p->channels.delete_v();
  all.p->words.delete_v();
  all.p->name.delete_v();

  if(all.ngram_strings.size() > 0)
    all.p->gram_mask.delete_v();

  if (all.p->examples != nullptr)
  { for (size_t i = 0; i < all.p->ring_size; i++)
      VW::dealloc_example(all.p->lp.delete_label, all.p->examples[i], all.delete_prediction);

    free(all.p->examples);
  }

  io_buf* output = all.p->output;
  if (output != nullptr)
  { output->finalname.delete_v();
    output->currentname.delete_v();
  }

  all.p->counts.delete_v();
}

void release_parser_datastructures(vw& all)
{ delete_mutex(&all.p->examples_lock);
  delete_mutex(&all.p->output_lock);
}

namespace VW
{
void end_parser(vw& all)
{
#ifndef _WIN32
  pthread_join(all.parse_thread, nullptr);
#else
  ::WaitForSingleObject(all.parse_thread, INFINITE);
  ::CloseHandle(all.parse_thread);
#endif
  release_parser_datastructures(all);
}

bool is_ring_example(vw& all, example* ae)
{ return all.p->examples <= ae && ae < all.p->examples + all.p->ring_size;
}
}
