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

  struct sequencetask_data {
    SearnUtil::history_info hinfo;
    v_array<size_t> yhat;
  };

  void initialize(vw& vw, searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    sequencetask_data* dat = new sequencetask_data();
    dat->hinfo.features        = 0;
    dat->hinfo.length          = dat->hinfo.features+1;
    dat->hinfo.bigrams         = dat->hinfo.length > 1;
    dat->hinfo.bigram_features = false;
    srn.task_data = dat;

    po::options_description desc("Searn[sequence] options");
    desc.add_options()
      ("searn_sequencetask_history",  po::value<size_t>(), "length of history to use")
      ("searn_sequencetask_features", po::value<size_t>(), "length of history to pair with observed features")
      ("searn_sequencetask_bigrams",                       "use bigrams from history")
      ("searn_sequencetask_bigram_features",               "use bigrams from history paired with observed features");

    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    po::parsed_options parsed_file = po::command_line_parser(vw.options_from_file_argc, vw.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);

    if(vm_file.count("searn")) { //we loaded a predictor file which contains the options we should use for the sequence task
      //load all params from file
      if(vm_file.count("searn_sequencetask_history"))         dat->hinfo.length   = vm_file["searn_sequencetask_history"].as<size_t>();
      if(vm_file.count("searn_sequencetask_features"))        dat->hinfo.features = vm_file["searn_sequencetask_features"].as<size_t>(); 
      if(vm_file.count("searn_sequencetask_bigrams"))         dat->hinfo.bigrams = true;
      if(vm_file.count("searn_sequencetask_bigram_features")) dat->hinfo.bigram_features = true;

      //check if there is a mismatch between what was specified in command line and alert user
      if( vm.count("searn_sequencetask_history") && dat->hinfo.length != vm["searn_sequencetask_history"].as<size_t>() )
        std::cerr << "warning: you specified a different value for --searn_sequencetask_history than the one loaded from regressor. Pursuing with loaded value: " << dat->hinfo.length << endl;

      if( vm.count("searn_sequencetask_features") && dat->hinfo.features != vm["searn_sequencetask_features"].as<size_t>() )
        std::cerr << "warning: you specified a different value for --searn_sequencetask_features than the one loaded from regressor. Pursuing with loaded value: " << dat->hinfo.features << endl;

      if( vm.count("searn_sequencetask_bigrams") && !dat->hinfo.bigrams )
        std::cerr << "warning: you specified --searn_sequencetask_bigrams but loaded regressor not using bigrams. Pursuing without bigrams." << endl;

      if( vm.count("searn_sequencetask_bigram_features") && !dat->hinfo.bigram_features )
        std::cerr << "warning: you specified --searn_sequencetask_bigram_features but loaded regressor not using bigram_features. Pursuing without bigram_features." << endl;
    } else {
      if (vm.count("searn_sequencetask_bigrams")) {
        dat->hinfo.bigrams = true;
        vw.options_from_file.append(" --searn_sequencetask_bigrams");
      }

      if (vm.count("searn_sequencetask_history")) { 
        dat->hinfo.length = vm["searn_sequencetask_history"].as<size_t>();

        std::stringstream ss;
        ss << " --searn_sequencetask_history " << dat->hinfo.length;
        vw.options_from_file.append(ss.str());
      }

      if (vm.count("searn_sequencetask_bigram_features")) {
        dat->hinfo.bigram_features = true;
        vw.options_from_file.append(" --searn_sequencetask_bigram_features");
      }

      if (vm.count("searn_sequencetask_features")) {
        dat->hinfo.features = vm["searn_sequencetask_features"].as<size_t>();

        std::stringstream ss;
        ss << " --searn_sequencetask_features " << dat->hinfo.features;
        vw.options_from_file.append(ss.str());
      }
    }
  }

void finish(vw& vw, searn& srn) {
    sequencetask_data* dat = (sequencetask_data*)srn.task_data;
    dat->yhat.delete_v();
    delete dat;
  }

  void get_oracle_labels(example*ec, v_array<uint32_t>*out) {
    out->erase();
    if (OAA::example_is_test(ec))
      return;
    OAA::mc_label *lab = (OAA::mc_label*)ec->ld;
    out->push_back( (uint32_t)lab->label );
  }

  void structured_predict_v1(vw& vw, searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    sequencetask_data* dat = (sequencetask_data*)srn.task_data;
    float total_loss  = 0;
    size_t history_length = max(dat->hinfo.features, dat->hinfo.length);
    bool is_train = false;

    dat->yhat.erase();
    dat->yhat.resize(history_length + len, true); // pad the beginning with zeros for <s>

    v_array<uint32_t> ystar;
    for (size_t i=0; i<len; i++) {
      srn.snapshot(vw, i, 1, &i, sizeof(i), true);
      srn.snapshot(vw, i, 2, dat->yhat.begin+i, sizeof(size_t)*history_length, true);
      srn.snapshot(vw, i, 3, &total_loss, sizeof(total_loss), false);

      get_oracle_labels(ec[i], &ystar);

      size_t prediction;
      SearnUtil::add_history_to_example(vw, &dat->hinfo, ec[i], dat->yhat.begin+i);
      prediction = srn.predict(vw, &ec[i], 0, NULL, &ystar);
      SearnUtil::remove_history_from_example(vw, &dat->hinfo, ec[i]);
      dat->yhat[i+history_length] = prediction;

      if (!OAA::example_is_test(ec[i])) {
        is_train = true;
        if (dat->yhat[i+history_length] != ystar.last())   // TODO: fix this
          total_loss += 1.0;
      }
    }
      
    if (output_ss != NULL) {
      for (size_t i=0; i<len; i++) {
        if (i > 0) (*output_ss) << ' ';
        (*output_ss) << dat->yhat[i+history_length];
      }
    }
    if (truth_ss != NULL) {
      for (size_t i=0; i<len; i++) {
        get_oracle_labels(ec[i], &ystar);
        if (i > 0) (*truth_ss) << ' ';
        if (ystar.size() > 0) (*truth_ss) << ystar[0];
        else (*truth_ss) << '?';
      }
    }
    
    ystar.erase();  ystar.delete_v();
    srn.declare_loss(vw, len, is_train ? total_loss : -1.f);
  }
}

