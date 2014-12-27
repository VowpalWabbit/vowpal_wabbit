/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "multiclass.h"
#include "simple_label.h"
#include "reductions.h"
#include "vw.h"

using namespace std;
using namespace LEARNER;
using namespace MULTICLASS;

namespace OAA {
  struct oaa{
    size_t k;
    bool shouldOutput;
    vw* all;
  };

  template <bool is_learn>
  void predict_or_learn(oaa& o, base_learner& base, example& ec) {
    multiclass mc_label_data = ec.l.multi;
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
  
  void finish_example(vw& all, oaa&, example& ec)
  {
    MULTICLASS::output_example(all, ec);
    VW::finish_example(all, &ec);
  }

  base_learner* setup(vw& all, po::variables_map& vm)
  {
    oaa& data = calloc_or_die<oaa>();
    //first parse for number of actions
    data.k = vm["oaa"].as<size_t>();
    //append oaa with nb_actions to options_from_file so it is saved to regressor later
    all.file_options << " --oaa " << data.k;

    data.shouldOutput = all.raw_prediction > 0;
    data.all = &all;
    all.p->lp = mc_label;

    learner<oaa>& l = init_learner(&data, all.l, data.k);
    l.set_learn(predict_or_learn<true>);
    l.set_predict(predict_or_learn<false>);
    l.set_finish_example(finish_example);

    return make_base(l);
  }
}
