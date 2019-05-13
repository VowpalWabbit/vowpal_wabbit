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
struct sfm_data
{
  bool rank;
};

template <bool is_learn>
void predict_or_learn(sfm_data& data, LEARNER::multi_learner& base, multi_ex& ec_seq)
{
  multi_ex tmp;
  multi_ex* source = &ec_seq;
  if (ec_seq.size() == 0)
    return;
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
    if (data.rank)
    {
      ec_seq[0]->pred.a_s.clear();
      ec_seq[0]->pred.a_s.push_back(tmp[0]->pred.a_s[0]);
      std::swap(ec_seq[0]->pred.a_s, tmp[0]->pred.a_s);
    }
  }
}

LEARNER::base_learner* shared_feature_merger_setup(config::options_i& options, vw& all)
{
  if (!options.was_supplied("csoaa_ldf"))
  {
    if (!options.was_supplied("wap_ldf"))
    {
      return nullptr;
    }
  }
  auto data = scoped_calloc_or_throw<sfm_data>();

  data->rank = options.was_supplied("csoaa_rank");
  auto* base = LEARNER::as_multiline(setup_base(options, all));
  auto& learner = LEARNER::init_learner(data, base, predict_or_learn<true>, predict_or_learn<false>);

  return LEARNER::make_base(learner);
}

}  // namespace shared_feature_merger

}  // namespace VW
