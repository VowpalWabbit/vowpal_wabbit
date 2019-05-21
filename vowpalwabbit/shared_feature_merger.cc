#include "shared_feature_merger.h"
#include "cb.h"
#include "example.h"
#include "label_dictionary.h"
#include "learner.h"
#include "options.h"
#include "parse_args.h"
#include "vw.h"

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
    if (options.was_supplied(opt))
      return true;
  }
  return false;
}

struct sfm_data
{
  LEARNER::multi_learner* base;
};

template <bool is_learn>
void predict_or_learn(sfm_data& data, LEARNER::multi_learner& base, multi_ex& ec_seq)
{
  if (ec_seq.size() == 0)
    THROW("cb_adf: At least one action must be provided for an example to be valid.");
  multi_ex tmp;
  multi_ex* source = &ec_seq;

  const bool has_example_header = CB::ec_is_example_header(*ec_seq[0]);
  if (has_example_header)
  {
    source = &tmp;
    // copy vector and pass it down instead
    tmp.reserve(ec_seq.size() - 1);
    // skip first element since its a shared feature
    auto startIter = std::next(ec_seq.begin());
    tmp.insert(tmp.end(), startIter, ec_seq.end());
    // merge sequences
    for (size_t k = 0; k < tmp.size(); k++) LabelDict::add_example_namespaces_from_example(*tmp[k], *ec_seq[0]);
  }
  if (source->size() == 0)
    return;
  if (is_learn)
    base.learn(*source);
  else
    base.predict(*source);

  if (has_example_header)
  {
    for (size_t k = 0; k < tmp.size(); k++) LabelDict::del_example_namespaces_from_example(*tmp[k], *ec_seq[0]);
    ec_seq[0]->pred = tmp[0]->pred;
  }
}

void finish_multiline_example(vw& all, sfm_data& data, multi_ex& ec_seq)
{
  multi_ex tmp;
  multi_ex* source = &ec_seq;

  const bool has_example_header = CB::ec_is_example_header(*ec_seq[0]);
  if (has_example_header)
  {
    source = &tmp;
    // copy vector and pass it down instead
    tmp.reserve(ec_seq.size() - 1);
    // skip first element since its a shared feature
    auto startIter = std::next(ec_seq.begin());
    tmp.insert(tmp.end(), startIter, ec_seq.end());
    // merge sequences
    for (size_t k = 0; k < tmp.size(); k++) LabelDict::add_example_namespaces_from_example(*tmp[k], *ec_seq[0]);
  }
  if (source->size() == 0)
    return;

  data.base->finish_example(all, *source);

  if (has_example_header)
  {
    for (size_t k = 0; k < tmp.size(); k++) LabelDict::del_example_namespaces_from_example(*tmp[k], *ec_seq[0]);
    ec_seq[0]->pred = tmp[0]->pred;
  }
}

LEARNER::base_learner* shared_feature_merger_setup(config::options_i& options, vw& all)
{
  if (!use_reduction(options))
    return nullptr;

  auto data = scoped_calloc_or_throw<sfm_data>();

  auto* base = LEARNER::as_multiline(setup_base(options, all));
  data->base = base;
  auto& learner = LEARNER::init_learner(data, base, predict_or_learn<true>, predict_or_learn<false>);

  learner.set_finish_example(finish_multiline_example);

  return LEARNER::make_base(learner);
}

}  // namespace shared_feature_merger

}  // namespace VW
