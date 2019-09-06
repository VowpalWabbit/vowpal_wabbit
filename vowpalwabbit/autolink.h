#pragma once

#include "reductions.h"

LEARNER::base_learner* autolink_setup(VW::config::options_i& options, vw& all);

namespace VW
{
  struct autolink
  {
    autolink(uint32_t d, uint32_t stride_shift);
    void predict(LEARNER::single_learner& base, example& ec);
    void learn(LEARNER::single_learner& base, example& ec);

  private:
    void prepare_example(LEARNER::single_learner& base, example& ec);
    void reset_example(example& ec);

    // degree of the polynomial
    const uint32_t _poly_degree;
    const uint32_t _stride_shift;
    static constexpr int AUTOCONSTANT = 524267083;
  };
}
