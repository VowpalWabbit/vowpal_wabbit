/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <sstream>
#include <float.h>
#include <math.h>
#include "reductions.h"
#include "rand48.h"
#include "vw_exception.h"
#include "vw.h"

struct oaa {
  size_t k;
  vw* all; // for raw
  polyprediction* pred;  // for multipredict
  size_t num_subsample; // for randomized subsampling, how many negatives to draw?
  uint32_t* subsample_order; // for randomized subsampling, in what order should we touch classes
  size_t subsample_id; // for randomized subsampling, where do we live in the list
  float* probs;
};

void learn_randomized(oaa& o, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t ld = ec.l.multi;
  if (ld.label == 0 || (ld.label > o.k && ld.label != (uint32_t)-1))
    cout << "label " << ld.label << " is not in {1,"<< o.k << "} This won't work right." << endl;

  stringstream outputStringStream;

  ec.l.simple = { 1., ld.weight, 0.f }; // truth
  base.learn(ec, ld.label-1);

  size_t prediction = ld.label;
  float best_partial_prediction = ec.partial_prediction;

  ec.l.simple.label = -1.;
  ec.l.simple.weight *= ((float)o.k) / (float)o.num_subsample;
  size_t p = o.subsample_id;
  size_t count = 0;
  while (count < o.num_subsample) {
    uint32_t l = o.subsample_order[p];
    p = (p+1) % o.k;
    if (l == ld.label-1) continue;
    base.learn(ec, l);
    if (ec.partial_prediction > best_partial_prediction) {
      best_partial_prediction = ec.partial_prediction;
      prediction = l+1;
    }
    count++;
  }
  o.subsample_id = p;

  ec.pred.multiclass = prediction;
  ec.l.multi = ld;
}

template <bool is_learn, bool print_all, bool is_probabilities>
void predict_or_learn(oaa& o, LEARNER::base_learner& base, example& ec) {
  MULTICLASS::label_t mc_label_data = ec.l.multi;
  if (mc_label_data.label == 0 || (mc_label_data.label > o.k && mc_label_data.label != (uint32_t)-1))
    cout << "label " << mc_label_data.label << " is not in {1,"<< o.k << "} This won't work right." << endl;

  stringstream outputStringStream;
  uint32_t prediction = 1;

  ec.l.simple = { FLT_MAX, mc_label_data.weight, 0.f };
  base.multipredict(ec, 0, o.k, o.pred, true);
  for (uint32_t i=2; i<=o.k; i++)
    if (o.pred[i-1].scalar > o.pred[prediction-1].scalar)
      prediction = i;

  if (is_learn) {
    for (uint32_t i=1; i<=o.k; i++) {
      ec.l.simple = { (mc_label_data.label == i) ? 1.f : -1.f, mc_label_data.weight, 0.f };
      ec.pred.scalar = o.pred[i-1].scalar;
      base.update(ec, i-1);
    }
  }

  if (print_all) {
    outputStringStream << "1:" << o.pred[0].scalar;
    for (uint32_t i=2; i<=o.k; i++) outputStringStream << ' ' << i << ':' << o.pred[i-1].scalar;
    o.all->print_text(o.all->raw_prediction, outputStringStream.str(), ec.tag);
  }

  if (is_probabilities) {
    float sum_prob = 0;
    for (uint32_t i=0; i<o.k; i++) {
      // probability of class (i+1) = logistic_link_function(raw_prediction)
      float prob = 1.f / (1.f + exp(- o.pred[i].scalar));
      o.probs[i] = prob;
      sum_prob += prob;
    }
    // make sure that the probabilities sum up (exactly) to one
    for (uint32_t i=0; i<o.k; i++)
      o.probs[i] /= sum_prob;
    // copy probs pointer to the example.
    // TODO: shouldn't we do a deep copy and free(ec.pred.probs) in finish_example()?
    ec.probs = o.probs;
  }

  ec.pred.multiclass = prediction;
  ec.l.multi = mc_label_data;
}

void finish(oaa&o) {
  free(o.pred);
  free(o.subsample_order);
  free(o.probs);
}

// TODO: partial code duplication with multiclass.cc:finish_example
void finish_example_probabilities(vw& all, oaa& o, example& ec) {
  float zero_one_loss = 0;
  if (ec.l.multi.label != (uint32_t)ec.pred.multiclass)
    zero_one_loss = ec.l.multi.weight;
  // TODO:
  // What to do if the correct label is unknown, i.e. (uint32_t)-1?
  //   Suggestion: increase all.sd->weighted_unlabeled_examples???,
  //               but not sd.example_number, so the average loss is not influenced.
  // What to do if the correct_class_prob==0?
  //   Suggestion: have some maximal multiclass_log_loss limit, e.g. 999.
  float multiclass_log_loss = 999; // -log(0) = plus infinity
  float correct_class_prob = 0;
  if (ec.l.multi.label <= o.k) // prevent segmentation fault if labeĺ==(uint32_t)-1
    correct_class_prob = ec.probs[ec.l.multi.label-1];
  if (correct_class_prob > 0)
    multiclass_log_loss = -log(correct_class_prob) * ec.l.multi.weight;

  all.sd->update(ec.test_only, zero_one_loss, ec.l.multi.weight, ec.num_features);
  //all.sd->update(ec.test_only, multiclass_log_loss, ec.l.multi.weight, ec.num_features);

  if (ec.test_only){
    all.sd->holdout_multiclass_log_loss += multiclass_log_loss;
  } else {
    all.sd->multiclass_log_loss += multiclass_log_loss;
  }

  char temp_str[10];
  ostringstream outputStringStream;
  for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++) {
    for (size_t i = 0; i < o.k; i++) {
      if (i > 0) outputStringStream << ' ';
      if (all.sd->ldict) {
        substring ss = all.sd->ldict->get(i+1);
        outputStringStream << string(ss.begin, ss.end - ss.begin);
      } else {
        outputStringStream << i+1;
      }
      sprintf(temp_str, "%f", ec.probs[i]); // 0.123 -> 0.123000
      outputStringStream << ':' << temp_str;
    }
    all.print_text(*sink, outputStringStream.str(), ec.tag);
  }

  MULTICLASS::print_update(all, ec);
  VW::finish_example(all, &ec);
}


LEARNER::base_learner* oaa_setup(vw& all)
{
  if (missing_option<size_t, true>(all, "oaa", "One-against-all multiclass with <k> labels"))
    return nullptr;
  new_options(all, "oaa options")
  ("oaa_subsample", po::value<size_t>(), "subsample this number of negative examples when learning");
  add_options(all);

  oaa* data_ptr = calloc_or_die<oaa>(1);
  oaa& data = *data_ptr;
  data.k = all.vm["oaa"].as<size_t>(); // number of classes
  data.probs = calloc_or_die<float>(data.k);

  if (all.sd->ldict && (data.k != all.sd->ldict->getK()))
    THROW("error: you have " << all.sd->ldict->getK() << " named labels; use that as the argument to oaa");

  data.all = &all;
  data.pred = calloc_or_die<polyprediction>(data.k);
  data.num_subsample = 0;
  data.subsample_order = nullptr;
  data.subsample_id = 0;
  if (all.vm.count("oaa_subsample")) {
    data.num_subsample = all.vm["oaa_subsample"].as<size_t>();
    if (data.num_subsample >= data.k) {
      data.num_subsample = 0;
      cerr << "oaa is turning off subsampling because your parameter >= K" << endl;
    } else {
      data.subsample_order = calloc_or_die<uint32_t>(data.k);
      for (size_t i=0; i<data.k; i++) data.subsample_order[i] = (uint32_t) i;
      for (size_t i=0; i<data.k; i++) {
        size_t j = (size_t)(frand48() * (float)(data.k-i)) + i;
        uint32_t tmp = data.subsample_order[i];
        data.subsample_order[i] = data.subsample_order[j];
        data.subsample_order[j] = tmp;
      }
    }
  }

  LEARNER::learner<oaa>* l;
  // the three boolean template parameters are: is_learn, print_all and is_probabilities
  if (all.probabilities) {
    l = &LEARNER::init_multiclass_learner(data_ptr, setup_base(all), predict_or_learn<true, false, true>,
                                          predict_or_learn<false, false, true>, all.p, data.k);
    l->set_finish_example(finish_example_probabilities);
  } else if (all.raw_prediction > 0) {
    l = &LEARNER::init_multiclass_learner(data_ptr, setup_base(all), predict_or_learn<true, true, false>,
                                          predict_or_learn<false, true, false>, all.p, data.k);
  } else {
    l = &LEARNER::init_multiclass_learner(data_ptr, setup_base(all),predict_or_learn<true, false, false>,
                                          predict_or_learn<false, false, false>, all.p, data.k);
  }

  if (data.num_subsample > 0)
    l->set_learn(learn_randomized);
  l->set_finish(finish);

  return make_base(*l);
}
