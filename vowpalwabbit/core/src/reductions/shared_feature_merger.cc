// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/shared_feature_merger.h"

#include "vw/config/options.h"
#include "vw/core/cb.h"
#include "vw/core/example.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/learner.h"
#include "vw/core/scope_exit.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"

#include <iterator>
#include <string>
#include <vector>
namespace
{
struct sfm_metrics
{
  size_t count_learn_example_with_shared = 0;
};

struct sfm_data
{
  std::unique_ptr<sfm_metrics> _metrics;
  VW::label_type_t label_type = VW::label_type_t::cb;
};

template <bool is_learn>
void predict_or_learn(sfm_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq)
{
  if (ec_seq.empty()) THROW("cb_adf: At least one action must be provided for an example to be valid.");

  VW::multi_ex::value_type shared_example = nullptr;

  const bool has_example_header = VW::LEARNER::ec_is_example_header(*ec_seq[0], data.label_type);

  if (has_example_header)
  {
    shared_example = ec_seq[0];
    ec_seq.erase(ec_seq.begin());
    // merge sequences
    for (auto& example : ec_seq) { LabelDict::add_example_namespaces_from_example(*example, *shared_example); }
    std::swap(ec_seq[0]->pred, shared_example->pred);
    std::swap(ec_seq[0]->tag, shared_example->tag);
  }

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([has_example_header, &shared_example, &ec_seq] {
    if (has_example_header)
    {
      for (auto& example : ec_seq) { LabelDict::del_example_namespaces_from_example(*example, *shared_example); }
      std::swap(shared_example->pred, ec_seq[0]->pred);
      std::swap(shared_example->tag, ec_seq[0]->tag);
      ec_seq.insert(ec_seq.begin(), shared_example);
    }
  });

  if (ec_seq.empty()) { return; }
  if (is_learn) { base.learn(ec_seq); }
  else
  {
    base.predict(ec_seq);
  }

  if (data._metrics)
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_COND_CONST_EXPR
    if (is_learn && has_example_header) { data._metrics->count_learn_example_with_shared++; }
    VW_WARNING_STATE_POP
  }
}

void persist(sfm_data& data, VW::metric_sink& metrics)
{
  if (data._metrics)
  { metrics.set_uint("sfm_count_learn_example_with_shared", data._metrics->count_learn_example_with_shared); }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::shared_feature_merger_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto* base = stack_builder.setup_base_learner();
  if (base == nullptr) { return nullptr; }
  std::set<label_type_t> sfm_labels = {label_type_t::cb, label_type_t::cs};
  if (sfm_labels.find(base->get_input_label_type()) == sfm_labels.end() || !base->is_multiline()) { return base; }

  auto data = VW::make_unique<sfm_data>();
  if (options.was_supplied("extra_metrics")) { data->_metrics = VW::make_unique<sfm_metrics>(); }

  auto* multi_base = VW::LEARNER::as_multiline(base);
  data->label_type = all.example_parser->lbl_parser.label_type;

  // Both label and prediction types inherit that of base.
  auto* learner = VW::LEARNER::make_reduction_learner(std::move(data), multi_base, predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(shared_feature_merger_setup))
                      .set_learn_returns_prediction(base->learn_returns_prediction)
                      .set_persist_metrics(persist)
                      .build();

  // TODO: Incorrect feature numbers will be reported without merging the example namespaces from the
  //       shared example in a finish_example function. However, its too expensive to perform the full operation.

  return VW::LEARNER::make_base(*learner);
}
