/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "searn.h"
#include "gd.h"
#include "parser.h"
#include "constant.h"
#include "oaa.h"
#include "searn_sequencetask.h"
#include "vw.h"

namespace SequenceTask {
  using namespace Searn;

  void initialize(vw& vw, searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    srn.task_data = NULL;
    srn.auto_history = true;
  }

  void finish(vw& vw, searn& srn) { }

  void get_oracle_labels(example*ec, v_array<uint32_t>*out) {
    out->erase();
    if (! OAA::example_is_test(ec))
      out->push_back( ((OAA::mc_label*)ec->ld)->label );
  }

  void structured_predict_v1(vw& vw, searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) { // TODO: get rid of vw
    float total_loss  = 0;

    v_array<uint32_t> ystar;
    for (size_t i=0; i<len; i++) {
      srn.snapshot(vw, i, 1, &i, sizeof(i), true);
      srn.snapshot(vw, i, 2, &total_loss, sizeof(total_loss), false);

      get_oracle_labels(ec[i], &ystar);

      size_t prediction = srn.predict(vw, &ec[i], 0, NULL, &ystar);

      if (ystar.size() > 0)
        total_loss += (float)(prediction != ystar[0]);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << ((ystar.size() == 0) ? '?' : ystar[0]) << ' ';
    }
    srn.declare_loss(vw, len, total_loss);

    ystar.erase();  ystar.delete_v();
  }
}

/* OLD VERSION THAT EXPLICITLY DOES HISTORY
namespace SequenceTask {
  using namespace Searn;

  struct sequencetask_data {
    SearnUtil::history_info hinfo;
    v_array<size_t> yhat;
  };

  void initialize(vw& vw, searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    sequencetask_data* dat = new sequencetask_data();
    srn.task_data = dat;

    SearnUtil::default_info(&dat->hinfo);

    po::options_description desc("Searn[sequence] options");
    desc.add_options()
      ("searn_sequencetask_history",  po::value<size_t>(), "length of history to use")
      ("searn_sequencetask_features", po::value<size_t>(), "length of history to pair with observed features")
      ("searn_sequencetask_bigrams",                       "use bigrams from history")
      ("searn_sequencetask_bigram_features",               "use bigrams from history paired with observed features");

    setup_searn_options(desc, vw, opts, vm, vm_file);
    
    check_option<size_t>(dat->hinfo.length, vw, vm, vm_file, "searn_sequencetask_history", false, size_equal,
                         "warning: you specified a different value for --searn_sequencetask_history than the one loaded from regressor. proceeding with loaded value: ", "");
    
    check_option<size_t>(dat->hinfo.features, vw, vm, vm_file, "searn_sequencetask_features", false, size_equal,
                         "warning: you specified a different value for --searn_sequencetask_features than the one loaded from regressor. proceeding with loaded value: ", "");
    
    check_option        (dat->hinfo.bigrams, vw, vm, vm_file, "searn_sequencetask_bigrams", false,
                         "warning: you specified --searn_sequencetask_bigrams but that wasn't loaded from regressor. proceeding with loaded value: ");
    
    check_option        (dat->hinfo.bigram_features, vw, vm, vm_file, "searn_sequencetask_bigram_features", false,
                         "warning: you specified --searn_sequencetask_bigram_features but that wasn't loaded from regressor. proceeding with loaded value: ");

  }

  void finish(vw& vw, searn& srn) {
    sequencetask_data* dat = (sequencetask_data*)srn.task_data;
    dat->yhat.erase();
    dat->yhat.delete_v();
    delete dat;
  }

  void get_oracle_labels(example*ec, v_array<uint32_t>*out) {
    out->erase();
    if (! OAA::example_is_test(ec))
      out->push_back( ((OAA::mc_label*)ec->ld)->label );
  }

  void structured_predict_v1(vw& vw, searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    sequencetask_data* dat = (sequencetask_data*)srn.task_data;
    float total_loss  = 0;
    size_t history_length = max(dat->hinfo.features, dat->hinfo.length);

    dat->yhat.erase();
    dat->yhat.resize(history_length + len, true); // pad the beginning with zeros for <s>

    v_array<uint32_t> ystar;
    for (size_t i=0; i<len; i++) {
      srn.snapshot(vw, i, 1, &i, sizeof(i), true);
      srn.snapshot(vw, i, 2, dat->yhat.begin+i, sizeof(uint32_t)*history_length, true);
      srn.snapshot(vw, i, 3, &total_loss, sizeof(total_loss), false);

      get_oracle_labels(ec[i], &ystar);

      size_t prediction = SearnUtil::predict_with_history(vw, ec[i], &ystar, dat->hinfo, dat->yhat.begin+i);
      dat->yhat[i+history_length] = prediction;

      if (ystar.size() > 0)
        total_loss += (float)(prediction != ystar[0]);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << ((ystar.size() == 0) ? '?' : ystar[0]) << ' ';
    }
      
    ystar.erase();  ystar.delete_v();
    srn.declare_loss(vw, len, total_loss);
  }
}
*/
