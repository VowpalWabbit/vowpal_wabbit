#include "reductions.h"

using namespace VW::config;

const int autoconstant = 524267083;

struct autolink
{
  uint32_t d;  // degree of the polynomial
  uint32_t stride_shift;
};

template <bool is_learn>
void predict_or_learn(autolink& b, LEARNER::single_learner& base, example& ec)
{
  base.predict(ec);
  float base_pred = ec.pred.scalar;
  // add features of label
  ec.indices.push_back(autolink_namespace);
  features& fs = ec.feature_space[autolink_namespace];
  for (size_t i = 0; i < b.d; i++)
    if (base_pred != 0.)
    {
      fs.push_back(base_pred, autoconstant + (i << b.stride_shift));
      base_pred *= ec.pred.scalar;
    }
  ec.total_sum_feat_sq += fs.sum_feat_sq;

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  ec.total_sum_feat_sq -= fs.sum_feat_sq;
  fs.clear();
  ec.indices.pop();
}

LEARNER::base_learner* autolink_setup(options_i& options, vw& all)
{
  free_ptr<autolink> data = scoped_calloc_or_throw<autolink>();
  option_group_definition new_options("Autolink");
  new_options.add(make_option("autolink", data->d).keep().help("create link function with polynomial d"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("autolink"))
    return nullptr;

  data->stride_shift = all.weights.stride_shift();

  LEARNER::learner<autolink, example>& ret =
      init_learner(data, as_singleline(setup_base(options, all)), predict_or_learn<true>, predict_or_learn<false>);

  return make_base(ret);
}
