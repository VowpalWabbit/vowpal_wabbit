#include <float.h>
#include <math.h>
#include <stdio.h>

#include "wap.h"
#include "simple_label.h"
#include "cache.h"
#include "csoaa.h"
#include "oaa.h"

using namespace std;

namespace WAP {
  
  size_t increment=0;
  size_t total_increment=0;

void mirror_features(example* ec, size_t offset1, size_t offset2)
{
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      size_t original_length = ec->atomics[*i].index();
      //cerr << "original_length = " << original_length << endl;
      for (size_t j = 0; j < original_length; j++)
	{
	  feature* f = &ec->atomics[*i][j];
	  feature temp = {- f->x, f->weight_index + offset2};
	  f->weight_index += offset1;
	  push(ec->atomics[*i], temp);
	}
      ec->sum_feat_sq[*i] *= 2;
      //cerr << "final_length = " << ec->atomics[*i].index() << endl;
    }
  if (global.audit)
    {
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  {
	    size_t original_length = ec->audit_features[*i].index();
	    for (size_t j = 0; j < original_length; j++)
	      {
		audit_data* f = &ec->audit_features[*i][j];
		char* new_space = NULL;
		if (f->space != NULL)
		  {
		    new_space = (char*)calloc(strlen(f->space)+1,sizeof(char));
		    strcpy(new_space, f->space);
		  }
		char* new_feature = (char*)calloc(strlen(f->feature)+2,sizeof(char));
		strcpy(new_feature+1, f->feature);
		*new_feature = '-';
		audit_data temp = {new_space, new_feature, f->weight_index + offset2, - f->x, true};
		f->weight_index += offset1;
		push(ec->audit_features[*i], temp);
	      }
            //cerr << "final_length = " << ec->audit_features[*i].index() << endl;
	  }
    }
  ec->num_features *= 2;
  ec->total_sum_feat_sq *= 2;
  //cerr << "total_sum_feat_sq = " << ec->total_sum_feat_sq << endl;
}

void unmirror_features(example* ec, size_t offset1, size_t offset2)
{
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      ec->atomics[*i].end = ec->atomics[*i].begin+ec->atomics[*i].index()/2;
      feature* end = ec->atomics[*i].end;
      for (feature* f = ec->atomics[*i].begin; f!= end; f++)
	f->weight_index -= offset1;
      ec->sum_feat_sq[*i] /= 2;
    }
  if (global.audit)
    {
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
	  {
	    for (audit_data *f = ec->audit_features[*i].begin + ec->audit_features[*i].index()/2; f != ec->audit_features[*i].end; f++)
	      {
		if (f->space != NULL)
		  free(f->space);
		free(f->feature);
		f->alloced = false;
	      }
	    ec->audit_features[*i].end = ec->audit_features[*i].begin+ec->audit_features[*i].index()/2;
	    for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	      f->weight_index -= offset1;
	  }
    }
  ec->num_features /= 2;
  ec->total_sum_feat_sq /= 2;
}

  struct float_index 
  {
    float v;
    float c;
    uint32_t i;
  };
  int fi_compare(const void *e1, const void* e2)
  {
    float_index* fi1 = (float_index*)e1;
    float_index* fi2 = (float_index*)e2;
    if (fi1->c > fi2->c)
      return 1;
    else if (fi1->c < fi2->c)
      return -1;
    else
      return 0;
  }
  int fi_compare_i(const void *e1, const void* e2)
  {
    float_index* fi1 = (float_index*)e1;
    float_index* fi2 = (float_index*)e2;
    if (fi1->i > fi2->i)
      return 1;
    else if (fi1->i < fi2->i)
      return -1;
    else
      return 0;
  }
  v_array<float_index> vs;

void train(example* ec)
{
  CSOAA::label* cost_label = (CSOAA::label*)ec->ld;
  
  float score = FLT_MAX;
  vs.erase();
  for (size_t i = 0; i < global.k; i++)
    {
      float_index temp = {0., cost_label->costs[i], i};
      if (temp.c < score)
	score = temp.c;
      push(vs, temp);
    }
  
  qsort(vs.begin, vs.index(), sizeof(float_index), fi_compare);
  
  for (size_t i = 0; i < global.k; i++)
    {
      vs[i].c -= score;
      if (i == 0)
	vs[i].v = 0.;
      else
	vs[i].v = vs[i-1].v + (vs[i].c-vs[i-1].c) / (float)i;

      //cerr << "vs[" << i << "] = " << vs[i].v << endl;
    }
  
  qsort(vs.begin, vs.index(), sizeof(float_index), fi_compare_i);

  for (size_t i = 1; i <= global.k; i++)
    for (size_t j = i+1; j <= global.k; j++)
      {
        //cerr << i << " vs " << j << endl;
	label_data simple_temp;
	simple_temp.weight = fabsf(vs[i-1].v - vs[j-1].v);
	if (simple_temp.weight > 1e-5)
	  {
	    simple_temp.initial = 0.;
	    
	    if (vs[i-1].v < vs[j-1].v)
	      simple_temp.label = 1;
	    else
	      simple_temp.label = -1;
	    
	    ec->ld = &simple_temp;
	    
	    ec->partial_prediction = 0.;
	    mirror_features(ec,(i-1)*increment, (j-1)*increment);

            //cerr << "label = " << simple_temp.label << ", weight = " << simple_temp.weight << endl;

	    global.learn(ec);
	    unmirror_features(ec,(i-1)*increment, (j-1)*increment);
	  }
      }
  
  ec->ld = cost_label;
}

uint32_t test(example* ec)
{
  uint32_t prediction = 1;
  float score = FLT_MIN;
  
  for(size_t i = 1; i <= global.k; i++)
    {
      label_data simple_temp;
      simple_temp.initial = 0.;
      simple_temp.weight = 0.;
      simple_temp.label = FLT_MAX;
      if (i!= 1)
	OAA::update_indicies(ec, increment);
      ec->partial_prediction = 0.;
      ec->ld = &simple_temp;
      global.learn(ec);
      if (ec->partial_prediction > score)
	{
	  score = ec->partial_prediction;
	  prediction = i;
	}
    }
  OAA::update_indicies(ec, -total_increment);  
  return prediction;
}

  void learn(example* ec)
  {
    CSOAA::label* cost_label = (CSOAA::label*)ec->ld;
    
    uint32_t prediction = test(ec);

    ec->ld = cost_label;
    
    if (cost_label->costs.index() > 0)
      train(ec);
    *(OAA::prediction_t*)&(ec->final_prediction) = prediction;
  }

  void initialize()
{
  global.initialize();
}

void finalize()
{
  global.finish();
}

void drive_wap()
{
  example* ec = NULL;
  initialize();
  while ( true )
    {
      if ((ec = get_example()) != NULL)//semiblocking operation.
	{
	  learn(ec);
          CSOAA::output_example(ec);
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
  *(global.lp) = CSOAA::cs_label_parser;
  global.k = s;
  global.driver = drive_wap;
  global.cs_initialize = initialize;
  global.cs_learn = learn;
  global.cs_finish = finalize;
  increment = (global.length()/global.k) * global.stride;
  total_increment = increment*(global.k-1);
}
}
