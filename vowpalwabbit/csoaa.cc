#include <float.h>
#include <math.h>
#include <stdio.h>

#include "csoaa.h"
#include "simple_label.h"
#include "cache.h"
#include "oaa.h"

using namespace std;

char* bufread_csoaa_label(csoaa_data* ld, char* c, io_buf& cache)
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

size_t read_cached_csoaa_label(void* v, io_buf& cache)
{
  csoaa_data* ld = (csoaa_data*) v;
  char *c;
  size_t total = sizeof(uint32_t);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_csoaa_label(ld,c, cache);

  return total;
}

float csoaa_weight(void* v)
{
  return 1.;
}

float csoaa_initial(void* v)
{
  return 0.;
}

char* bufcache_csoaa_label(csoaa_data* ld, char* c)
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

void cache_csoaa_label(void* v, io_buf& cache)
{
  char *c;
  csoaa_data* ld = (csoaa_data*) v;
  buf_write(cache, c, sizeof(uint32_t)+sizeof(float)*ld->costs.index());
  bufcache_csoaa_label(ld,c);
}

void default_csoaa_label(void* v)
{
  csoaa_data* ld = (csoaa_data*) v;
  ld->costs.erase();
}

void delete_csoaa_label(void* v)
{
  csoaa_data* ld = (csoaa_data*) v;
  if (ld->costs.begin != NULL)
    free (ld->costs.begin);
}

size_t csoaa_k=0;
label_data csoaa_simple_temp;
csoaa_data* csoaa_label_data;
size_t csoaa_increment=0;
size_t csoaa_counter = 0;
example* csoaa_current_example=NULL;
size_t csoaa_prediction = 1;
float csoaa_score = INT_MAX;
example* (*csoaa_gf)();
void (*csoaa_rf)(example*);

void parse_csoaa_label(void* v, v_array<substring>& words)
{
  csoaa_data* ld = (csoaa_data*)v;

  if(words.index() != 0 && words.index() != csoaa_k)
    {
      cerr << "malformed example!\n";
      cerr << "#costs = " << words.index() << " but " << csoaa_k << " or 0 required" << endl;
    }

  for (size_t i = 0; i < words.index(); i++)
    push(ld->costs, float_of_substring(words[0]));
}

void print_csoaa_update(example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      csoaa_data* ld = (csoaa_data*) ec->ld;
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
	      (int)csoaa_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sd->sum_loss_since_last_dump = 0.0;
      global.sd->old_weighted_examples = global.sd->weighted_examples;
      global.sd->dump_interval *= 2;
    }
}

void output_csoaa_example(example* ec)
{
  csoaa_data* ld = (csoaa_data*)ec->ld;
  global.sd->weighted_examples += 1.;
  global.sd->total_features += ec->num_features;
  float loss = 0.;
  if (ld->costs.index() == csoaa_k)
    {//need to compute exact loss
      float chosen_loss = ld->costs[csoaa_prediction];
      float min = INT_MAX;
      for (size_t i = 0; i < csoaa_k; i++)
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
      global.print(f, csoaa_prediction, 0, ec->tag);
    }
  
  global.sd->example_number++;

  print_csoaa_update(ec);
}

void return_csoaa_example(example* ec)
{
  if (ec==NULL)
    {
      free_example(ec);
      csoaa_current_example = NULL;
      csoaa_counter = 1;
      return;
    }
  if (ec->partial_prediction < csoaa_score)
    {
      csoaa_score = ec->partial_prediction;
      csoaa_prediction = csoaa_counter;
    }
  if (csoaa_counter == csoaa_k)
    {
      ec->ld = csoaa_label_data;
      output_csoaa_example(ec);
      ec->final_prediction = csoaa_prediction;
      csoaa_rf(ec);
      csoaa_current_example = NULL;
      csoaa_counter = 1;
    }
  else
    {
      csoaa_counter++;
      csoaa_current_example = ec;
    }
}

example* get_csoaa_example()
{
  if (csoaa_current_example == NULL) {
    csoaa_current_example=csoaa_gf();
  }
  if (csoaa_current_example == NULL)
    return NULL;
  if (csoaa_counter == 1)
    {
      csoaa_label_data = (csoaa_data*)csoaa_current_example->ld;
      csoaa_prediction = 1;
      csoaa_score = INT_MAX;
    }
  else
    csoaa_current_example->partial_prediction = 0.;

  if (csoaa_label_data->costs.index() == csoaa_k)
    {
      csoaa_simple_temp.label = csoaa_label_data->costs[csoaa_counter];
      csoaa_simple_temp.weight = 1.;
    }
  else
    {
      csoaa_simple_temp.label = 0.;
      csoaa_simple_temp.weight = 0.;
    }
  csoaa_current_example->ld = &csoaa_simple_temp;
  if (csoaa_counter != 1)
    OAA::update_indicies(csoaa_current_example, csoaa_increment);
  return csoaa_current_example;
}

void parse_csoaa_flag(size_t s, example* (*get_function)(), void (*return_function)(example*) )
{
  *(global.lp) = csoaa_label;
  csoaa_gf = get_function;
  csoaa_rf = return_function;
  csoaa_k = s;
  csoaa_counter = 1;
  csoaa_increment = (global.length()/csoaa_k) * global.stride;
}
