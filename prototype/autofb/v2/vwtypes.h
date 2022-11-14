#pragma once

#include "base.h"

namespace pseudo_vw
{
  struct example;

  using multi_ex = std::vector<example>;

  struct learner
  {
    
  };

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

  struct magic_t {};
  using reduction_features = magic_t;
  namespace VW
  {
    using example = pseudo_vw::example;
    using multi_ex = pseudo_vw::multi_ex;

    using string_view = magic_t;
    using label_parser_reuse_mem = magic_t;
    using named_labels = magic_t;

    namespace io {
      using logger = magic_t;
    }

    using label_type_t = magic_t;

  }

  using io_buf = magic_t;

  using namespace VW;

struct label_parser
{
  void (*default_label)(polylabel& label);
  void (*parse_label)(polylabel& label, reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
      const VW::named_labels* ldict, const std::vector<VW::string_view>& words, VW::io::logger& logger);
  size_t (*cache_label)(const polylabel& label, const reduction_features& red_features, io_buf& cache,
      const std::string& upstream_name, bool text);
  size_t (*read_cached_label)(polylabel& label, reduction_features& red_features, io_buf& cache);
  float (*get_weight)(const polylabel& label, const reduction_features& red_features);
  bool (*test_label)(const polylabel& label);
  VW::label_type_t label_type;
};

  struct stack_builder_t
  {

  };
}