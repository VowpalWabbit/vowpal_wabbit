// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "shared_feature_merger.h"
#include "cb.h"
#include "example.h"
#include "label_dictionary.h"
#include "learner.h"
#include "options.h"
#include "parse_args.h"
#include "vw.h"
#include "scope_exit.h"

#include <iterator>

namespace VW
{
namespace shared_feature_merger
{
static const std::vector<std::string> option_strings = {
    "csoaa_ldf", "wap_ldf", "cb_adf", "explore_eval", "cbify_ldf", "cb_explore_adf", "warm_cb"};

bool use_reduction(config::options_i& options)
{
  for (const auto& opt : option_strings)
  {
    if (options.was_supplied(opt)) return true;
  }
  return false;
}

struct sfm_data
{
};

template <bool is_learn>
void predict_or_learn(sfm_data&, VW::LEARNER::multi_learner& base, multi_ex& ec_seq)
{
  if (ec_seq.size() == 0) THROW("cb_adf: At least one action must be provided for an example to be valid.");

  multi_ex::value_type shared_example = nullptr;

  const bool has_example_header = CB::ec_is_example_header(*ec_seq[0]);
  if (has_example_header)
  {
    shared_example = ec_seq[0];
    ec_seq.erase(ec_seq.begin());
    // merge sequences
    for (auto& example : ec_seq) LabelDict::add_example_namespaces_from_example(*example, *shared_example);
    std::swap(ec_seq[0]->pred, shared_example->pred);
    std::swap(ec_seq[0]->tag, shared_example->tag);
  }

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([has_example_header, &shared_example, &ec_seq] {
    if (has_example_header)
    {
      for (auto& example : ec_seq) LabelDict::del_example_namespaces_from_example(*example, *shared_example);
      std::swap(shared_example->pred, ec_seq[0]->pred);
      std::swap(shared_example->tag, ec_seq[0]->tag);
      ec_seq.insert(ec_seq.begin(), shared_example);
    }
  });

  if (ec_seq.size() == 0) return;
  if (is_learn)
    base.learn(ec_seq);
  else
    base.predict(ec_seq);
}

VW::LEARNER::base_learner* shared_feature_merger_setup(config::options_i& options, vw& all)
{
  if (!use_reduction(options)) return nullptr;

  auto data = scoped_calloc_or_throw<sfm_data>();

  auto* base = VW::LEARNER::as_multiline(setup_base(options, all));

  auto& learner = VW::LEARNER::init_learner(
    data, base, predict_or_learn<true>, predict_or_learn<false>,
      all.get_setupfn_name(shared_feature_merger_setup), base->learn_returns_prediction);

  // TODO: Incorrect feature numbers will be reported without merging the example namespaces from the
  //       shared example in a finish_example function. However, its too expensive to perform the full operation.

  return VW::LEARNER::make_base(learner);
}

}  // namespace shared_feature_merger

}  // namespace VW
