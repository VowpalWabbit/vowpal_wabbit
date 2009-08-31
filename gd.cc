/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <float.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"

void* gd_thread(void *in)
{
  gd_thread_params* params = (gd_thread_params*) in;
  regressor reg = params->reg;
  size_t thread_num = params->thread_num;
  example* ec = NULL;

  while ( (ec = get_example(ec,thread_num)) )
    {
      label_data* ld = (label_data*)ec->ld;
      if ( ((ld->tag).begin != (ld->tag).end) 
	   && ((ld->tag)[0] == 's')&&((ld->tag)[1] == 'a')&&((ld->tag)[2] == 'v')&&((ld->tag)[3] == 'e'))
	{
	  if ((*(params->final_regressor_name)) != "") 
	    {
	      ofstream tempOut;
	      tempOut.open((*(params->final_regressor_name)).c_str());
	      dump_regressor(tempOut, reg);
	    }
	}
      else
	train_one_example(reg,ec,thread_num,*(params->vars));
    }

  return NULL;
}

float final_prediction(float ret, size_t num_features, gd_vars& vars, float &norm) 
{
  if (num_features > 0)
    norm = 1. / sqrtf(num_features);
  else 
    norm = 1.;
  ret *= norm;
  if (isnan(ret))
    return 0.5;
  if ( ret > vars.max_prediction )
    return vars.max_prediction;
  if (ret < vars.min_prediction)
    return vars.min_prediction;
  return ret;

}

float inline_predict(regressor &reg, example* &ec, size_t thread_num)
{
  float prediction = 0.0;

  weight* weights = reg.weight_vectors[thread_num];
  size_t thread_mask = reg.global->thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    prediction += sd_add(weights,thread_mask,ec->subsets[*i][thread_num], ec->subsets[*i][thread_num+1]);
  
  for (vector<string>::iterator i = reg.global->pairs.begin(); i != reg.global->pairs.end();i++) 
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	  temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	  for (; temp.begin != temp.end; temp.begin++)
	    prediction += one_pf_quad_predict(weights,*temp.begin,
					      ec->atomics[(int)(*i)[1]],thread_mask);
	}
    }
  
  if ( thread_num == 0 ) 
    prediction += weights[0];
  
  return prediction;
}

float inline_offset_predict(regressor &reg, example* &ec, size_t thread_num, size_t offset)
{
  float prediction = 0.0;

  weight* weights = reg.weight_vectors[thread_num];
  size_t thread_mask = reg.global->thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    for (feature *f = ec->subsets[*i][thread_num]; f != ec->subsets[*i][thread_num+1]; f++)
      prediction += weights[(f->weight_index + offset) & thread_mask] * f->x;
  for (vector<string>::iterator i = reg.global->pairs.begin(); i != reg.global->pairs.end();i++) 
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	  temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	  for (; temp.begin != temp.end; temp.begin++)
	    prediction += offset_quad_predict(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, offset);
	}
    }

  if ( thread_num == 0 )
    prediction += weights[offset & thread_mask];

  return prediction;
}

void print_offset_features(regressor &reg, example* &ec, size_t offset)
{
  weight* weights = reg.weight_vectors[0];
  size_t thread_mask = reg.global->thread_mask;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
      for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	{
	  // old
	  //	  cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index << ':' << f->x;
	  // new
	  cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index <<"(" << ((f->weight_index + offset) & thread_mask)  << ")" << ':' << f->x;

	  cout << ':' << weights[(f->weight_index + offset) & thread_mask];
	}
    else
      for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	{
	  cout << '\t' << f->weight_index << ':' << f->x;
	  cout << ':' << weights[(f->weight_index + offset) & thread_mask];
	}
  for (vector<string>::iterator i = reg.global->pairs.begin(); i != reg.global->pairs.end();i++) 
    if (ec->audit_features[(int)(*i)[0]].begin != ec->audit_features[(int)(*i)[0]].end)
      for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f != ec->audit_features[(int)(*i)[0]].end; f++)
	print_offset_audit_quad(weights, *f, ec->audit_features[(int)(*i)[1]], reg.global->thread_mask, offset);
    else
      for (feature* f = ec->atomics[(int)(*i)[0]].begin; f != ec->atomics[(int)(*i)[0]].end; f++)
	print_offset_quad(weights, *f, ec->atomics[(int)(*i)[1]], reg.global->thread_mask, offset);      

  cout << "\tConstant:0:1:" << weights[offset & reg.global->thread_mask] << endl;
}

float predict(weight* weights, const v_array<feature> &features)
{
  float prediction = 0.0;
  for (feature* j = features.begin; j != features.end; j++)
    prediction += weights[j->weight_index] * j->x;
  return prediction;
}

void one_pf_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    weights[(halfhash + ele->weight_index) & mask] += update * ele->x;
}

void offset_quad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float update, size_t offset)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index + offset;
  update *= page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    weights[(halfhash + ele->weight_index) & mask] += update * ele->x;
}

void inline_train(regressor &reg, example* &ec, size_t thread_num, float update)
{
  if (fabs(update) > 0.)
    {
      weight* weights = reg.weight_vectors[thread_num];
      size_t thread_mask = reg.global->thread_mask;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	{
	  feature *f = ec->subsets[*i][thread_num];
	  for (; f != ec->subsets[*i][thread_num+1]; f++)
	    weights[f->weight_index & thread_mask] += update * f->x;
	}
      
      for (vector<string>::iterator i = reg.global->pairs.begin(); i != reg.global->pairs.end();i++) 
	{
	  if (ec->subsets[(int)(*i)[0]].index() > 0)
	    {
	      v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	      temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	      temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	      for (; temp.begin != temp.end; temp.begin++)
		one_pf_quad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, update);
	    } 
	}
      
      if ( thread_num == 0 )
	weights[0] += update;
    }
}  

void offset_train(regressor &reg, example* &ec, size_t thread_num, float update, size_t offset)
{
  if (fabs(update) > 0.)
    {
      weight* weights = reg.weight_vectors[thread_num];
      size_t thread_mask = reg.global->thread_mask;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	for (feature *f = ec->subsets[*i][thread_num]; f != ec->subsets[*i][thread_num+1]; f++)
	  weights[(f->weight_index+offset) & thread_mask] += update * f->x;
      
      for (vector<string>::iterator i = reg.global->pairs.begin(); i != reg.global->pairs.end();i++) 
	{
	  if (ec->subsets[(int)(*i)[0]].index() > 0)
	    {
	      v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	      temp.begin = ec->subsets[(int)(*i)[0]][thread_num];
	      temp.end = ec->subsets[(int)(*i)[0]][thread_num+1];
	      for (; temp.begin != temp.end; temp.begin++)
		offset_quad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, update, offset);
	    } 
	}
      
      if ( thread_num == 0 )
	weights[offset & thread_mask] += update;
    }
}  

void train(weight* weights, const v_array<feature> &features, float update)
{
  if (fabs(update) > 0.)
    for (feature* j = features.begin; j != features.end; j++)
      weights[j->weight_index] += update * j->x;
}

size_t read_cached_simple_label(void* v, io_buf& cache)
{
  label_data* ld = (label_data*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight)+int_size;
  size_t tag_size = 0;
  if (buf_read(cache, c, total) < total) 
    return 0;

  ld->label = *(double *)c;
  c += sizeof(ld->label);
  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  c = run_len_decode(c, tag_size);

  cache.set(c);
  if (buf_read(cache, c, tag_size) < tag_size) 
    return 0;

  ld->tag.erase();
  push_many(ld->tag, c, tag_size); 
  return total+tag_size;
}

void cache_simple_label(void* v, io_buf& cache)
{
  char *c;
  label_data* ld = (label_data*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight)+int_size+ld->tag.index());
  *(double *)c = ld->label;
  c += sizeof(ld->label);
  *(float *)c = ld->weight;
  c += sizeof(ld->weight);
  
  c = run_len_encode(c, ld->tag.index());
  memcpy(c,ld->tag.begin,ld->tag.index());
  c += ld->tag.index();
  cache.set(c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  ld->label = FLT_MAX;
  ld->weight = 1.;
  ld->undo = false;
  ld->tag.erase();
}

void delete_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  if (ld->tag.end_array != ld->tag.begin)
    {
      free(ld->tag.begin);
      ld->tag.end_array = ld->tag.begin;
    }
}

void parse_simple_label(void* v, substring label_space, v_array<substring>& words)
{
  label_data* ld = (label_data*)v;
  char* tab_location = safe_index(label_space.start,'\t',label_space.end);
  if (tab_location != label_space.end)
    label_space.start = tab_location+1;
  
  tokenize(' ',label_space, words);
  switch(words.index()) {
  case 0:
    break;
  case 1:
    ld->label = double_of_substring(words[0]);
    break;
  case 2:
    ld->label = double_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    break;
  case 3:
    ld->label = double_of_substring(words[0]);
    
    ld->weight = float_of_substring(words[1]);
    push_many(ld->tag, words[2].start, 
	      words[2].end - words[2].start);
    if (ld->tag.index() == 4 && ld->tag[0] == 'u' && ld->tag[1] == 'n' && ld->tag[2] == 'd' && ld->tag[3] == 'o')
      ld->undo = true;
    break;
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
}

void print(example *ec, gd_vars& vars)
{
  if (vars.weighted_examples > vars.dump_interval && !vars.quiet)
    {
      cerr.precision(4);
      cerr << vars.sum_loss/vars.weighted_examples << "\t" 
	   << vars.sum_loss_since_last_dump / (vars.weighted_examples - vars.old_weighted_examples) << "\t"
	   << vars.example_number << "\t";
      cerr.precision(2);
      cerr << vars.weighted_examples << "\t";
      cerr.precision(4);
      label_data* ld = (label_data*) ec->ld;
      cerr << ld->label << "\t" << ec->partial_prediction << "\t"
	   << ec->num_features << "\t" << endl;
      
      vars.sum_loss_since_last_dump = 0.0;
      vars.old_weighted_examples = vars.weighted_examples;
      vars.dump_interval *= 2;
    }
}

void print_result(int f, float res, v_array<char> tag)
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
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	cerr << "write error" << endl;
    }
}

void print_audit_features(regressor &reg, example* ec, size_t offset)
{
  label_data* ld = (label_data*) ec->ld;
  print_result(fileno(stdout),ec->partial_prediction,ld->tag);
  print_offset_features(reg, ec, offset);
}

/**
   calculates the update for this example and sets
   ec->eta_round to it.
 */
void compute_update(example* ec, gd_vars& vars)
{
  label_data* ld = (label_data*)ec->ld;

  /* the following is a wasted computation. */
  float norm;  
  if (ec->num_features > 0)
    norm = 1. / sqrtf(ec->num_features);
  else 
    norm = 1.;

  float example_loss = (ec->partial_prediction - ld->label) 
    * (ec->partial_prediction - ld->label);
  example_loss *= ld->weight;
  vars.t += ld->weight;
  vars.sum_loss = vars.sum_loss + example_loss;
  
  vars.sum_loss_since_last_dump += example_loss;
  print(ec,vars);      

  ec->eta_round = vars.eta/pow(vars.t,vars.power_t)
    * (ld->label - ec->partial_prediction)
    * norm * ld->weight;
    
  if (ld->undo)
    ec->eta_round = -ec->eta_round;
}

void process(example* ec, size_t num_threads, gd_vars& vars, regressor& reg, size_t offset)
{
  vars.example_number++;
  label_data* ld = (label_data*)ec->ld;
  vars.weighted_examples += ld->weight;
  vars.weighted_labels += ld->label * ld->weight;
  vars.total_features += ec->num_features;

  vars.print(vars.raw_predictions, ec->partial_prediction, ld->tag);

  float norm;
  ec->partial_prediction = 
    final_prediction(ec->partial_prediction, ec->num_features, vars, norm);

  vars.print(vars.predictions, ec->partial_prediction, ld->tag);
  if (reg.global->audit)
    print_audit_features(reg, ec, offset);

  if (ld->label != FLT_MAX)
    {
      float example_loss = reg.loss->getLoss(ec->partial_prediction, ld->label) * ld->weight;
      vars.t += ld->weight;
      vars.sum_loss = vars.sum_loss + example_loss;
            
      vars.sum_loss_since_last_dump += example_loss;

      print(ec,vars);
      ec->eta_round = vars.eta/pow(vars.t,vars.power_t)
	* (ld->label - ec->partial_prediction)
	* norm * ld->weight;
      
      float example_update = reg.loss->getUpdate(ec->partial_prediction, ld->label) * ld->weight;
      ec->eta_round = vars.eta/pow(vars.t,vars.power_t) * example_update * norm;
      if (ld->undo)
	ec->eta_round = -ec->eta_round;
    }
  ec->threads_to_finish = num_threads;
}

pthread_cond_t finished_sum = PTHREAD_COND_INITIALIZER;

float predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  float prediction = inline_predict(r, ex, thread_num);
  pthread_mutex_lock(&ex->lock);
  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0) 
    pthread_cond_wait(&finished_sum, &ex->lock);
  else // We are the last thread using this example.
    {
      process(ex, r.global->num_threads(),vars,r,0);
      pthread_cond_broadcast(&finished_sum);
    }
  pthread_mutex_unlock(&ex->lock);
  return ex->partial_prediction;
}

float offset_predict(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset)
{
  float prediction = inline_offset_predict(r, ex, thread_num, offset);
  pthread_mutex_lock(&ex->lock);
  ex->partial_prediction += prediction;
  if (--ex->threads_to_finish != 0) 
    pthread_cond_wait(&finished_sum, &ex->lock);
  else // We are the last thread using this example.
    {
      process(ex, r.global->num_threads(),vars,r,offset);
      pthread_cond_broadcast(&finished_sum);
    }
  pthread_mutex_unlock(&ex->lock);
  return ex->partial_prediction;
}

// trains regressor r on one example ex.
void train_one_example(regressor& r, example* ex, size_t thread_num, gd_vars& vars)
{
  predict(r,ex,thread_num,vars);
  label_data* ld = (label_data*) ex->ld;
  if (ld->label != FLT_MAX && vars.training) 
    inline_train(r, ex, thread_num, ex->eta_round);
}

// trains regressor r on one example ex.
void train_offset_example(regressor& r, example* ex, size_t thread_num, gd_vars& vars, size_t offset)
{
  offset_predict(r,ex,thread_num,vars,offset);
  label_data* ld = (label_data*) ex->ld;
  if (ld->label != FLT_MAX && vars.training) 
    offset_train(r, ex, thread_num, ex->eta_round, offset);
}

pthread_t* threads;
gd_thread_params** passers;
size_t num_threads;

void setup_gd(gd_thread_params t)
{
  num_threads = t.thread_num;
  threads = (pthread_t*)calloc(num_threads,sizeof(pthread_t));
  passers = (gd_thread_params**)calloc(num_threads,sizeof(gd_thread_params*));

  for (size_t i = 0; i < num_threads; i++)
    {
      passers[i] = (gd_thread_params*)calloc(1, sizeof(gd_thread_params));
      *(passers[i]) = t;
      passers[i]->thread_num = i;
      pthread_create(&threads[i], NULL, gd_thread, (void *) passers[i]);
    }
}

void destroy_gd()
{
  for (size_t i = 0; i < num_threads; i++) 
    {
      pthread_join(threads[i], NULL);
      free(passers[i]);
    }
  free(threads);
  free(passers);
}

