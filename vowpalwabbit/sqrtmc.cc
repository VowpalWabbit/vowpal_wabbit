/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <sstream>
#include "reductions.h"
#include "gd.h"

struct sqrtmc{
  uint32_t k, layer1, layer2;
  vw* all;
  polyprediction* pred;
};

// this spreads heavy hitters out
template <bool is_learn, bool print_all>
void predict_or_learn(sqrtmc& mc, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t ld = ec.l.multi;
  if (ld.label == 0 || (ld.label > mc.k && ld.label != (uint32_t)-1))
    cout << "label " << ld.label << " is not in {1,"<< mc.k << "} This won't work right." << endl;

  uint32_t trueL = ld.label - 1;
  uint32_t true0 = trueL % mc.layer1;

  base.multipredict(ec, 0, mc.layer1, mc.pred);
  uint32_t pred0 = 0;
  for (uint32_t i=1; i<mc.layer1; i++)
    if (mc.pred[i].scalar > mc.pred[pred0].scalar)
      pred0 = i;

  uint32_t top = min(mc.layer2, mc.k - mc.layer2 * pred0 + 1);
  
  base.multipredict(ec, mc.layer1 + mc.layer2 * pred0, top, mc.pred);
  uint32_t pred1 = 0;
  for (uint32_t i=1; i<top; i++)
    if (mc.pred[i].scalar > mc.pred[pred1].scalar)
      pred1 = i;

  if (is_learn) {
    ec.l.simple = { 0.f, ld.weight, 0.f };
    for (uint32_t i=0; i<mc.layer1; i++) {
      ec.l.simple.label = (i == true0) ? 1. : -1.;
      base.learn(ec, i);
    }

    top = min(mc.layer2, mc.k - mc.layer2 * true0 + 1);
    for (uint32_t i=0; i<top; i++) {
      ec.l.simple.label = (true0 + mc.layer1*i == trueL) ? 1. : -1.;
      base.learn(ec, mc.layer1 + mc.layer2 * true0 + i);
    }
  }

  ec.pred.multiclass = pred0 + mc.layer1 * pred1 + 1;
  ec.l.multi = ld;
}

/*
// this puts all the heavy hitters in the same bucket -- this seems worse
template <bool is_learn, bool print_all>
void predict_or_learn(sqrtmc& mc, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t ld = ec.l.multi;
  if (ld.label == 0 || (ld.label > mc.k && ld.label != (uint32_t)-1))
    cout << "label " << ld.label << " is not in {1,"<< mc.k << "} This won't work right." << endl;

  uint32_t trueL = ld.label - 1;
  uint32_t true0 = trueL / mc.layer2;

  base.multipredict(ec, 0, mc.layer1, mc.pred);
  uint32_t pred0 = 0;
  for (uint32_t i=1; i<mc.layer1; i++)
    if (mc.pred[i].scalar > mc.pred[pred0].scalar)
      pred0 = i;

  uint32_t top = mc.layer2; // min(mc.layer2, mc.k - mc.layer2 * pred0 + 1);
  
  base.multipredict(ec, mc.layer1 + mc.layer2 * pred0, top, mc.pred);
  uint32_t pred1 = 0;
  for (uint32_t i=1; i<top; i++)
    if (mc.pred[i].scalar > mc.pred[pred1].scalar)
      pred1 = i;

  if (is_learn) {
    ec.l.simple = { 0.f, ld.weight, 0.f };
    for (uint32_t i=0; i<mc.layer1; i++) {
      ec.l.simple.label = (i == true0) ? 1. : -1.;
      base.learn(ec, i);
    }

    //top = min(mc.layer2, mc.k - mc.layer2 * true0 + 1);
    for (uint32_t i=0; i<top; i++) {
      ec.l.simple.label = (true0 * mc.layer2 + i == trueL) ? 1. : -1.;
      base.learn(ec, mc.layer1 + mc.layer2 * true0 + i);
    }
  }

  ec.pred.multiclass = pred0 * mc.layer2 + pred1 + 1;
  ec.l.multi = ld;
}
*/

LEARNER::base_learner* sqrtmc_setup(vw& all)
{
  if (missing_option<size_t, true>(all, "sqrtmc", "Square-root multiclass with <k> labels")) 
    return NULL;
  new_options(all, "sqrtmc options")
      ("sqrtmc_layer_size", po::value<size_t>(), "override the size of the first layer [def: ceil(sqrt(K))]");
  add_options(all);
  
  sqrtmc& data = calloc_or_die<sqrtmc>();
  data.k = all.vm["sqrtmc"].as<size_t>();
  data.layer1 = (uint32_t)ceilf(sqrt(data.k));
  if (all.vm.count("sqrtmc_layer_size"))
    data.layer1 = (uint32_t)all.vm["sqrtmc_layer_size"].as<size_t>();  
  data.layer2 = (uint32_t)ceilf((float)data.k / (float)data.layer1);
  data.all = &all;
  data.pred = calloc_or_die<polyprediction>(max(data.layer1, data.layer2));
  
  // # of params is layer1 to make first decision
  // then for each of those there are layer2 classes
  // so total is layer1 + layer1*layer2 = layer1(layer2 + 1)
  
  LEARNER::learner<sqrtmc>* l;
  if (all.raw_prediction > 0)
    l = &LEARNER::init_multiclass_learner(&data, setup_base(all), predict_or_learn<true, true>, 
					  predict_or_learn<false, true>, all.p, data.layer1 * (data.layer2 + 1));
  else
    l = &LEARNER::init_multiclass_learner(&data, setup_base(all),predict_or_learn<true, false>, 
					  predict_or_learn<false, false>, all.p, data.layer1 * (data.layer2 + 1));
    
  return make_base(*l);
}

/*
versus OAA:
2003 classes, 31489586 features, sqrt(2003)=45

 10: 0.760607   9m26s
 20: 0.736388   5m58s
 40: 0.728325   5m37s
 80: 0.717287   4m22s
160: 0.714768   6m42s
320: 0.711064   9m20s
640: 0.710312  16m57s
OAA: 0.706174  52m21s

echo OAA; ( time ../vowpal_wabbit/vowpalwabbit/vw -b29 -d wsj.vw2k.gz --passes 5 -k -c --oaa 2003 --ngram 6 --early_terminate 100 ) 2>&1 | egrep '^average loss|^real'
for size in 10 20 40 80 160 320 640 ; do
  echo $size
  ( time ../vowpal_wabbit/vowpalwabbit/vw -b29 -d wsj.vw2k.gz --passes 5 -k -c --sqrtmc 2003 --sqrtmc_layer_size $size --ngram 6 --early_terminate 100 ) 2>&1 | egrep '^average loss|^real'
done

get confusion matrix for 80 (fastest)

../vowpal_wabbit/vowpalwabbit/vw -b29 -d wsj.vw2k.tr.gz --passes 5 -k -c --sqrtmc 2003 --sqrtmc_layer_size 80 --ngram 6 --early_terminate 100 -f wsj.vw2k.tr.model

*/
