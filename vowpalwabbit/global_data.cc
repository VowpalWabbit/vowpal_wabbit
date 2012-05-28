#include <pthread.h>
#include <stdio.h>
#include <float.h>
#include <iostream>
#include <math.h>
#include "gd.h"
#include "global_data.h"
#include "config.h"

using namespace std;

vw global;
string version = PACKAGE_VERSION;

pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t output_done = PTHREAD_COND_INITIALIZER;

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
      if (tag.begin != tag.end){
	temp[0] = ' ';
	t = write(f, temp, 1);
	if (t != 1)
	  cerr << "write error" << endl;
	t = write(f, tag.begin, sizeof(char)*tag.index());
	if (t != (ssize_t) (sizeof(char)*tag.index()))
	  cerr << "write error" << endl;
      }
      if(global.active && weight >= 0)
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

void print_lda_result(int f, float* res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      ssize_t t;
      int num;
      for (size_t k = 0; k < global.lda; k++)
	{
	  num = sprintf(temp, "%f ", res[k]);
	  t = write(f, temp, num);
	  if (t != num)
	    cerr << "write error" << endl;
	}
      if (tag.begin != tag.end){
	temp[0] = ' ';
	t = write(f, temp, 1);
	if (t != 1)
	  cerr << "write error" << endl;
	t = write(f, tag.begin, sizeof(char)*tag.index());
	if (t != (ssize_t) (sizeof(char)*tag.index()))
	  cerr << "write error" << endl;
      }
      if(global.active && weight >= 0)
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

void set_mm(double label)
{
  global.sd->min_label = min(global.sd->min_label, label);
  if (label != FLT_MAX)
    global.sd->max_label = max(global.sd->max_label, label);
}

void noop_mm(double label)
{}

void (*set_minmax)(double label) = set_mm;

vw::vw()
{
  global.sd = (shared_data *) malloc(sizeof(shared_data));
  global.sd->queries = 0;
  global.sd->example_number = 0;
  global.sd->weighted_examples = 0.;
  global.sd->old_weighted_examples = 0.;
  global.sd->weighted_labels = 0.;
  global.sd->total_features = 0;
  global.sd->sum_loss = 0.0;
  global.sd->sum_loss_since_last_dump = 0.0;
  global.sd->dump_interval = exp(1.);
  global.sd->gravity = 0.;
  global.sd->contraction = 1.;
  global.sd->min_label = 0.;
  global.sd->max_label = 1.;
  global.sd->t = 1.;
  global.lp = (label_parser*)malloc(sizeof(label_parser));
  *(global.lp) = simple_label;
  global.reg_mode = 0;
  global.local_example_number = 0;
  global.bfgs = false;
  global.hessian_on = false;
  global.sequence = false;
  global.stride = 1;
  global.num_bits = 18;
  global.default_bits = true;
  global.daemon = false;
  global.num_children = 10;
  global.lda_alpha = 0.1;
  global.lda_rho = 0.1;
  global.lda_D = 10000.;
  global.minibatch = 1;
  global.span_server = "";
  global.m = 15; 

  global.driver = drive_gd;
  global.k = 0;
  
  global.power_t = 0.5;
  global.eta = 10;
  global.numpasses = 1;
  global.rel_threshold = 0.001;
  global.rank = 0;

  global.final_prediction_sink.begin = global.final_prediction_sink.end=global.final_prediction_sink.end_array = NULL;
  global.raw_prediction = -1;
  global.print = print_result;
  global.lda = 0;
  global.random_weights = false;
  global.per_feature_regularizer_input = "";
  global.per_feature_regularizer_output = "";
  global.per_feature_regularizer_text = "";
  global.ring_size = 1 << 8;
  global.nonormalize = false;
  global.binary_label = false;
  global.l1_lambda = 0.0;
  global.l2_lambda = 0.0;

  global.eta_decay_rate = 1.0;
  global.initial_weight = 0.0;

  global.unique_id = 0;
  global.total = 1;
  global.node = 0;
  
  global.adaptive = false;
  global.add_constant = true;
  global.exact_adaptive_norm = false;
  global.audit = false;
  global.active = false;
  global.active_c0 = 8.;
  global.active_simulation = false;
  global.reg.weight_vectors = NULL;
  global.reg.regularizers = NULL;
  global.pass_length = (size_t)-1;

  global.save_per_pass = false;
}
