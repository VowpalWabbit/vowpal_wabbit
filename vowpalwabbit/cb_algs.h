/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef CB_ALGS_H
#define CB_ALGS_H

//TODO: extend to handle CSOAA_LDF and WAP_LDF
namespace CB_ALGS {

  LEARNER::learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);

  template <bool is_learn>
    float get_cost_pred(vw& all, CB::cb_class* known_cost, example& ec, uint32_t index, uint32_t base)
  {
    CB::label* ld = (CB::label*)ec.ld;

    label_data simple_temp;
    simple_temp.initial = 0.;
    if (known_cost != NULL && index == known_cost->action)
      {
	simple_temp.label = known_cost->cost;
	simple_temp.weight = 1.;
      }
    else 
      {
	simple_temp.label = FLT_MAX;
	simple_temp.weight = 0.;
      }
    
    ec.ld = &simple_temp;

    if (is_learn)
      all.scorer->learn(ec, index-1+base);
    else
      all.scorer->predict(ec, index-1+base);
    ec.ld = ld;

    return ec.final_prediction;
  }
}

#endif
