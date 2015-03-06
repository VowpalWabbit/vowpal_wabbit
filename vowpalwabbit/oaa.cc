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
  polyprediction* pred;  // for multipredict
  polylabel* label; // for multipredict
};

template <bool is_learn, bool print_all>
void predict_or_learn(oaa& o, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t mc_label_data = ec.l.multi;
  if (mc_label_data.label == 0 || (mc_label_data.label > o.k && mc_label_data.label != (uint32_t)-1))
    cout << "label " << mc_label_data.label << " is not in {1,"<< o.k << "} This won't work right." << endl;

  stringstream outputStringStream;
  uint32_t prediction = 1;

  ec.l.simple = { FLT_MAX, mc_label_data.weight, 0.f };
  //if (!is_learn) {
  base.multipredict(ec, 0, o.k, o.pred);
  for (uint32_t i=2; i<=o.k; i++)
    if (o.pred[i-1].scalar > o.pred[prediction-1].scalar)
      prediction = i;
  //}
  
  if (is_learn) {
    for (uint32_t i=1; i<=o.k; i++)
      o.label[i-1].simple = { (mc_label_data.label == i) ? 1.f : -1.f, mc_label_data.weight, 0.f };
    base.multiupdate(ec, 0, o.k, o.pred, o.label);
  } 
  /*
  if (is_learn) {
    //float score = INT_MIN;
    for (uint32_t i = 1; i <= o.k; i++) {
      ec.l.simple.label = (mc_label_data.label == i) ? 1.f : -1.f;
      ec.pred.scalar = o.pred[i-1].scalar;
      base.update(ec, i-1);
      //if (ec.pred.scalar > score) { score = ec.pred.scalar; prediction = i; }
    }
    }*/

  if (print_all) {
    outputStringStream << "1:" << o.pred[0].scalar;
    for (uint32_t i=2; i<=o.k; i++) outputStringStream << ' ' << i << ':' << o.pred[i-1].scalar;
    o.all->print_text(o.all->raw_prediction, outputStringStream.str(), ec.tag);
  }
  
  ec.pred.multiclass = prediction;
  ec.l.multi = mc_label_data;
}

void finish(oaa&o) { free(o.pred); free(o.label); }

LEARNER::base_learner* oaa_setup(vw& all)
{
  if (missing_option<size_t, true>(all, "oaa", "One-against-all multiclass with <k> labels")) 
    return NULL;
  
  oaa& data = calloc_or_die<oaa>();
  data.k = all.vm["oaa"].as<size_t>();
  data.all = &all;
  data.pred = calloc_or_die<polyprediction>(data.k);
  data.label = calloc_or_die<polylabel>(data.k);
  
  LEARNER::learner<oaa>* l;
  if (all.raw_prediction > 0)
    l = &LEARNER::init_multiclass_learner(&data, setup_base(all), predict_or_learn<true, true>, 
					  predict_or_learn<false, true>, all.p, data.k);
  else
    l = &LEARNER::init_multiclass_learner(&data, setup_base(all),predict_or_learn<true, false>, 
					  predict_or_learn<false, false>, all.p, data.k);
  l->set_finish(finish);
  
  return make_base(*l);
}
