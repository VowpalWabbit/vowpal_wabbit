#include "autolink.h"

using namespace VW::config;

VW::autolink::autolink(uint32_t d, uint32_t stride_shift)
  : m_d(d), m_stride_shift(stride_shift)
{}

void VW::autolink::predict(LEARNER::single_learner& base, example& ec)
{
  prepare_example(base, ec);
  base.predict(ec);
  reset_example(ec);
}

void VW::autolink::learn(LEARNER::single_learner& base, example& ec)
{
  prepare_example(base, ec);
  base.learn(ec);
  reset_example(ec);
}

void VW::autolink::prepare_example(LEARNER::single_learner& base, example& ec)
{
  base.predict(ec);
  float base_pred = ec.pred.scalar;

  // Add features of label.
  ec.indices.push_back(autolink_namespace);
  features& fs = ec.feature_space[autolink_namespace];
  for (size_t i = 0; i < m_d; i++)
  {
    if (base_pred != 0.)
    {
      fs.push_back(base_pred, AUTOCONSTANT + (i << m_stride_shift));
      base_pred *= ec.pred.scalar;
    }
  }
  ec.total_sum_feat_sq += fs.sum_feat_sq;
}

void VW::autolink::reset_example(example& ec)
{
  features& fs = ec.feature_space[autolink_namespace];
  ec.total_sum_feat_sq -= fs.sum_feat_sq;
  ec.feature_space[autolink_namespace].clear();
  ec.indices.pop();
}

template <bool is_learn>
void predict_or_learn(VW::autolink& b, LEARNER::single_learner& base, example& ec)
{
  if (is_learn)
    b.learn(base, ec);
  else
    b.predict(base, ec);
}

LEARNER::base_learner* autolink_setup(options_i& options, vw& all)
{
  uint32_t d;
  option_group_definition new_options("Autolink");
  new_options.add(make_option("autolink", d).keep().help("create link function with polynomial d"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("autolink"))
    return nullptr;

  auto autolink_reduction = scoped_calloc_or_throw<VW::autolink>(d, all.weights.stride_shift());
  return make_base(init_learner(
    autolink_reduction,
    as_singleline(setup_base(options, all)),
    predict_or_learn<true>,
    predict_or_learn<false>));
}
