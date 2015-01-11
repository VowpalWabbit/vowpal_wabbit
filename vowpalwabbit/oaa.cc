/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <sstream>
#include "reductions.h"

struct oaa{
  size_t k;
  bool shouldOutput;
  vw* all;
};

template <bool is_learn>
void predict_or_learn(oaa& o, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t mc_label_data = ec.l.multi;
  if (mc_label_data.label == 0 || (mc_label_data.label > o.k && mc_label_data.label != (uint32_t)-1))
    cout << "label " << mc_label_data.label << " is not in {1,"<< o.k << "} This won't work right." << endl;
  
  ec.l.simple = {0.f, mc_label_data.weight, 0.f};
  stringstream outputStringStream;
  uint32_t prediction = 1;
  float score = INT_MIN;
  for (uint32_t i = 1; i <= o.k; i++)
    {
      if (is_learn)
	{
	  if (mc_label_data.label == i)
	    ec.l.simple.label = 1;
	  else
	    ec.l.simple.label = -1;
	  
	  base.learn(ec, i-1);
	}
      else
	base.predict(ec, i-1);
      
      if (ec.partial_prediction > score)
	{
	  score = ec.partial_prediction;
	  prediction = i;
	}
      
      if (o.shouldOutput) {
	if (i > 1) outputStringStream << ' ';
	outputStringStream << i << ':' << ec.partial_prediction;
      }
    }	
  ec.pred.multiclass = prediction;
  ec.l.multi = mc_label_data;
  
  if (o.shouldOutput) 
    o.all->print_text(o.all->raw_prediction, outputStringStream.str(), ec.tag);
}

LEARNER::base_learner* oaa_setup(vw& all)
{
  if (missing_option<size_t, true>(all, "oaa", "One-against-all multiclass with <k> labels")) 
    return NULL;
  
  oaa& data = calloc_or_die<oaa>();
  data.k = all.vm["oaa"].as<size_t>();
  data.shouldOutput = all.raw_prediction > 0;
  data.all = &all;
  
  LEARNER::learner<oaa>& l = 
    LEARNER::init_multiclass_learner(&data, setup_base(all), predict_or_learn<true>, 
				     predict_or_learn<false>, all.p, data.k);
  return make_base(l);
}
