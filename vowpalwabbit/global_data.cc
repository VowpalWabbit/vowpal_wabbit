/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <stdio.h>
#include <float.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <assert.h>

#include "global_data.h"
#include "simple_label.h"
#include "parser.h"
#include "gd.h"
#include "memory.h"

using namespace std;

struct global_prediction {
  float p;
  float weight;
};

size_t really_read(int sock, void* in, size_t count)
{
  char* buf = (char*)in;
  size_t done = 0;
  int r = 0;
  while (done < count)
    {
      if ((r =
#ifdef _WIN32
		  recv(sock,buf,(unsigned int)(count-done),0)
#else
		  read(sock,buf,(unsigned int)(count-done))
#endif
		  ) == 0)
	return 0;
      else
	if (r < 0)
	  {
	    cerr << "read(" << sock << "," << count << "-" << done << "): " << strerror(errno) << endl;
	    throw exception();
	  }
	else
	  {
	    done += r;
	    buf += r;
	  }
    }
  return done;
}

void get_prediction(int sock, float& res, float& weight)
{
  global_prediction p;
  really_read(sock, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
}

void send_prediction(int sock, global_prediction p)
{
  if (
#ifdef _WIN32
	  send(sock, reinterpret_cast<const char*>(&p), sizeof(p), 0)
#else
	  write(sock, &p, sizeof(p))
#endif
	  < (int)sizeof(p))
    {
      cerr << "send_prediction write(" << sock << "): " << strerror(errno) << endl;
      throw exception();
    }
}

void binary_print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      global_prediction ps = {res, weight};
      send_prediction(f, ps);
    }
}

int print_tag(std::stringstream& ss, v_array<char> tag)
{
  if (tag.begin != tag.end){
    ss << ' ';
    ss.write(tag.begin, sizeof(char)*tag.size());
  }
  return tag.begin != tag.end;
}

void print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      sprintf(temp, "%f", res);
      std::stringstream ss;
      ss << temp;
      print_tag(ss, tag);
      ss << '\n';
      ssize_t len = ss.str().size();
      ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
      if (t != len)
        {
          cerr << "write error" << endl;
        }
    }
}

void print_raw_text(int f, string s, v_array<char> tag)
{
  if (f < 0)
    return;

  std::stringstream ss;
  ss << s;
  print_tag (ss, tag);
  ss << '\n';
  ssize_t len = ss.str().size();
  ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
  if (t != len)
    {
      cerr << "write error" << endl;
    }
}

void print_lda_result(vw& all, int f, float* res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      std::stringstream ss;
      char temp[30];
      for (size_t k = 0; k < all.lda; k++)
	{
	  sprintf(temp, "%f ", res[k]);
          ss << temp;
	}
      print_tag(ss, tag);
      ss << '\n';
      ssize_t len = ss.str().size();
      ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);

      if (t != len)
	cerr << "write error" << endl;
    }
}

void set_mm(shared_data* sd, float label)
{
  sd->min_label = min(sd->min_label, label);
  if (label != FLT_MAX)
    sd->max_label = max(sd->max_label, label);
}

void noop_mm(shared_data* sd, float label)
{}

void vw::learn(example* ec)
{
  this->l->learn(*ec);
}

void compile_gram(vector<string> grams, uint32_t* dest, char* descriptor, bool quiet)
{
  for (size_t i = 0; i < grams.size(); i++)
    {
      string ngram = grams[i];
      if ( isdigit(ngram[0]) )
	{
	  int n = atoi(ngram.c_str());
	  if (!quiet)
	    cerr << "Generating " << n << "-" << descriptor << " for all namespaces." << endl;
	  for (size_t j = 0; j < 256; j++)
	    dest[j] = n;
	}
      else if ( ngram.size() == 1)
	cout << "You must specify the namespace index before the n" << endl;
      else {
	int n = atoi(ngram.c_str()+1);
	dest[(uint32_t)ngram[0]] = n;
	if (!quiet)
	  cerr << "Generating " << n << "-" << descriptor << " for " << ngram[0] << " namespaces." << endl;
      }
    }
}

void compile_limits(vector<string> limits, uint32_t* dest, bool quiet)
{
  for (size_t i = 0; i < limits.size(); i++)
    {
      string limit = limits[i];
      if ( isdigit(limit[0]) )
	{
	  int n = atoi(limit.c_str());
	  if (!quiet)
	    cerr << "limiting to " << n << "features for each namespace." << endl;
	  for (size_t j = 0; j < 256; j++)
	    dest[j] = n;
	}
      else if ( limit.size() == 1)
	cout << "You must specify the namespace index before the n" << endl;
      else {
	int n = atoi(limit.c_str()+1);
	dest[(uint32_t)limit[0]] = n;
	if (!quiet)
	  cerr << "limiting to " << n << " for namespaces " << limit[0] << endl;
      }
    }
}

po::variables_map add_options(vw& all, po::options_description& opts)
{
  all.opts.add(opts);
  po::variables_map new_vm;

  //parse local opts once for notifications.
  po::parsed_options parsed = po::command_line_parser(all.args).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(opts).allow_unregistered().run();
  po::store(parsed, new_vm);
  po::notify(new_vm); 
  //parse all opts for a complete variable map.
  parsed = po::command_line_parser(all.args).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(all.opts).allow_unregistered().run();
  po::store(parsed, new_vm);
  return new_vm;
}

vw::vw()
{
  sd = (shared_data *) calloc_or_die(1, sizeof(shared_data));
  sd->dump_interval = 1.;   // next update progress dump
  sd->contraction = 1.;
  sd->max_label = 1.;
  sd->min_label = 0.;

  p = new_parser();
  p->emptylines_separate_examples = false;
  p->lp = simple_label;

  reg_mode = 0;
  current_pass = 0;

  data_filename = "";

  bfgs = false;
  hessian_on = false;
  active = false;
  reg.stride_shift = 0;
  num_bits = 18;
  default_bits = true;
  daemon = false;
  num_children = 10;
  lda_alpha = 0.1f;
  lda_rho = 0.1f;
  lda_D = 10000.;
  lda_epsilon = 0.001f;
  minibatch = 1;
  span_server = "";
  m = 15;
  save_resume = false;

  random_positive_weights = false;

  set_minmax = set_mm;

  power_t = 0.5;
  eta = 0.5; //default learning rate for normalized adaptive updates, this is switched to 10 by default for the other updates (see parse_args.cc)
  numpasses = 1;
  rel_threshold = 0.001f;
  rank = 0;

  final_prediction_sink.begin = final_prediction_sink.end=final_prediction_sink.end_array = NULL;
  raw_prediction = -1;
  print = print_result;
  print_text = print_raw_text;
  lda = 0;
  random_weights = false;
  per_feature_regularizer_input = "";
  per_feature_regularizer_output = "";
  per_feature_regularizer_text = "";

  file_options = "";

  #ifdef _WIN32
  stdout_fileno = _fileno(stdout);
  #else
  stdout_fileno = fileno(stdout);
  #endif

  searchstr = NULL;

  nonormalize = false;
  l1_lambda = 0.0;
  l2_lambda = 0.0;

  eta_decay_rate = 1.0;
  initial_weight = 0.0;
  initial_constant = 0.0;

  unique_id = 0;
  total = 1;
  node = 0;

  for (size_t i = 0; i < 256; i++)
    {
      ngram[i] = 0;
      skips[i] = 0;
      limit[i] = INT_MAX;
      affix_features[i] = 0;
      spelling_features[i] = 0;
    }

  //by default use invariant normalized adaptive updates
  adaptive = true;
  normalized_updates = true;
  invariant_updates = true;

  normalized_idx = 2;

  add_constant = true;
  audit = false;
  reg.weight_vector = NULL;
  pass_length = (size_t)-1;
  passes_complete = 0;

  save_per_pass = false;

  stdin_off = false;
  do_reset_source = false;
  holdout_set_off = true;
  holdout_period = 10;
  holdout_after = 0;
  check_holdout_every_n_passes = 1;
  early_terminate = false;

  max_examples = (size_t)-1;

  hash_inv = false;
  print_invert = false;

  // Set by the '--progress <arg>' option and affect sd->dump_interval
  progress_add = false;   // default is multiplicative progress dumps
  progress_arg = 2.0;     // next update progress dump multiplier
}

