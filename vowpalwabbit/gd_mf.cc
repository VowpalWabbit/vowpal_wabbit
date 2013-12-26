/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <float.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"
#include "simple_label.h"
#include "rand48.h"
#include "vw.h"

using namespace std;

namespace GDMF {
  struct gdmf {
    vw* all;
  };

void mf_local_predict(example* ec, regressor& reg);

float mf_inline_predict(vw& all, example* &ec)
{
  float prediction = all.p->lp->get_initial(ec->ld);

  // clear stored predictions
  ec->topic_predictions.erase();

  float linear_prediction = 0;
  // linear terms
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
    GD::foreach_feature<float*,vec_add>(all, &linear_prediction, ec->atomics[*i].begin, ec->atomics[*i].end);

  // store constant + linear prediction
  // note: constant is now automatically added
  ec->topic_predictions.push_back(linear_prediction);
  
  prediction += linear_prediction;

  // interaction terms
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].size() > 0 && ec->atomics[(int)(*i)[1]].size() > 0)
	{
	  for (uint32_t k = 1; k <= all.rank; k++)
	    {
	      // x_l * l^k
	      // l^k is from index+1 to index+all.rank
	      //float x_dot_l = sd_offset_add(weights, mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k);
              float x_dot_l = 0;
	      GD::foreach_feature<float*,vec_add>(all, &x_dot_l, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k);
	      // x_r * r^k
	      // r^k is from index+all.rank+1 to index+2*all.rank
	      //float x_dot_r = sd_offset_add(weights, mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+all.rank);
              float x_dot_r = 0;
	      GD::foreach_feature<float*,vec_add>(all, &x_dot_r, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+all.rank);

	      prediction += x_dot_l * x_dot_r;

	      // store prediction from interaction terms
	      ec->topic_predictions.push_back(x_dot_l);
	      ec->topic_predictions.push_back(x_dot_r);
	    }
	}
    }

  if (all.triples.begin() != all.triples.end()) {
    cerr << "cannot use triples in matrix factorization" << endl;
    throw exception();
  }
    
  // ec->topic_predictions has linear, x_dot_l_1, x_dot_r_1, x_dot_l_2, x_dot_r_2, ... 

  return prediction;
}

void mf_inline_train(vw& all, example* &ec, float update)
{
      weight* weights = all.reg.weight_vector;
      size_t mask = all.reg.weight_mask;
      label_data* ld = (label_data*)ec->ld;

      // use final prediction to get update size
      // update = eta_t*(y-y_hat) where eta_t = eta/(3*t^p) * importance weight
      float eta_t = all.eta/pow(ec->example_t,all.power_t) / 3.f * ld->weight;
      update = all.loss->getUpdate(ec->final_prediction, ld->label, eta_t, 1.); //ec->total_sum_feat_sq);

      float regularization = eta_t * all.l2_lambda;

      // linear update
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
	sd_offset_update(weights, mask, ec->atomics[*i].begin, ec->atomics[*i].end, 0, update, regularization);
      
      // quadratic update
      for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
	{
	  if (ec->atomics[(int)(*i)[0]].size() > 0 && ec->atomics[(int)(*i)[1]].size() > 0)
	    {

	      // update l^k weights
	      for (size_t k = 1; k <= all.rank; k++)
		{
		  // r^k \cdot x_r
		  float r_dot_x = ec->topic_predictions[2*k];
		  // l^k <- l^k + update * (r^k \cdot x_r) * x_l
		  sd_offset_update(weights, mask, ec->atomics[(int)(*i)[0]].begin, ec->atomics[(int)(*i)[0]].end, k, update*r_dot_x, regularization);
		}

	      // update r^k weights
	      for (size_t k = 1; k <= all.rank; k++)
		{
		  // l^k \cdot x_l
		  float l_dot_x = ec->topic_predictions[2*k-1];
		  // r^k <- r^k + update * (l^k \cdot x_l) * x_r
		  sd_offset_update(weights, mask, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, k+all.rank, update*l_dot_x, regularization);
		}

	    }
	}
  if (all.triples.begin() != all.triples.end()) {
    cerr << "cannot use triples in matrix factorization" << endl;
    throw exception();
  }

}  

void mf_print_offset_features(vw& all, example* &ec, size_t offset)
{
  weight* weights = all.reg.weight_vector;
  size_t mask = all.reg.weight_mask;
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
    if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
      for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
	{
	  cout << '\t' << f->space << '^' << f->feature << ':' << f->weight_index <<"(" << ((f->weight_index + offset) & mask)  << ")" << ':' << f->x;

	  cout << ':' << weights[(f->weight_index + offset) & mask];
	}
    else
      for (feature *f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++)
	{
	  size_t index = (f->weight_index + offset) & all.reg.weight_mask;
	  
	  cout << "\tConstant:";
	  cout << (index/all.reg.stride & all.parse_mask) << ':' << f->x;
	  cout  << ':' << trunc_weight(weights[index], (float)all.sd->gravity) * (float)all.sd->contraction;
	}
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    if (ec->atomics[(int)(*i)[0]].size() > 0 && ec->atomics[(int)(*i)[1]].size() > 0)
      {
	/* print out nsk^feature:hash:value:weight:nsk^feature^:hash:value:weight:prod_weights */
	for (size_t k = 1; k <= all.rank; k++)
	  {
	    for (audit_data* f = ec->audit_features[(int)(*i)[0]].begin; f!= ec->audit_features[(int)(*i)[0]].end; f++)
	      for (audit_data* f2 = ec->audit_features[(int)(*i)[1]].begin; f2!= ec->audit_features[(int)(*i)[1]].end; f2++)
		{
		  cout << '\t' << f->space << k << '^' << f->feature << ':' << ((f->weight_index+k)&mask) 
		       <<"(" << ((f->weight_index + offset +k) & mask)  << ")" << ':' << f->x;
		  cout << ':' << weights[(f->weight_index + offset + k) & mask];
		  
		  cout << ':' << f2->space << k << '^' << f2->feature << ':' << ((f2->weight_index+k+all.rank)&mask) 
		       <<"(" << ((f2->weight_index + offset +k+all.rank) & mask)  << ")" << ':' << f2->x;
		  cout << ':' << weights[(f2->weight_index + offset + k+all.rank) & mask];
		  
		  cout << ':' <<  weights[(f->weight_index + offset + k) & mask] * weights[(f2->weight_index + offset + k + all.rank) & mask];
		}
	  }
      }
  if (all.triples.begin() != all.triples.end()) {
    cerr << "cannot use triples in matrix factorization" << endl;
    throw exception();
  }
  cout << endl;
}

void mf_print_audit_features(vw& all, example* ec, size_t offset)
{
  print_result(all.stdout_fileno,ec->final_prediction,-1,ec->tag);
  mf_print_offset_features(all, ec, offset);
}

void mf_local_predict(vw& all, example* ec)
{
  label_data* ld = (label_data*)ec->ld;
  all.set_minmax(all.sd, ld->label);

  ec->final_prediction = GD::finalize_prediction(all, ec->partial_prediction);

  if (ld->label != FLT_MAX)
    {
      ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ld->label) * ld->weight;
    }

  if (all.audit)
    mf_print_audit_features(all, ec, 0);
}

float mf_predict(vw& all, example* ex)
{
  float prediction = mf_inline_predict(all, ex);

  ex->partial_prediction = prediction;
  mf_local_predict(all, ex);

  return ex->final_prediction;
}

  void save_load(void* d, io_buf& model_file, bool read, bool text)
{
  vw* all = ((gdmf*)d)->all;
  uint32_t length = 1 << all->num_bits;
  uint32_t stride = all->reg.stride;

  if(read)
    {
      initialize_regressor(*all);
      if(all->random_weights)
	for (size_t j = 0; j < all->reg.stride*length; j++)
	  all->reg.weight_vector[j] = (float) (0.1 * frand48()); 
    }

  if (model_file.files.size() > 0)
    {
      uint32_t i = 0;
      uint32_t text_len;
      char buff[512];
      size_t brw = 1;

      do 
	{
	  brw = 0;
	  size_t K = all->rank*2+1;
	  
	  text_len = sprintf(buff, "%d ", i);
	  brw += bin_text_read_write_fixed(model_file,(char *)&i, sizeof (i),
					   "", read,
					   buff, text_len, text);
	  if (brw != 0)
	    for (uint32_t k = 0; k < K; k++)
	      {
		uint32_t ndx = stride*i+k;
		
		weight* v = &(all->reg.weight_vector[ndx]);
		text_len = sprintf(buff, "%f ", *v);
		brw += bin_text_read_write_fixed(model_file,(char *)v, sizeof (*v),
						 "", read,
						 buff, text_len, text);
		
	      }
	  if (text)
	    brw += bin_text_read_write_fixed(model_file,buff,0,
					     "", read,
					     "\n",1,text);
	  
	  if (!read)
	    i++;
	}  
      while ((!read && i < length) || (read && brw >0));
    }
}

void end_pass(void* d)
{
  vw* all = ((gdmf*)d)->all;

   all->eta *= all->eta_decay_rate;
   if (all->save_per_pass)
     save_predictor(*all, all->final_regressor_name, all->current_pass);
   
   all->current_pass++;
}

  void learn(void* d, learner& base, example* ec)
  {
    vw* all = ((gdmf*)d)->all;
 
    mf_predict(*all,ec);
    if (all->training && ((label_data*)(ec->ld))->label != FLT_MAX)
      mf_inline_train(*all, ec, ec->eta_round);
  }

  learner* setup(vw& all)
  {
    gdmf* data = (gdmf*)calloc(1,sizeof(gdmf)); 
    data->all = &all;
    learner* l = new learner(data,learn, save_load, all.reg.stride);
    l->set_end_pass(end_pass);

    return l;
  }
}
