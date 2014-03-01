#include <float.h>
#include <math.h>
#include <stdio.h>

#include "simple_label.h"
#include "cache.h"
#include "rand48.h"
#include "vw.h"
#include "accumulate.h"

using namespace std;

char* bufread_simple_label(shared_data* sd, label_data* ld, char* c)
{
  ld->label = *(float *)c;
  c += sizeof(ld->label);
  if (sd->binary_label && fabs(ld->label) != 1.f && ld->label != FLT_MAX)
    cout << "You are using a label not -1 or 1 with a loss function expecting that!" << endl;
  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  ld->initial = *(float *)c;
  c += sizeof(ld->initial);
  return c;
}

size_t read_cached_simple_label(shared_data* sd, void* v, io_buf& cache)
{
  label_data* ld = (label_data*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight)+sizeof(ld->initial);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_simple_label(sd, ld,c);

  return total;
}

float get_weight(void* v)
{
  label_data* ld = (label_data*) v;
  return ld->weight;
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

void parse_simple_label(parser* p, shared_data* sd, void* v, v_array<substring>& words)
{
  label_data* ld = (label_data*)v;

  switch(words.size()) {
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
    cerr << "words.size() = " << words.size() << endl;
  }
  if (words.size() > 0 && sd->binary_label && fabs(ld->label) != 1.f)
    cout << "You are using a label not -1 or 1 with a loss function expecting that!" << endl;
}

float get_active_coin_bias(float k, float l, float g, float c0)
{
  float b,sb,rs,sl;
  b=(float)(c0*(log(k+1.)+0.0001)/(k+0.0001));
  sb=sqrt(b);
  if (l > 1.0) { l = 1.0; } else if (l < 0.0) { l = 0.0; } //loss should be in [0,1]
  sl=sqrt(l)+sqrt(l+g);
  if (g<=sb*sl+b)
    return 1;
  rs = (sl+sqrt(sl*sl+4*g))/(2*g);
  return b*rs*rs;
}

float query_decision(vw& all, example& ec, float k)
{
  float bias, avg_loss, weighted_queries;
  if (k<=1.)
    bias=1.;
  else{
    weighted_queries = (float)(all.initial_t + all.sd->weighted_examples - all.sd->weighted_unlabeled_examples);
    avg_loss = (float)(all.sd->sum_loss/k + sqrt((1.+0.5*log(k))/(weighted_queries+0.0001)));
    bias = get_active_coin_bias(k, avg_loss, ec.revert_weight/k, all.active_c0);
  }
  if(frand48()<bias)
    return 1.f/bias;
  else
    return -1.;
}

void print_update(vw& all, example& ec)
{
  if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
      label_data* ld = (label_data*) ec.ld;
      char label_buf[32];
      if (ld->label == FLT_MAX)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf,"%8.4f",ld->label);
      
      if(!all.holdout_set_off && all.current_pass >= 1){
        if(all.sd->holdout_sum_loss == 0. && all.sd->weighted_holdout_examples == 0.)
          fprintf(stderr, " unknown   ");
        else
	  fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss/all.sd->weighted_holdout_examples);

        if(all.sd->holdout_sum_loss_since_last_dump == 0. && all.sd->weighted_holdout_examples_since_last_dump == 0.)
          fprintf(stderr, " unknown   ");
        else
	  fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss_since_last_dump/all.sd->weighted_holdout_examples_since_last_dump);
        
        fprintf(stderr, "%10ld %11.1f %s %8.4f %8lu h\n",
	      (long int)all.sd->example_number,
	      all.sd->weighted_examples,
	      label_buf,
	      ec.final_prediction,
	      (long unsigned int)ec.num_features);

        all.sd->weighted_holdout_examples_since_last_dump = 0.;
        all.sd->holdout_sum_loss_since_last_dump = 0.0;
      }
      else
        fprintf(stderr, "%-10.6f %-10.6f %10ld %11.1f %s %8.4f %8lu\n",
	      all.sd->sum_loss/all.sd->weighted_examples,
	      all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
	      (long int)all.sd->example_number,
	      all.sd->weighted_examples,
	      label_buf,
	      ec.final_prediction,
	      (long unsigned int)ec.num_features);
     
      all.sd->sum_loss_since_last_dump = 0.0;
      all.sd->old_weighted_examples = all.sd->weighted_examples;
      VW::update_dump_interval(all);
	  fflush(stderr);
    }
}

void output_and_account_example(vw& all, example& ec)
{
  label_data* ld = (label_data*)ec.ld;

  if(ec.test_only)
  {
    all.sd->weighted_holdout_examples += ec.global_weight;//test weight seen
    all.sd->weighted_holdout_examples_since_last_dump += ec.global_weight;
    all.sd->weighted_holdout_examples_since_last_pass += ec.global_weight;
    all.sd->holdout_sum_loss += ec.loss;
    all.sd->holdout_sum_loss_since_last_dump += ec.loss;
    all.sd->holdout_sum_loss_since_last_pass += ec.loss;//since last pass
  }
  else
  {
    if (ld->label != FLT_MAX)
      all.sd->weighted_labels += ld->label * ld->weight;
    all.sd->weighted_examples += ld->weight;
    all.sd->sum_loss += ec.loss;
    all.sd->sum_loss_since_last_dump += ec.loss;
    all.sd->total_features += ec.num_features;
    all.sd->example_number++;
  }
  all.print(all.raw_prediction, ec.partial_prediction, -1, ec.tag);

  float ai=-1; 
  if(all.active && ld->label == FLT_MAX)
    ai=query_decision(all, ec, (float)all.sd->weighted_unlabeled_examples);
  all.sd->weighted_unlabeled_examples += ld->label == FLT_MAX ? ld->weight : 0;
  
  for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
    {
      int f = (int)all.final_prediction_sink[i];
      if(all.active && all.lda == 0)
	active_print_result(f, ec.final_prediction, ai, ec.tag);
      else if (all.lda > 0)
	print_lda_result(all, f,ec.topic_predictions.begin,0.,ec.tag);
      else
	all.print(f, ec.final_prediction, 0, ec.tag);
    }

  print_update(all, ec);
}

void return_simple_example(vw& all, void*, example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all,&ec);
}

bool summarize_holdout_set(vw& all, size_t& no_win_counter)
{
  float thisLoss = (all.sd->weighted_holdout_examples_since_last_pass > 0) ? (float)(all.sd->holdout_sum_loss_since_last_pass / all.sd->weighted_holdout_examples_since_last_pass) : FLT_MAX;

  if (all.span_server != "")
    thisLoss = accumulate_scalar(all, all.span_server, thisLoss);

  all.sd->weighted_holdout_examples_since_last_pass = 0;
  all.sd->holdout_sum_loss_since_last_pass = 0;

  if (thisLoss < all.sd->holdout_best_loss) {
    all.sd->holdout_best_loss = thisLoss;
    all.sd->holdout_best_pass = all.current_pass;
    no_win_counter = 0;
    return true;
  }

  no_win_counter++;
  return false;          
} 
