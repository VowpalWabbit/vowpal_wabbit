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

using namespace std;
using namespace LEARNER;
using namespace MULTICLASS;

namespace OAA {

  struct oaa{
    uint32_t k;
    bool shouldOutput;
    vw* all;
  };

  template <bool is_learn>
  void predict_or_learn(oaa& o, learner& base, example& ec) {
    vw* all = o.all;

    mc_label* mc_label_data = (mc_label*)ec.ld;
    float prediction = 1;
    float score = INT_MIN;
  
    if (mc_label_data->label == 0 || (mc_label_data->label > o.k && mc_label_data->label != (uint32_t)-1))
      cout << "label " << mc_label_data->label << " is not in {1,"<< o.k << "} This won't work right." << endl;
    
    string outputString;
    stringstream outputStringStream(outputString);

    label_data simple_temp;
    simple_temp.initial = 0.;
    simple_temp.weight = mc_label_data->weight;
    ec.ld = &simple_temp;

    for (size_t i = 1; i <= o.k; i++)
      {
	if (is_learn)
	  {
	    if (mc_label_data->label == i)
	      simple_temp.label = 1;
	    else
	      simple_temp.label = -1;

	    base.learn(ec, i-1);
	  }
	else
	  base.predict(ec, i-1);

        if (ec.partial_prediction > score)
          {
            score = ec.partial_prediction;
            prediction = (float)i;
          }
	
        if (o.shouldOutput) {
          if (i > 1) outputStringStream << ' ';
          outputStringStream << i << ':' << ec.partial_prediction;
        }
      }	
    ec.ld = mc_label_data;
    ec.final_prediction = prediction;
    
    if (o.shouldOutput) 
      all->print_text(all->raw_prediction, outputStringStream.str(), ec.tag);
  }
  
  void finish_example(vw& all, oaa&, example& ec)
  {
    MULTICLASS::output_example(all, ec);
    VW::finish_example(all, &ec);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    oaa* data = (oaa*)calloc_or_die(1, sizeof(oaa));
    //first parse for number of actions
    if( vm_file.count("oaa") ) {
      data->k = (uint32_t)vm_file["oaa"].as<size_t>();
      if( vm.count("oaa") && (uint32_t)vm["oaa"].as<size_t>() != data->k )
        std::cerr << "warning: you specified a different number of actions through --oaa than the one loaded from predictor. Pursuing with loaded value of: " << data->k << endl;
    }
    else {
      data->k = (uint32_t)vm["oaa"].as<size_t>();

      //append oaa with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --oaa " << data->k;
      all.options_from_file.append(ss.str());
    }


    data->shouldOutput = all.raw_prediction > 0;
    data->all = &all;
    all.p->lp = mc_label_parser;

    learner* l = new learner(data, all.l, data->k);
    l->set_learn<oaa, predict_or_learn<true> >();
    l->set_predict<oaa, predict_or_learn<false> >();
    l->set_finish_example<oaa, finish_example>();

    return l;
  }
}
