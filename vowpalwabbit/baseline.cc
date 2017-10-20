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

namespace
{ const float max_multiplier = 1000.f;
}

struct baseline
{ example* ec;
  vw* all;
  bool lr_scaling; // whether to scale baseline learning rate based on max label
  bool global_only; // only use a global constant for the baseline
};

template <bool is_learn>
void predict_or_learn(baseline& data, base_learner& base, example& ec)
{ // always do a full prediction, for safety in accurate predictive validation
  if (data.global_only)
  { VW::copy_example_metadata(/*audit=*/false, data.ec, &ec);
    base.predict(*data.ec);
    ec.l.simple.initial = data.ec->pred.scalar;
    base.predict(ec);
  }
  else
    base.predict(ec);

  if (is_learn)
  { const double pred = ec.pred.scalar; // save 'safe' prediction

    // now learn
    data.ec->l.simple = ec.l.simple;
    if (!data.global_only)
    { // move label & constant features data over to baseline example
      VW::copy_example_metadata(/*audit=*/false, data.ec, &ec);
      VW::move_feature_namespace(data.ec, &ec, constant_namespace);
    }

    // regress baseline on label
    if (data.lr_scaling)
    { float multiplier = max<float>(0.0001f,
        max<float>(abs(data.all->sd->min_label), abs(data.all->sd->max_label)));
      if (multiplier > max_multiplier)
        multiplier = max_multiplier;
      cout << data.all->sd->min_label << " " << data.all->sd->max_label << " mult " << multiplier << endl;
      data.all->eta *= multiplier;
      base.learn(*data.ec);
      data.all->eta /= multiplier;
    }
    else
      base.learn(*data.ec);

    // regress residual
    ec.l.simple.initial = data.ec->pred.scalar;
    base.learn(ec);

    if (!data.global_only)
    { // move feature data back to the original example
      VW::move_feature_namespace(&ec, data.ec, constant_namespace);
    }

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
  new_options(all, "BASELINE options")
  ("global_only", "use separate example with only global constant for baseline predictions");
  add_options(all);

  baseline& data = calloc_or_throw<baseline>();

  // initialize baseline example
  data.ec = VW::alloc_examples(simple_label.label_size, 1);
  data.ec->in_use = true;
  data.all = &all;
  if (!all.vm.count("loss_function") || all.vm["loss_function"].as<string>() != "logistic" )
    data.lr_scaling = true;
  data.global_only = all.vm.count("global_only") > 0;
  if (data.global_only)
  { // use a separate global
    VW::add_constant_feature(all, data.ec);
  }

  base_learner* base = setup_base(all);
  learner<baseline>& l = init_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>);

  l.set_finish(finish);

  return make_base(l);
}
