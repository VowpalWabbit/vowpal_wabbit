#include <float.h>
#include <math.h>
#include <stdio.h>

#include "csoaa.h"
#include "simple_label.h"
#include "cache.h"
#include "oaa.h"

using namespace std;

namespace CSOAA {
  
char* bufread_label(label* ld, char* c, io_buf& cache)
{
  uint32_t num = *(uint32_t *)c;
  c += sizeof(uint32_t);
  size_t total = sizeof(float)*num;
  if (buf_read(cache, c, total) < total) 
    {
      cout << "error in demarshal of cost data" << endl;
      return c;
    }
  for (uint32_t i = 0; i<num; i++)
    {
      float temp = *(float *)c;
      c += sizeof(float);
      push(ld->costs, temp);
    }
  
  return c;
}

size_t read_cached_label(void* v, io_buf& cache)
{
  label* ld = (label*) v;
  char *c;
  size_t total = sizeof(uint32_t);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_label(ld,c, cache);

  return total;
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
  *(uint32_t *)c = ld->costs.index();
  c += sizeof(uint32_t);
  for (size_t i = 0; i< ld->costs.index(); i++)
    {
      *(float *)c = ld->costs[i];
      c += sizeof(float);
    }
  return c;
}

void cache_label(void* v, io_buf& cache)
{
  char *c;
  label* ld = (label*) v;
  buf_write(cache, c, sizeof(uint32_t)+sizeof(float)*ld->costs.index());
  bufcache_label(ld,c);
}

void default_label(void* v)
{
  label* ld = (label*) v;
  ld->costs.erase();
}

void delete_label(void* v)
{
  label* ld = (label*) v;
  if (ld->costs.begin != NULL)
    free (ld->costs.begin);
}

size_t k=0;
size_t increment=0;
size_t total_increment=0;

void parse_label(void* v, v_array<substring>& words)
{
  label* ld = (label*)v;

  if(words.index() != 0 && words.index() != k)
    {
      cerr << "malformed example!\n";
      cerr << "#costs = " << words.index() << " but " << k << " or 0 required" << endl;
    }

  for (size_t i = 0; i < words.index(); i++)
    push(ld->costs, float_of_substring(words[i]));
}

void print_update(example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      label* ld = (label*) ec->ld;
      char label_buf[32];
      if (ld->costs.index() == 0)
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
  if (ld->costs.index() == k)
    {//need to compute exact loss
      float chosen_loss = ld->costs[*(OAA::prediction_t*)&ec->final_prediction -1];
      float min = INT_MAX;
      for (size_t i = 0; i < k; i++)
	{
	  if (ld->costs[i] < min)
	    min = ld->costs[i];
	}
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
    label* cost_label = (label*)ec->ld;
    float prediction = 1;
    float score = INT_MAX;
    
    for (size_t i = 1; i <= k; i++)
      {
	label_data simple_temp;
	simple_temp.initial = 0.;
	if (cost_label->costs.index() == k)
	  {
	    simple_temp.label = cost_label->costs[i-1];
	    simple_temp.weight = 1.;
	  }
	else
	  {
	    simple_temp.label = 0.;
	    simple_temp.weight = 0.;
	  }
	ec->ld = &simple_temp;
	if (i != 1)
	  OAA::update_indicies(ec, increment);
	ec->partial_prediction = 0.;
	global.learn(ec);
	if (ec->partial_prediction < score)
	  {
	    score = ec->partial_prediction;
	    prediction = i;
	  }
      }
    ec->ld = cost_label;
    *(OAA::prediction_t*)&(ec->final_prediction) = prediction;
    OAA::update_indicies(ec, -total_increment);
    output_example(ec);
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
	learn(ec);
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
  *(global.lp) = csoaa_label;
  k = s;
  global.driver = drive_csoaa;
  global.cs_initialize = initialize;
  global.cs_learn = learn;
  global.cs_finish = finalize;
  increment = (global.length()/k) * global.stride;
  total_increment = increment*(k-1);
}

}
