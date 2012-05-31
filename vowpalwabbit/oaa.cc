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

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
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

  void parse_label(shared_data*, void* v, v_array<substring>& words)
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

  //nonreentrant
size_t k=0;
size_t increment=0;
size_t total_increment=0;

  void print_update(vw& all, example *ec)
{
  if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
      mc_label* ld = (mc_label*) ec->ld;
      char label_buf[32];
      if (ld->label == INT_MAX)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf,"%8i",ld->label);

      fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8i %8lu\n",
	      all.sd->sum_loss/all.sd->weighted_examples,
	      all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
	      (long int)all.sd->example_number,
	      all.sd->weighted_examples,
	      label_buf,
	      *(prediction_t*)&ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      all.sd->sum_loss_since_last_dump = 0.0;
      all.sd->old_weighted_examples = all.sd->weighted_examples;
      all.sd->dump_interval *= 2;
    }
}

  void output_example(vw& all, example* ec)
{
  mc_label* ld = (mc_label*)ec->ld;
  all.sd->weighted_examples += ld->weight;
  all.sd->total_features += ec->num_features;
  size_t loss = 1;
  if (ld->label == *(prediction_t*)&(ec->final_prediction))
    loss = 0;
  all.sd->sum_loss += loss;
  all.sd->sum_loss_since_last_dump += loss;
  
  for (size_t i = 0; i<all.final_prediction_sink.index(); i++)
    {
      int f = all.final_prediction_sink[i];
      all.print(f, *(prediction_t*)&(ec->final_prediction), 0, ec->tag);
    }
  
  all.sd->example_number++;

  print_update(all, ec);
}

  void update_indicies(vw& all, example* ec, size_t amount)
{
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature* end = ec->atomics[*i].end;
      for (feature* f = ec->atomics[*i].begin; f!= end; f++)
	f->weight_index += amount;
    }
  if (all.audit)
    {
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	    f->weight_index += amount;
    }
}

  void (*base_learner)(void*,example*) = NULL;

  void learn(void*a, example* ec)
{
  vw* all = (vw*)a;
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
	update_indicies(*all, ec, increment);
      base_learner(all,ec);
      if (ec->partial_prediction > score)
	{
	  score = ec->partial_prediction;
	  prediction = i;
	}
      ec->partial_prediction = 0.;
    }
  ec->ld = mc_label_data;
  *(prediction_t*)&(ec->final_prediction) = prediction;
  update_indicies(*all, ec, -total_increment);
}

void drive_oaa(void *in)
{
  vw* all = (vw*)in;
  example* ec = NULL;
  while ( true )
    {
      if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
	{
	  learn(all, ec);
          output_example(*all, ec);
	  free_example(*all, ec);
	}
      else if (parser_done(all->p))
	{
	  all->finish(all);
	  return;
	}
      else 
	;
    }
}

  void parse_flags(vw& all, size_t s)
  {
    *(all.lp) = mc_label_parser;
    k = s;
    all.driver = drive_oaa;
    base_learner = all.learn;
    all.learn = learn;
    increment = (all.length()/k) * all.stride;
    total_increment = increment*(k-1);
  }
}
