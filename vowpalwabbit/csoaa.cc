#include <float.h>
#include <math.h>
#include <stdio.h>

#include "csoaa.h"
#include "simple_label.h"
#include "cache.h"
#include "oaa.h"

using namespace std;

namespace CSOAA {

bool is_test_label(label* ld)
{
  if (ld->costs.index() == 0)
    return true;
  float cost_0 = ld->costs[0].x;
  for (size_t i=1; i<ld->costs.index(); i++)
    if (cost_0 != ld->costs[i].x)
      return false;
  return true;
}
  
char* bufread_label(label* ld, char* c, io_buf& cache)
{
  return 0;
}

size_t read_cached_label(void* v, io_buf& cache)
{
  return 0;
}

float weight(void* v)
{
  return 1.;
}

float initial(void* v)
{
  return 0.;
}

char* bufcache_label(label* ld, char* c)
{
  return 0;
}

void cache_label(void* v, io_buf& cache)
{
}

void default_label(void* v)
{
}

void delete_label(void* v)
{
}

size_t increment=0;

void parse_label(void* v, v_array<substring>& words)
{
}

void print_update(example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      label* ld = (label*) ec->ld;
      char label_buf[32];
      if (is_test_label(ld))
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf," known");

      fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8i %8lu\n",
	      global.sd->sum_loss/global.sd->weighted_examples,
	      global.sd->sum_loss_since_last_dump / (global.sd->weighted_examples - global.sd->old_weighted_examples),
	      (long int)global.sd->example_number,
	      global.sd->weighted_examples,
	      label_buf,
	      *(OAA::prediction_t*)&ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sd->sum_loss_since_last_dump = 0.0;
      global.sd->old_weighted_examples = global.sd->weighted_examples;
      global.sd->dump_interval *= 2;
    }
}

void output_example(example* ec)
{
  label* ld = (label*)ec->ld;
  global.sd->weighted_examples += 1.;
  global.sd->total_features += ec->num_features;
  float loss = 0.;
  if (!is_test_label(ld))
    {//need to compute exact loss
      size_t pred = *(OAA::prediction_t*)&ec->final_prediction;

      float chosen_loss = FLT_MAX;
      float min = FLT_MAX;
      for (feature *cl = ld->costs.begin; cl != ld->costs.end; cl ++) {
        if (cl->weight_index == pred)
          chosen_loss = cl->x;
        if (cl->x < min)
          min = cl->x;
      }
      if (chosen_loss == FLT_MAX)
        cerr << "warning: csoaa predicted an invalid class" << endl;

      loss = chosen_loss - min;
    }

  global.sd->sum_loss += loss;
  global.sd->sum_loss_since_last_dump += loss;
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i];
      global.print(f, *(OAA::prediction_t*)&ec->final_prediction, 0, ec->tag);
    }
  
  global.sd->example_number++;

  print_update(ec);
}


  void learn(example* ec)
  {
    label* ld = (label*)ec->ld;
    float prediction = 1;
    float score = FLT_MAX;
    size_t current_increment = 0;
    
    for (feature *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      {
        size_t i = cl->weight_index;

	label_data simple_temp;
	simple_temp.initial = 0.;
	if (is_test_label(ld))
	  {
	    simple_temp.label = FLT_MAX;
	    simple_temp.weight = 0.;
	  }
	else
	  {
	    simple_temp.label = cl->x;
	    simple_temp.weight = 1.;
	  }
	ec->ld = &simple_temp;

        size_t desired_increment = increment * (i-1);
        if (desired_increment != current_increment) {
	  OAA::update_indicies(ec, desired_increment - current_increment);
          current_increment = desired_increment;
        }
	ec->partial_prediction = 0.;

	global.learn(ec);
	if (ec->partial_prediction < score)
	  {
	    score = ec->partial_prediction;
	    prediction = i;
	  }
      }
    ec->ld = ld;
    *(OAA::prediction_t*)&(ec->final_prediction) = prediction;
    if (current_increment != 0)
      OAA::update_indicies(ec, -current_increment);
  }

  void initialize()
{
  global.initialize();
}

void finalize()
{
  global.finish();
}

void drive_csoaa()
{
  example* ec = NULL;
  initialize();
  while ( true )
    {
      if ((ec = get_example()) != NULL)//semiblocking operation.
	{
	  learn(ec);
          output_example(ec);
	  free_example(ec);
	}
      else if (parser_done())
	{
	  finalize();
	  return;
	}
      else 
	;
    }
}

void parse_flag(size_t s)
{
  *(global.lp) = cs_label_parser;
  global.k = s;
  global.driver = drive_csoaa;
  global.cs_initialize = initialize;
  global.cs_learn = learn;
  global.cs_finish = finalize;
  increment = (global.length()/global.k) * global.stride;
}

}
