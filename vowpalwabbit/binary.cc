// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "reductions.h"

using namespace VW::config;
using std::endl;

namespace VW { namespace binary {
  template <bool is_learn>
  void predict_or_learn(char&, VW::LEARNER::single_learner& base, example& ec)
  {
    if (is_learn)
    {
      base.learn(ec);
    }
    else
    {
      base.predict(ec);
    }

    if (ec.pred.scalar > 0)
      ec.pred.scalar = 1;
    else
      ec.pred.scalar = -1;

    if (ec.l.simple.label != FLT_MAX)
    {
      if (fabs(ec.l.simple.label) != 1.f)
        std::cout << "You are using label " << ec.l.simple.label << " not -1 or 1 as loss function expects!" << std::endl;
      else if (ec.l.simple.label == ec.pred.scalar)
        ec.loss = 0.;
      else
        ec.loss = ec.weight;
    }
  }

  VW::LEARNER::base_learner* binary_setup(options_i& options, vw& all)
  {
    bool binary = false;
    option_group_definition new_options("Binary loss");
    new_options.add(make_option("binary", binary).keep().help("report loss as binary classification on -1,1"));
    options.add_and_parse(new_options);

    if (!binary)
      return nullptr;

    VW::LEARNER::learner<char, example>& ret =
        VW::LEARNER::init_learner(as_singleline(setup_base(options, all)), predict_or_learn<true>, predict_or_learn<false>);
    return make_base(ret);
  }

}  // namespace binary
}  // namespace VW
