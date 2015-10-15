#include "reductions.h"

const int autoconstant = 524267083;

struct autolink {
  uint32_t d; // degree of the polynomial
  uint32_t stride_shift;
};

template <bool is_learn>
void predict_or_learn(autolink& b, LEARNER::base_learner& base, example& ec)
{
  base.predict(ec);
  float base_pred = ec.pred.scalar;
  // add features of label
  ec.indices.push_back(autolink_namespace);
  float sum_sq = 0;
  for (size_t i = 0; i < b.d; i++)
    if (base_pred != 0.)
    {
      feature f = { base_pred, (uint32_t) (autoconstant + (i << b.stride_shift)) };
      ec.atomics[autolink_namespace].push_back(f);
      sum_sq += base_pred*base_pred;
      base_pred *= ec.pred.scalar;
    }
  ec.total_sum_feat_sq += sum_sq;

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  ec.atomics[autolink_namespace].erase();
  ec.indices.pop();
  ec.total_sum_feat_sq -= sum_sq;
}

LEARNER::base_learner* autolink_setup(vw& all)
{
  if (missing_option<size_t, true>(all, "autolink", "create link function with polynomial d"))
    return nullptr;

  autolink& data = calloc_or_throw<autolink>();
  data.d = (uint32_t)all.vm["autolink"].as<size_t>();
  data.stride_shift = all.reg.stride_shift;

  LEARNER::learner<autolink>& ret =
    init_learner(&data, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);

  return make_base(ret);
}
