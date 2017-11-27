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
{
const float max_multiplier = 1000.f;
const size_t baseline_enabled_idx = 1357; // feature index for enabling baseline
}

namespace BASELINE
{
void set_baseline_enabled(example* ec)
{ auto& fs = ec->feature_space[message_namespace];
  for (auto& f : fs)
  { if (f.index() == baseline_enabled_idx)
    { f.value() = 1;
      return;
    }
  }
  // if not found, push new feature
  fs.push_back(1, baseline_enabled_idx);
}

void reset_baseline_disabled(example* ec)
{ auto& fs = ec->feature_space[message_namespace];
  for (auto& f : fs)
  { if (f.index() == baseline_enabled_idx)
    { f.value() = 0;
      return;
    }
  }
}

bool baseline_enabled(example* ec)
{ auto& fs = ec->feature_space[message_namespace];
  for (auto& f : fs)
  { if (f.index() == baseline_enabled_idx)
      return f.value() == 1;
  }
  return false;
}
}

struct baseline
{ example* ec;
  vw* all;
  bool lr_scaling; // whether to scale baseline learning rate based on max label
  float lr_multiplier;
  bool global_only; // only use a global constant for the baseline
  bool global_initialized;
  bool check_enabled; // only use baseline when the example contains enabled flag
};

void init_global(baseline& data)
{ if (!data.global_only)
    return;
  // use a separate global constant
  data.ec->indices.push_back(constant_namespace);
  // different index from constant to avoid conflicts
  data.ec->feature_space[constant_namespace].push_back(
      1, ((constant - 17) * data.all->wpp) << data.all->weights.stride_shift());
  data.ec->total_sum_feat_sq++;
  data.ec->num_features++;
}

template <bool is_learn>
void predict_or_learn(baseline& data, base_learner& base, example& ec)
{ // no baseline if check_enabled is true and example contains flag
  if (data.check_enabled && !BASELINE::baseline_enabled(&ec))
  { if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);
    return;
  }

  // always do a full prediction, for safety in accurate predictive validation
  if (data.global_only)
  { if (!data.global_initialized)
    { init_global(data);
      data.global_initialized = true;
    }
    VW::copy_example_metadata(/*audit=*/false, data.ec, &ec);
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
    { float multiplier = data.lr_multiplier;
      if (multiplier == 0)
      { multiplier = max<float>(0.0001f,
            max<float>(abs(data.all->sd->min_label), abs(data.all->sd->max_label)));
        if (multiplier > max_multiplier)
          multiplier = max_multiplier;
      }
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
  ("lr_multiplier", po::value<float>(), "learning rate multiplier for baseline model")
  ("global_only", "use separate example with only global constant for baseline predictions")
  ("check_enabled", "only use baseline when the example contains enabled flag");
  add_options(all);

  baseline& data = calloc_or_throw<baseline>();

  // initialize baseline example
  data.ec = VW::alloc_examples(simple_label.label_size, 1);
  data.ec->in_use = true;
  data.all = &all;
  if (!all.vm.count("loss_function") || all.vm["loss_function"].as<string>() != "logistic" )
    data.lr_scaling = true;
  if (all.vm.count("lr_multiplier"))
    data.lr_multiplier = all.vm["lr_multiplier"].as<float>();
  data.check_enabled = all.vm.count("check_enabled") > 0;
  data.global_only = all.vm.count("global_only") > 0; // initialization done later

  base_learner* base = setup_base(all);
  learner<baseline>& l = init_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>);

  l.set_finish(finish);

  return make_base(l);
}
