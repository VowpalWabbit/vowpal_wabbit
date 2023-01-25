#pragma once

#include "../base.h"
#include "example_types.h"
#include "options_types.h"
#include "learner_types.h"
#include "workspace_types.h"

namespace pseudo_vw
{
  struct magic_t {};

  using reduction_features = magic_t;

  namespace VW
  {
    using string_view = magic_t;
    using label_parser_reuse_mem = magic_t;
    using named_labels = magic_t;

    namespace io {
      using logger = magic_t;
    }

    using label_type_t = magic_t;
  }

  using io_buf = magic_t;

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
    //void delayed_state_attach()

    VW::LEARNER::base_learner* setup_base_learner();
    VW::config::options_i* get_options();
    VW::workspace* get_all_pointer();
  };

  namespace VW
  {
    using setup_base_i = stack_builder_t;
  }
}