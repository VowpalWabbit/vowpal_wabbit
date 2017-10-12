/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD (revised)
  license as described in the file LICENSE.
*/
#include <float.h>
#include <errno.h>

#include "reductions.h"
#include "vw.h"

using namespace std;
using namespace LEARNER;

struct baseline
{ example* ec;
  vw* all;
  bool lr_scaling; // whether to scale baseline learning rate based on max label
};

template <bool is_learn>
void predict_or_learn(baseline& data, base_learner& base, example& ec)
{ // always do a full prediction, for safety in accurate predictive validation
  base.predict(ec);

  if (is_learn)
  { const double pred = ec.pred.scalar; // save 'safe' prediction

    // now learn
    // move label & constant features data over to baseline example
    data.ec->l.simple = ec.l.simple;
    VW::copy_example_metadata(/*audit=*/false, data.ec, &ec);
    VW::move_feature_namespace(data.ec, &ec, constant_namespace);

    // regress baseline on label
    if (data.lr_scaling)
    { const float multiplier = max(0.0001, max(abs(data.all->sd->min_label), abs(data.all->sd->max_label)));
      cout << "mult " << multiplier << endl;
      data.all->eta *= multiplier;
      base.learn(*data.ec);
      data.all->eta /= multiplier;
    }
    else
      base.learn(*data.ec);

    // regress residual
    ec.l.simple.initial = data.ec->pred.scalar;
    base.learn(ec);

    // move feature data back to the original example
    VW::move_feature_namespace(&ec, data.ec, constant_namespace);

    // return the safe prediction
    ec.pred.scalar = pred;
  }
}

void finish(baseline& data)
{ VW::dealloc_example(simple_label.delete_label, *data.ec);
  free(data.ec);
}

base_learner* baseline_setup(vw& all)
{ if (missing_option(all, true, "baseline", "Learn an additive baseline (from constant features) and a residual separately in regression."))
    return nullptr;

  baseline& data = calloc_or_throw<baseline>();

  // initialize baseline example
  data.ec = VW::alloc_examples(simple_label.label_size, 1);
  data.ec->in_use = true;
  data.all = &all;

  base_learner* base = setup_base(all);
  learner<baseline>& l = init_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>);

  l.set_finish(finish);

  return make_base(l);
}
