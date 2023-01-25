#pragma once

#include "../base.h"

namespace pseudo_vw
{
  struct example;

  using multi_ex = std::vector<example>;

  struct scalar_prediction
  {
    using value_type = float;
  };

  union polyprediction
  {
    scalar_prediction::value_type scalar;
  };

  struct simple_label
  {
    using value_type = float;
  };

  union polylabel
  {
    simple_label::value_type simple;
  };

  namespace VW {
    using example = pseudo_vw::example;
    using multi_ex = pseudo_vw::multi_ex;
  }
}

