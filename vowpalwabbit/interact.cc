/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <sstream>
#include <float.h>
#include "reductions.h"
#include "v_array.h"

struct interact {
  unsigned char n1, n2;  //namespaces to interact
  v_array<feature> feat_store;
  vw *all;
  float n1_feat_sq;
  float total_sum_feat_sq;
  size_t num_features;
};

float multiply(v_array<feature>& f_dest, v_array<feature>& f_src2, interact& in) {
  f_dest.erase();
  v_array<feature> f_src1 = in.feat_store;
  vw* all = in.all;
  size_t weight_mask = all->reg.weight_mask;
  size_t base_id1 = f_src1[0].weight_index & weight_mask;
  size_t base_id2 = f_src2[0].weight_index & weight_mask;
  
  feature f;
  f.weight_index = f_src1[0].weight_index;
  f.x = f_src1[0].x*f_src2[0].x;
  float sum_sq = f.x*f.x;
  f_dest.push_back(f);

  for(size_t i1 = 1, i2 = 1; i1 < f_src1.size() && i2 < f_src2.size();) {
    size_t cur_id1 = (size_t)(((f_src1[i1].weight_index & weight_mask) - base_id1) & weight_mask);
    size_t cur_id2 = (size_t)(((f_src2[i2].weight_index & weight_mask) - base_id2) & weight_mask);

    if(cur_id1 == cur_id2) {
      feature f;
      f.weight_index = f_src1[i1].weight_index;
      f.x = f_src1[i1].x*f_src2[i2].x;
      sum_sq += f.x*f.x;
      f_dest.push_back(f);
      i1++;
      i2++;
    }
    else if(cur_id1 < cur_id2)
      i1++;
    else
      i2++;    
  }
  return sum_sq;
}

template <bool is_learn, bool print_all>
void predict_or_learn(interact& in, LEARNER::base_learner& base, example& ec) {
  v_array<feature>* f1 = &ec.atomics[in.n1];
  v_array<feature>* f2 = &ec.atomics[in.n2];

  in.num_features = ec.num_features;
  in.total_sum_feat_sq = ec.total_sum_feat_sq;
  in.n1_feat_sq = ec.sum_feat_sq[in.n1];
  ec.total_sum_feat_sq -= in.n1_feat_sq;
  ec.total_sum_feat_sq -= ec.sum_feat_sq[in.n2];
  ec.num_features -= f1->size();
  ec.num_features -= f2->size();
  
  in.feat_store.erase();
  push_many(in.feat_store, f1->begin, f1->size());
  
  ec.sum_feat_sq[in.n1] = multiply(*f1, *f2, in);
  ec.total_sum_feat_sq += ec.sum_feat_sq[in.n1];
  ec.num_features += f1->size();
  
  /*for(size_t i = 0;i < f1.size();i++)
    cout<<f1[i].weight_index<<":"<<f1[i].x<<" ";
    cout<<endl;*/
  
  // remove 2nd namespace
  int n2_i = -1;
  for (size_t i = 0; i < ec.indices.size(); i++) {
	  if (ec.indices[i] == in.n2) {
		  n2_i = (int)i;
		  memmove(&ec.indices[n2_i], &ec.indices[n2_i+1], sizeof(unsigned char) * (ec.indices.size() - n2_i - 1));
		  ec.indices.decr();
		  break;
	  } 
  }
  assert(n2_i >= 0);

  base.predict(ec);
  if(is_learn)
    base.learn(ec);
  
  // re-insert namespace into the right position
  ec.indices.incr();
  memmove(&ec.indices[n2_i + 1], &ec.indices[n2_i], sizeof(unsigned char) * (ec.indices.size() - n2_i - 1));
  ec.indices[n2_i] = in.n2;

  ec.atomics[in.n1].erase();
  push_many(ec.atomics[in.n1], in.feat_store.begin, in.feat_store.size());  
  ec.total_sum_feat_sq = in.total_sum_feat_sq;
  ec.sum_feat_sq[in.n1] = in.n1_feat_sq;
  ec.num_features = in.num_features;
}

void finish(interact& in) {in.feat_store.delete_v();}

LEARNER::base_learner* interact_setup(vw& all) 
{
  if(missing_option<string, true>(all, "interact", "Put weights on feature products from namespaces <n1> and <n2>"))
    return nullptr;
  string s = all.vm["interact"].as<string>();
  if(s.length() != 2) {
    cerr<<"Need two namespace arguments to interact!! EXITING\n";
    return nullptr;
  }
  
  interact& data = calloc_or_die<interact>();
  
  data.n1 = (unsigned char) s[0];
  data.n2 = (unsigned char) s[1];
  cout<<"Interacting namespaces "<<data.n1<<" and "<<data.n2<<endl;
  data.all = &all;

  LEARNER::learner<interact>* l;
  l = &LEARNER::init_learner(&data, setup_base(all), predict_or_learn<true, true>, predict_or_learn<false, true>, 1);

  l->set_finish(finish);
  return make_base(*l);
}
