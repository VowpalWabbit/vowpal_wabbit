#include <float.h>
#include <math.h>
#include <stdio.h>

#include "simple_label.h"
#include "cache.h"

using namespace std;

char* bufread_simple_label(label_data* ld, char* c)
{
  ld->label = *(float *)c;
  c += sizeof(ld->label);
  if (global.binary_label && fabs(ld->label) != 1.f && ld->label != FLT_MAX)
    cout << "You are using a label not -1 or 1 with a loss function expecting that!" << endl;
  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  ld->initial = *(float *)c;
  c += sizeof(ld->initial);
  return c;
}

size_t read_cached_simple_label(void* v, io_buf& cache)
{
  label_data* ld = (label_data*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight)+sizeof(ld->initial);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_simple_label(ld,c);

  return total;
}

float get_weight(void* v)
{
  label_data* ld = (label_data*) v;
  return ld->weight;
}

float get_initial(void* v)
{
  label_data* ld = (label_data*) v;
  return ld->initial;
}

char* bufcache_simple_label(label_data* ld, char* c)
{
  *(float *)c = ld->label;
  c += sizeof(ld->label);
  *(float *)c = ld->weight;
  c += sizeof(ld->weight);
  *(float *)c = ld->initial;
  c += sizeof(ld->initial);
  return c;
}

void cache_simple_label(void* v, io_buf& cache)
{
  char *c;
  label_data* ld = (label_data*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight)+sizeof(ld->initial));
  c = bufcache_simple_label(ld,c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  ld->label = FLT_MAX;
  ld->weight = 1.;
  ld->initial = 0.;
}

void delete_simple_label(void* v)
{
}

void parse_simple_label(void* v, v_array<substring>& words)
{
  label_data* ld = (label_data*)v;

  switch(words.index()) {
  case 0:
    break;
  case 1:
    ld->label = float_of_substring(words[0]);
    break;
  case 2:
    ld->label = float_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    break;
  case 3:
    ld->label = float_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    ld->initial = float_of_substring(words[2]);
    break;
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
  if (words.index() > 0 && global.binary_label && fabs(ld->label) != 1.f)
    cout << "You are using a label not -1 or 1 with a loss function expecting that!" << endl;
}

float get_active_coin_bias(float k, float l, float g, float c0)
{
  float b,sb,rs,sl;
  b=c0*(log(k+1.)+0.0001)/(k+0.0001);
  sb=sqrt(b);
  if (l > 1.0) { l = 1.0; } else if (l < 0.0) { l = 0.0; } //loss should be in [0,1]
  sl=sqrt(l)+sqrt(l+g);
  if (g<=sb*sl+b)
    return 1;
  rs = (sl+sqrt(sl*sl+4*g))/(2*g);
  return b*rs*rs;
}

float query_decision(example* ec, float k)
{
  float bias, avg_loss, weighted_queries;
  if (k<=1.)
    bias=1.;
  else{
    weighted_queries = global.initial_t + global.sd->weighted_examples - global.sd->weighted_unlabeled_examples;
    avg_loss = global.sd->sum_loss/k + sqrt((1.+0.5*log(k))/(weighted_queries+0.0001));
    bias = get_active_coin_bias(k, avg_loss, ec->revert_weight/k, global.active_c0);
  }
  if(drand48()<bias)
    return 1./bias;
  else
    return -1.;
}

void print_update(example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      label_data* ld = (label_data*) ec->ld;
      char label_buf[32];
      if (ld->label == FLT_MAX)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf,"%8.4f",ld->label);

      fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8.4f %8lu\n",
	      global.sd->sum_loss/global.sd->weighted_examples,
	      global.sd->sum_loss_since_last_dump / (global.sd->weighted_examples - global.sd->old_weighted_examples),
	      (long int)global.sd->example_number,
	      global.sd->weighted_examples,
	      label_buf,
	      ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sd->sum_loss_since_last_dump = 0.0;
      global.sd->old_weighted_examples = global.sd->weighted_examples;
      global.sd->dump_interval *= 2;
    }
}

void output_and_account_example(example* ec)
{
  label_data* ld = (label_data*)ec->ld;
  global.sd->weighted_examples += ld->weight;
  global.sd->weighted_labels += ld->label == FLT_MAX ? 0 : ld->label * ld->weight;
  global.sd->total_features += ec->num_features;
  global.sd->sum_loss += ec->loss;
  global.sd->sum_loss_since_last_dump += ec->loss;
  
  global.print(global.raw_prediction, ec->partial_prediction, -1, ec->tag);

  float ai=-1; 
  if(global.active && ld->label == FLT_MAX)
    ai=query_decision(ec, global.sd->weighted_unlabeled_examples);
  global.sd->weighted_unlabeled_examples += ld->label == FLT_MAX ? ld->weight : 0;
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i];
      if(global.active)
	global.print(f, ec->final_prediction, ai, ec->tag);
      else if (global.lda > 0)
	print_lda_result(f,ec->topic_predictions.begin,0.,ec->tag);
      else
	global.print(f, ec->final_prediction, 0, ec->tag);
    }

  global.sd->example_number++;

  print_update(ec);
}

void return_simple_example(example* ec)
{
  output_and_account_example(ec);
  free_example(ec);
}

example* get_simple_example()
{
  return get_example();
}
