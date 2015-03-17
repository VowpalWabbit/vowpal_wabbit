/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <sstream>
#include <float.h>
#include "reductions.h"

struct oaa{
  size_t k;
  vw* all; // for raw
};

template <bool is_learn, bool print_all>
void predict_or_learn(oaa& o, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t mc_label_data = ec.l.multi;
  if (mc_label_data.label == 0 || (mc_label_data.label > o.k && mc_label_data.label != (uint32_t)-1))
    cout << "label " << mc_label_data.label << " is not in {1,"<< o.k << "} This won't work right." << endl;

  stringstream outputStringStream;
  uint32_t prediction = 1;

  ec.l.simple = {FLT_MAX, mc_label_data.weight, 0.f};
  float score = INT_MIN;
  for (uint32_t i = 1; i <= o.k; i++) {
    if (is_learn) {
      ec.l.simple.label = (mc_label_data.label == i) ? 1.f : -1.f;
      base.learn(ec, i-1);
    } else
      base.predict(ec, i-1);
        
    if (ec.partial_prediction > score) {
      score = ec.partial_prediction;
      prediction = i;
    }
      
    if (print_all) {
      if (i > 1) outputStringStream << ' ';
      outputStringStream << i << ':' << ec.partial_prediction;
    }
  }

  ec.pred.multiclass = prediction;
  ec.l.multi = mc_label_data;
  
  if (print_all) 
    o.all->print_text(o.all->raw_prediction, outputStringStream.str(), ec.tag);
}

LEARNER::base_learner* oaa_setup(vw& all)
{
  if (missing_option<size_t, true>(all, "oaa", "One-against-all multiclass with <k> labels")) 
    return nullptr;
  
  oaa& data = calloc_or_die<oaa>();
  data.k = all.vm["oaa"].as<size_t>();
  data.all = &all;
  
  LEARNER::learner<oaa>* l;
  if (all.raw_prediction > 0)
    l = &LEARNER::init_multiclass_learner(&data, setup_base(all), predict_or_learn<true, true>, 
					  predict_or_learn<false, true>, all.p, data.k);
  else
    l = &LEARNER::init_multiclass_learner(&data, setup_base(all),predict_or_learn<true, false>, 
					  predict_or_learn<false, false>, all.p, data.k);
    
  return make_base(*l);
}
