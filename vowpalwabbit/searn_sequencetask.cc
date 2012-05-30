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
  bool   fake_as_ldf = false;
  size_t seq_max_action = 1;
  size_t constant_pow_length = 0;

  struct seq_state {
    // global stuff -- common to any state in a trajectory
    example** ec_start;
    size_t    length;

    // trajectory-specific stuff
    size_t    pos;
    history   predictions;
    size_t    predictions_hash;
    float     cum_loss;

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
    if (vm.count("searn_sequencetask_fake_ldf"))         fake_as_ldf = true;

    seq_max_action = vm["searn_max_action"].as<size_t>();
    constant_pow_length = 1;
    for (size_t i=0; i < hinfo.length; i++)
      constant_pow_length *= quadratic_constant;

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
      int old_val = s->predictions[0];
      s->predictions_hash -= old_val * constant_pow_length;
      s->predictions_hash += a;
      s->predictions_hash *= quadratic_constant;
      for (size_t i=1; i<hinfo.length; i++)
        s->predictions[i-1] = s->predictions[i];
      s->predictions[hinfo.length-1] = a;
    }

    s->pos = s->pos + 1;
  }

  action oracle(state s0)
  {
    seq_state* s = (seq_state*)s0;
    return ((OAA::mc_label*)s->ec_start[s->pos]->ld)->label;
  }

  state copy(state src0)
  {
    seq_state* src = (seq_state*)src0;
    seq_state* dst = (seq_state*)SearnUtil::calloc_or_die(1, sizeof(seq_state));
    //memcpy(dst, src, sizeof(seq_state));
    dst->ec_start = src->ec_start;
    dst->length   = src->length;
    dst->pos      = src->pos;
    dst->predictions_hash = src->predictions_hash;
    dst->cum_loss = src->cum_loss;
    dst->predictions = (history)SearnUtil::calloc_or_die(hinfo.length, sizeof(uint32_t));
    for (size_t t=0; t<hinfo.length; t++)
      dst->predictions[t] = src->predictions[t];

    //    cerr << "copy returning s = " << dst << endl;
    return (state)dst;
  }

  void finish(state s0)
  {
    seq_state* s = (seq_state*)s0;

    //    cerr << "finish    with s = " << s << " and s->predictions = " << s->predictions << endl;
    SearnUtil::free_it(s->predictions);
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

    s->predictions_hash = 0;

    //    cerr << "ssml returning s = " << s << endl;

    *s0 = s;
  }


  void cs_example(vw&all, state s0, example*&ec, bool create)
  {
    seq_state* s = (seq_state*)s0;
    example* cur = s->ec_start[s->pos];
    if (create) {
      SearnUtil::add_history_to_example(all, &hinfo, cur, s->predictions);
      ec = cur;
    } else { // destroy
      SearnUtil::remove_history_from_example(all, &hinfo, cur);
      ec = NULL;
    }
  }

  size_t hash(state s0)
  {
    seq_state* s = (seq_state*)s0;
    return quadratic_constant * (s->pos + quadratic_constant * s->predictions_hash);
  }
  
  bool equivalent(state a0, state b0)
  {
    seq_state* a = (seq_state*)a0;
    seq_state* b = (seq_state*)b0;

    if (a->pos != b->pos) return false;
    if (a->predictions_hash != b->predictions_hash) return false;

    for (size_t i=0; i<hinfo.length; i++)
      if (a->predictions[i] != b->predictions[i])
        return false;

    return true;
  }

  // // The following is just to test out LDF... we "fake" being an
  // // LDF-based task.

  // bool allowed(state s, action a)
  // {
  //   return ((a >= 1) && (a <= seq_max_action));
  // }

  // void cs_ldf_example(vw& all, state s, action a, example*& ec, bool create)
  // {
  //   seq_state* s = (seq_state*)s0;
  //   example* cur = s->ec_start[s->pos];
  //   if (create) {
  //   } else {
  //   }
  // }
}
