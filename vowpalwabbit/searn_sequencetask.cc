#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "searn.h"
#include "gd.h"
#include "io.h"
#include "parser.h"
#include "constant.h"
#include "oaa.h"
#include "csoaa.h"
#include "searn_sequencetask.h"

namespace SequenceTask {
  SearnUtil::history_info hinfo;
  v_array<action> truth = v_array<action>();

  struct seq_state {
    // global stuff -- common to any state in a trajectory
    example** ec_start;
    size_t length;

    // trajectory-specific stuff
    size_t pos;
    history predictions;
    float cum_loss;

    // everything is zero based, so pos starts out at zero and is what
    // we will predict NEXT.  this means that when pos==length we're
    // done.
  };

  bool initialize(po::variables_map& vm)
  {
    SearnUtil::default_info(&hinfo);

    if (vm.count("searn_sequencetask_bigrams"))          hinfo.bigrams = true;
    if (vm.count("searn_sequencetask_history"))          hinfo.length = vm["searn_sequencetask_history"].as<size_t>();
    if (vm.count("searn_sequencetask_bigram_features"))  hinfo.bigram_features = true;
    if (vm.count("searn_sequencetask_features"))         hinfo.features = vm["searn_sequencetask_features"].as<size_t>();

    return true;
  }

  bool final(state s0)
  {
    seq_state* s = (seq_state*)s0;
    return s->pos >= s->length;
  }


  float loss(state s0)
  {
    return ((seq_state*)s0)->cum_loss;
  }


  void step(state s0, action a)
  {
    seq_state* s = (seq_state*)s0;

    s->cum_loss += (oracle(s0) == a) ? 0.0 : 1.0;

    if (hinfo.length > 0) {
      for (size_t i=1; i<hinfo.length; i++)
        s->predictions[i-1] = s->predictions[i];
      s->predictions[hinfo.length-1] = a;
    }

    s->pos = s->pos + 1;
  }

  action oracle(state s0)
  {
    seq_state* s = (seq_state*)s0;
    return truth[s->pos];
  }

  void copy(state src0, state*dst0)
  {
    seq_state* src = (seq_state*)src0;
    seq_state* dst = (seq_state*)SearnUtil::calloc_or_die(1, sizeof(seq_state));
    memcpy(dst, src, sizeof(seq_state));
    *dst0 = dst;
  }

  void finish(state*s0)
  {
    seq_state* s = (seq_state*)&s0;
    free(s->predictions);
    free(truth.begin);
    SearnUtil::free_it(s);
  }

  void start_state_multiline(example**ec, size_t len, state*s0)
  {
    seq_state* s = (seq_state*)SearnUtil::calloc_or_die(1, sizeof(seq_state));

    s->ec_start = ec;
    s->length   = len;
    s->pos      = 0;
    s->cum_loss = 0.;

    s->predictions = (history)SearnUtil::calloc_or_die(hinfo.length, sizeof(uint32_t));
    for (size_t t=0; t<hinfo.length; t++)
      s->predictions[t] = 0;

    truth.erase();
    for (size_t t=0; t<len; t++) {
      CSOAA::label*ld = (CSOAA::label*)(ec[t]->ld);
      float best_cost = FLT_MAX;
      size_t best_label = 0;
      for (size_t i=0; i<ld->costs.index(); i++)
        if (ld->costs[i].x < best_cost) {
          best_cost  = ld->costs[i].x;
          best_label = ld->costs[i].weight_index;
        }
      push(truth, best_label);
    }

    *s0 = s;
  }


  void cs_example(state s0, example*&ec, bool create)
  {
    seq_state* s = (seq_state*)s0;
    example* cur = s->ec_start[s->pos];
    if (create) {
      SearnUtil::add_history_to_example(&hinfo, cur, s->predictions);
      ec = cur;
    } else { // destroy
      SearnUtil::remove_history_from_example(cur);
      ec = NULL;
    }
  }
}
