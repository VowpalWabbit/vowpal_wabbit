#include <pthread.h>
#include <stdio.h>
#include <float.h>
#include <iostream>
#include <math.h>
#include <assert.h>

#include "global_data.h"
#include "simple_label.h"
#include "parser.h"
#include "gd.h"

using namespace std;

struct global_prediction {
  float p;
  float weight;
};

int really_read(int sock, void* in, size_t count)
{
  char* buf = (char*)in;
  size_t done = 0;
  int r = 0;
  while (done < count)
    {
      if ((r = read(sock,buf,count-done)) == 0)
	return 0;
      else
	if (r < 0)
	  {
	    cerr << "argh! bad read! on message from " << sock << endl;
	    perror(NULL);
	    exit(0);
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
  int count = really_read(sock, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
  
  assert(count == sizeof(p));
}

void send_prediction(int sock, global_prediction p)
{
  if (write(sock, &p, sizeof(p)) < (int)sizeof(p))
    {
      cerr << "argh! bad global write! " << sock << endl;
      perror(NULL);
      exit(0);
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

void print_tag(int f, v_array<char> tag)
{
  char temp[30];
  ssize_t t;
  if (tag.begin != tag.end){
    temp[0] = ' ';
    t = write(f, temp, 1);
    if (t != 1)
      cerr << "write error" << endl;
    t = write(f, tag.begin, sizeof(char)*tag.index());
    if (t != (ssize_t) (sizeof(char)*tag.index()))
      cerr << "write error" << endl;
  }  
}

void print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      int num = sprintf(temp, "%f", res);
      ssize_t t;
      t = write(f, temp, num);
      if (t != num) 
	cerr << "write error" << endl;
      print_tag(f, tag);
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	cerr << "write error" << endl;
    }
}

void print_raw_text(int f, string s, v_array<char> tag)
{
  if (f < 0)
    return;

  ssize_t t;
  int num = s.length();
  t = write(f, s.c_str(), num);
  if (t != num) 
    cerr << "write error" << endl;
  print_tag(f, tag);
  char temp = '\n';
  t = write(f, &temp, 1);     
  if (t != 1)
    cerr << "write error" << endl;
}

void active_print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      int num = sprintf(temp, "%f", res);
      ssize_t t;
      t = write(f, temp, num);
      if (t != num) 
	cerr << "write error" << endl;
      print_tag(f, tag);
      if(weight >= 0)
	{
	  num = sprintf(temp, " %f", weight);
	  t = write(f, temp, num);
	  if (t != num)
	    cerr << "write error" << endl;
	}
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	cerr << "write error" << endl;
    }
}

void print_lda_result(vw& all, int f, float* res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      ssize_t t;
      int num;
      for (size_t k = 0; k < all.lda; k++)
	{
	  num = sprintf(temp, "%f ", res[k]);
	  t = write(f, temp, num);
	  if (t != num)
	    cerr << "write error" << endl;
	}
      print_tag(f, tag);
      temp[0] = '\n';
      t = write(f, temp, 1);
      if (t != 1)
	cerr << "write error" << endl;
    }
}

void set_mm(shared_data* sd, double label)
{
  sd->min_label = min(sd->min_label, label);
  if (label != FLT_MAX)
    sd->max_label = max(sd->max_label, label);
}

void noop_mm(shared_data* sd, double label)
{}

vw::vw()
{
  sd = (shared_data *) malloc(sizeof(shared_data));
  sd->queries = 0;
  sd->example_number = 0;
  sd->weighted_examples = 0.;
  sd->old_weighted_examples = 0.;
  sd->weighted_labels = 0.;
  sd->total_features = 0;
  sd->sum_loss = 0.0;
  sd->sum_loss_since_last_dump = 0.0;
  sd->dump_interval = exp(1.);
  sd->gravity = 0.;
  sd->contraction = 1.;
  sd->min_label = 0.;
  sd->max_label = 1.;
  sd->t = 1.;
  sd->binary_label = false;
  sd->k = 0;
  
  p = new_parser();
  p->lp = (label_parser*)malloc(sizeof(label_parser));
  *(p->lp) = simple_label;

  reg_mode = 0;

  bfgs = false;
  hessian_on = false;
  sequence = false;
  stride = 1;
  num_bits = 18;
  default_bits = true;
  daemon = false;
  num_children = 10;
  lda_alpha = 0.1;
  lda_rho = 0.1;
  lda_D = 10000.;
  minibatch = 1;
  span_server = "";
  m = 15; 

  driver = drive_gd;
  learn = learn_gd;
  finish = finish_gd;
  set_minmax = set_mm;

  power_t = 0.5;
  eta = 10;
  numpasses = 1;
  rel_threshold = 0.001;
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

  nonormalize = false;
  l1_lambda = 0.0;
  l2_lambda = 0.0;

  eta_decay_rate = 1.0;
  initial_weight = 0.0;

  unique_id = 0;
  total = 1;
  node = 0;

  ngram = 0;
  skips = 0;

  adaptive = false;
  add_constant = true;
  exact_adaptive_norm = false;
  audit = false;
  active = false;
  active_c0 = 8.;
  active_simulation = false;
  reg.weight_vectors = NULL;
  reg.regularizers = NULL;
  pass_length = (size_t)-1;
  passes_complete = 0;

  save_per_pass = false;
}
