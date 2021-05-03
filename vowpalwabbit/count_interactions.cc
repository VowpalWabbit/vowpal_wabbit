// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <sstream>
#include <cfloat>
#include "count_interactions.h"
#include "reductions.h"
#include "v_array.h"

#include "io/logger.h"
#include "interactions.h"
#include "vw_math.h"
#include <algorithm>
#include <iterator>

using namespace VW::config;

namespace logger = VW::io::logger;

struct count_interactions
{
};

using count_func_t = void(const example_predict& ec, size_t& new_features_cnt, float& new_features_value);

template <bool is_learn, count_func_t count_func>
void transform_single_ex(count_interactions&, VW::LEARNER::single_learner& base, example& ec)
{
  size_t new_features_cnt = 0;
  float new_features_sum_feat_sq = 0.f;
  count_func(ec, new_features_cnt, new_features_sum_feat_sq);
  ec.num_features_from_interactions = new_features_cnt;
  ec.total_sum_feat_sq_from_interactions = new_features_sum_feat_sq;

  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
}

template <count_func_t count_func>
inline void multipredict(count_interactions&, VW::LEARNER::single_learner& base, example& ec, size_t count, size_t,
    polyprediction* pred, bool finalize_predictions)
{
  size_t new_features_cnt = 0;
  float new_features_sum_feat_sq = 0.f;
  count_func(ec, new_features_cnt, new_features_sum_feat_sq);
  ec.num_features_from_interactions = new_features_cnt;
  ec.total_sum_feat_sq_from_interactions = new_features_sum_feat_sq;

  base.multipredict(ec, 0, count, pred, finalize_predictions);
}

template <count_func_t count_func>
void update(count_interactions&, VW::LEARNER::single_learner& base, example& ec)
{
  size_t new_features_cnt = 0;
  float new_features_sum_feat_sq = 0.f;
  count_func(ec, new_features_cnt, new_features_sum_feat_sq);
  ec.num_features_from_interactions = new_features_cnt;
  ec.total_sum_feat_sq_from_interactions = new_features_sum_feat_sq;
  base.update(ec);
}

VW::LEARNER::base_learner* count_interactions_setup(options_i& options, vw& all)
{
  bool permutations;
  option_group_definition new_options("Generate interactions");
  new_options.add(make_option("permutations", permutations)
               .help("Use permutations instead of combinations for feature interactions of same namespace."));
  options.add_and_parse(new_options);

  // This reduction is not needed when there are no interactions.
  // CCB implicitly adds interactions and so much be explicitly specified here.
  if ((all.interactions.interactions.empty() && !all.interactions.quadratics_wildcard_expansion) && !options.was_supplied("ccb_explore_adf"))
  {
    return nullptr;
  }

  using learn_pred_func_t = void (*)(count_interactions&, VW::LEARNER::single_learner&, example&);
  using multipredict_func_t =
      void (*)(count_interactions&, VW::LEARNER::single_learner&, example&, size_t, size_t, polyprediction*, bool);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;
  learn_pred_func_t update_func;
  multipredict_func_t multipredict_func;

  if (permutations)
  {
    learn_func = transform_single_ex<true, INTERACTIONS::eval_count_of_generated_ft_permutations>;
    pred_func = transform_single_ex<false, INTERACTIONS::eval_count_of_generated_ft_permutations>;
    update_func = update<INTERACTIONS::eval_count_of_generated_ft_permutations>;
    multipredict_func = multipredict<INTERACTIONS::eval_count_of_generated_ft_permutations>;
  }
  else
  {
    learn_func = transform_single_ex<true, INTERACTIONS::eval_count_of_generated_ft_combinations>;
    pred_func = transform_single_ex<false, INTERACTIONS::eval_count_of_generated_ft_combinations>;
    update_func = update<INTERACTIONS::eval_count_of_generated_ft_combinations>;
    multipredict_func = multipredict<INTERACTIONS::eval_count_of_generated_ft_combinations>;

  }

  auto data = VW::make_unique<count_interactions>();
  auto* base = as_singleline(setup_base(options, all));
  auto* l = VW::LEARNER::make_reduction_learner(
      std::move(data), base, learn_func, pred_func, all.get_setupfn_name(count_interactions_setup))
                .set_multipredict(multipredict_func)
                .set_update(update_func)
                .build();
  return VW::LEARNER::make_base(*l);
}
