/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#include <float.h>
#include <math.h>
#include <stdio.h>

#include "simple_label.h"
#include "cost_sensitive.h"
#include "reductions.h"

using namespace std;
using namespace LEARNER;

namespace WAP {
  struct wap{
    size_t increment; //wap does funky things with the increment, so we keep explicit access
    vw* all;
  };
  
  void mirror_features(vw& all, example& ec, uint32_t offset1, uint32_t offset2)
  {
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) 
      {
        size_t original_length = ec.atomics[*i].size();
        for (uint32_t j = 0; j < original_length; j++)
          {
            feature* f = &ec.atomics[*i][j];
            feature temp = {- f->x, f->weight_index + offset2};
            f->weight_index += offset1;
            ec.atomics[*i].push_back(temp);
          }
        ec.sum_feat_sq[*i] *= 2;
      }
    if (all.audit || all.hash_inv)
      {
        for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) 
          if (ec.audit_features[*i].begin != ec.audit_features[*i].end)
            {
              size_t original_length = ec.audit_features[*i].size();
              for (uint32_t j = 0; j < original_length; j++)
                {
                  audit_data* f = &ec.audit_features[*i][j];
                  char* new_space = NULL;
                  if (f->space != NULL)
                    {
                      new_space = (char*)calloc_or_die(strlen(f->space)+1,sizeof(char));
                      strcpy(new_space, f->space);
                    }
                  char* new_feature = (char*)calloc_or_die(strlen(f->feature)+2,sizeof(char));
                  strcpy(new_feature+1, f->feature);
                  *new_feature = '-';
                  audit_data temp = {new_space, new_feature, f->weight_index + offset2, - f->x, true};
                  f->weight_index += offset1;
                  ec.audit_features[*i].push_back(temp);
                }
            }
      }
    ec.num_features *= 2;
    ec.total_sum_feat_sq *= 2;
  }

  void unmirror_features(vw& all, example& ec, uint32_t offset1, uint32_t offset2)
  {
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) 
      {
        ec.atomics[*i].end = ec.atomics[*i].begin+ec.atomics[*i].size()/2;
        feature* end = ec.atomics[*i].end;
        for (feature* f = ec.atomics[*i].begin; f!= end; f++)
          f->weight_index -= offset1;
        ec.sum_feat_sq[*i] /= 2;
      }
    if (all.audit || all.hash_inv)
      {
        for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) 
          if (ec.audit_features[*i].begin != ec.audit_features[*i].end)
            {
              for (audit_data *f = ec.audit_features[*i].begin + ec.audit_features[*i].size()/2; f != ec.audit_features[*i].end; f++)
                {
                  if (f->space != NULL)
                    free(f->space);
                  free(f->feature);
                  f->alloced = false;
                }
              ec.audit_features[*i].end = ec.audit_features[*i].begin+ec.audit_features[*i].size()/2;
              for (audit_data *f = ec.audit_features[*i].begin; f != ec.audit_features[*i].end; f++)
                f->weight_index -= offset1;
            }
      }
    ec.num_features /= 2;
    ec.total_sum_feat_sq /= 2;
  }

  struct float_wclass
  {
    float v;
    COST_SENSITIVE::wclass ci;
  };
  int fi_compare(const void *e1, const void* e2)
  {
    float_wclass* fi1 = (float_wclass*)e1;
    float_wclass* fi2 = (float_wclass*)e2;
    if (fi1->ci.x > fi2->ci.x)
      return 1;
    else if (fi1->ci.x < fi2->ci.x)
      return -1;
    else
      return 0;
  }
  int fi_compare_i(const void *e1, const void* e2)
  {
    float_wclass* fi1 = (float_wclass*)e1;
    float_wclass* fi2 = (float_wclass*)e2;
    if (fi1->ci.weight_index > fi2->ci.weight_index)
      return 1;
    else if (fi1->ci.weight_index < fi2->ci.weight_index)
      return -1;
    else
      return 0;
  }
  v_array<float_wclass> vs;  // TODO: fix this since it's not re-entrant!

  void train(vw& all, wap& w, learner& base, example& ec)
  {
    COST_SENSITIVE::label* ld = (COST_SENSITIVE::label*)ec.ld;

    COST_SENSITIVE::wclass* old_end = ld->costs.end;
    COST_SENSITIVE::wclass* j = ld->costs.begin; 
    for (COST_SENSITIVE::wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      if (cl->x != FLT_MAX)
        *j++ = *cl;
    ld->costs.end = j;
  
    float score = FLT_MAX;
    vs.erase();
    for (COST_SENSITIVE::wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      {
        float_wclass temp = {0., *cl};
        if (temp.ci.x < score)
          score = temp.ci.x;
        vs.push_back(temp);
      }
  
    qsort(vs.begin, vs.size(), sizeof(float_wclass), fi_compare);
  
    for (uint32_t i = 0; i < ld->costs.size(); i++)
      {
        vs[i].ci.x -= score;
        if (i == 0)
          vs[i].v = 0.;
        else
          vs[i].v = vs[i-1].v + (vs[i].ci.x-vs[i-1].ci.x) / (float)i;
      }
  
    qsort(vs.begin, vs.size(), sizeof(float_wclass), fi_compare_i);

    for (uint32_t i = 0; i < ld->costs.size(); i++)
      for (uint32_t j = i+1; j < ld->costs.size(); j++)
        {
          label_data simple_temp;
          simple_temp.weight = fabsf(vs[i].v - vs[j].v);
          if (simple_temp.weight > 1e-5)
            {
              simple_temp.initial = 0.;
	    
              if (vs[i].v < vs[j].v)
                simple_temp.label = 1;
              else
                simple_temp.label = -1;
	    
              ec.ld = &simple_temp;
	    
              ec.partial_prediction = 0.;
              uint32_t myi = (uint32_t)vs[i].ci.weight_index;
              uint32_t myj = (uint32_t)vs[j].ci.weight_index;

              mirror_features(all, ec, (uint32_t)((myi-1)*w.increment), (uint32_t)((myj-1)*w.increment));

              base.learn(ec);
              unmirror_features(all, ec, (uint32_t)((myi-1)*w.increment), (uint32_t)((myj-1)*w.increment));
            }
        }

    ld->costs.end = old_end;
    ec.ld = ld;
  }

  size_t test(vw& all, wap& w, learner& base, example& ec)
  {
    size_t prediction = 1;
    float score = -FLT_MAX;
  
    COST_SENSITIVE::label* cost_label = (COST_SENSITIVE::label*)ec.ld; 

    for (uint32_t i = 0; i < cost_label->costs.size(); i++)
      {
        label_data simple_temp;
        simple_temp.initial = 0.;
        simple_temp.weight = 0.;
        simple_temp.label = FLT_MAX;
        uint32_t myi = (uint32_t)cost_label->costs[i].weight_index;
        ec.ld = &simple_temp;
        base.predict(ec, myi-1);
        if (ec.partial_prediction > score)
          {
            score = ec.partial_prediction;
            prediction = myi;
          }
        cost_label->costs[i].partial_prediction = -ec.partial_prediction;
      }
    return prediction;
  }

  template <bool is_learn>
  void predict_or_learn(wap& w, learner& base, example& ec)
  {
    COST_SENSITIVE::label* cost_label = (COST_SENSITIVE::label*)ec.ld;
    vw* all = w.all;
    
    size_t prediction = test(*all, w, base, ec);
    ec.ld = cost_label;
    
    if (is_learn && cost_label->costs.size() > 0)
      train(*all, w, base, ec);
    ec.final_prediction = (float)prediction;
  }

  void finish_example(vw& all, wap&, example& ec)
  {
    COST_SENSITIVE::output_example(all, ec);
    VW::finish_example(all, &ec);
  }
  
  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file)
  {
    wap* w=(wap*)calloc_or_die(1,sizeof(wap));
    w->all = &all;
    uint32_t nb_actions = 0;
    if( vm_file.count("wap") ) { //if loaded options from regressor
      nb_actions = (uint32_t)vm_file["wap"].as<size_t>();
      if( vm.count("wap") && (uint32_t)vm["wap"].as<size_t>() != nb_actions )
        std::cerr << "warning: you specified a different number of actions through --wap than the one loaded from regressor. Pursuing with loaded value of: " << nb_actions << endl;
    }
    else {
      nb_actions = (uint32_t)vm["wap"].as<size_t>();

     //append wap with nb_actions to options_from_file so it is saved to regressor later
     std::stringstream ss;
     ss << " --wap " << nb_actions;
     all.options_from_file.append(ss.str());
    }

    all.p->lp = COST_SENSITIVE::cs_label_parser;

    all.sd->k = (uint32_t)nb_actions;

    learner* l = new learner(w, all.l, nb_actions);
    l->set_learn<wap, predict_or_learn<true> >();
    l->set_predict<wap, predict_or_learn<false> >();
    l->set_finish_example<wap,finish_example>();
    w->increment = l->increment;

    return l;
  }
}
