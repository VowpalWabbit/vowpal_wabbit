#include <float.h>
#include <math.h>
#include <stdio.h>

#include "oaa.h"
#include "simple_label.h"
#include "cache.h"

using namespace std;

namespace OAA {

char* bufread_label(mc_label* ld, char* c)
{
  ld->label = *(uint32_t *)c;
  c += sizeof(ld->label);
  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  return c;
}

size_t read_cached_label(void* v, io_buf& cache)
{
  mc_label* ld = (mc_label*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_label(ld,c);

  return total;
}

float weight(void* v)
{
  mc_label* ld = (mc_label*) v;
  return ld->weight;
}

float initial(void* v)
{
  return 0.;
}

char* bufcache_label(mc_label* ld, char* c)
{
  *(uint32_t *)c = ld->label;
  c += sizeof(ld->label);
  *(float *)c = ld->weight;
  c += sizeof(ld->weight);
  return c;
}

void cache_label(void* v, io_buf& cache)
{
  char *c;
  mc_label* ld = (mc_label*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
  c = bufcache_label(ld,c);
}

void default_label(void* v)
{
  mc_label* ld = (mc_label*) v;
  ld->label = -1;
  ld->weight = 1.;
}

void delete_label(void* v)
{
}

void parse_label(void* v, v_array<substring>& words)
{
  mc_label* ld = (mc_label*)v;

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
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
}

size_t k=0;
size_t increment=0;
size_t total_increment=0;

void print_update(example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      mc_label* ld = (mc_label*) ec->ld;
      char label_buf[32];
      if (ld->label == INT_MAX)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf,"%8i",ld->label);

      fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8i %8lu\n",
	      global.sd->sum_loss/global.sd->weighted_examples,
	      global.sd->sum_loss_since_last_dump / (global.sd->weighted_examples - global.sd->old_weighted_examples),
	      (long int)global.sd->example_number,
	      global.sd->weighted_examples,
	      label_buf,
	      *(prediction_t*)&ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sd->sum_loss_since_last_dump = 0.0;
      global.sd->old_weighted_examples = global.sd->weighted_examples;
      global.sd->dump_interval *= 2;
    }
}

void output_example(example* ec)
{
  mc_label* ld = (mc_label*)ec->ld;
  global.sd->weighted_examples += ld->weight;
  global.sd->total_features += ec->num_features;
  size_t loss = 1;
  if (ld->label == *(prediction_t*)&(ec->final_prediction))
    loss = 0;
  global.sd->sum_loss += loss;
  global.sd->sum_loss_since_last_dump += loss;
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i];
      global.print(f, *(prediction_t*)&(ec->final_prediction), 0, ec->tag);
    }
  
  global.sd->example_number++;

  print_update(ec);
}

void update_indicies(example* ec, size_t amount)
{
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature* end = ec->atomics[*i].end;
      for (feature* f = ec->atomics[*i].begin; f!= end; f++)
	f->weight_index += amount;
    }
  if (global.audit)
    {
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	    f->weight_index += amount;
    }
}

  void (*base_learner)(example*) = NULL;
  void (*base_finish)() = NULL;

void learn(example* ec)
{
  mc_label* mc_label_data = (mc_label*)ec->ld;
  size_t prediction = 1;
  float score = INT_MIN;
  
  if (mc_label_data->label > k && mc_label_data->label != (uint32_t)-1)
    cerr << "warning: label " << mc_label_data->label << " is greater than " << k << endl;
  
  for (size_t i = 1; i <= k; i++)
    {
      label_data simple_temp;
      simple_temp.initial = 0.;
      if (mc_label_data->label == i)
	simple_temp.label = 1;
      else
	simple_temp.label = -1;
      simple_temp.weight = mc_label_data->weight;
      ec->ld = &simple_temp;
      if (i != 0)
	update_indicies(ec, increment);
      base_learner(ec);
      if (ec->partial_prediction > score)
	{
	  score = ec->partial_prediction;
	  prediction = i;
	}
      ec->partial_prediction = 0.;
    }
  ec->ld = mc_label_data;
  *(prediction_t*)&(ec->final_prediction) = prediction;
  update_indicies(ec, -total_increment);
}

void finish()
{
  base_finish();
}

void drive_oaa()
{
  example* ec = NULL;
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
	  finish();
	  return;
	}
      else 
	;
    }
}

  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)())
{
  *(global.lp) = mc_label_parser;
  k = s;
  global.driver = drive_oaa;
  base_learner = base_l;
  base_finish = base_f;
  increment = (global.length()/k) * global.stride;
  total_increment = increment*(k-1);
}

}
