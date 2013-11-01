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

  void structured_predict_v1(vw& vw, searn& srn, learner& base, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) { // TODO: get rid of vw
    float total_loss  = 0;

    v_array<uint32_t> ystar;
    for (size_t i=0; i<len; i++) {
      srn.snapshot(vw, i, 1, &i, sizeof(i), true);
      srn.snapshot(vw, i, 2, &total_loss, sizeof(total_loss), false);

      get_oracle_labels(ec[i], &ystar);

      size_t prediction = srn.predict(vw, base, &ec[i], 0, NULL, &ystar);

      if (ystar.size() > 0)
        total_loss += (float)(prediction != ystar[0]);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << ((ystar.size() == 0) ? '?' : ystar[0]) << ' ';
    }
    srn.declare_loss(vw, len, total_loss);

    ystar.erase();  ystar.delete_v();
  }
}
