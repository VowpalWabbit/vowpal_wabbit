// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cerrno>
#include <cfloat>

#include "reductions.h"
#include "rand48.h"
#include "vw.h"
#include "active.h"
#include "vw_exception.h"

using namespace LEARNER;
using namespace VW::config;

float get_active_coin_bias(float k, float avg_loss, float g, float c0)
{
  float b, sb, rs, sl;
  b = (float)(c0 * (log(k + 1.) + 0.0001) / (k + 0.0001));
  sb = std::sqrt(b);
  avg_loss = std::min(1.f, std::max(0.f, avg_loss));  // loss should be in [0,1]

  sl = std::sqrt(avg_loss) + std::sqrt(avg_loss + g);
  if (g <= sb * sl + b)
    return 1;
  rs = (sl + std::sqrt(sl * sl + 4 * g)) / (2 * g);
  return b * rs * rs;
}

float query_decision(active& a, float ec_revert_weight, float k)
{
  float bias, avg_loss, weighted_queries;
  if (k <= 1.)
    bias = 1.;
  else
  {
    weighted_queries = (float)a.all->sd->weighted_labeled_examples;
    avg_loss = (float)(a.all->sd->sum_loss / k + std::sqrt((1. + 0.5 * log(k)) / (weighted_queries + 0.0001)));
    bias = get_active_coin_bias(k, avg_loss, ec_revert_weight / k, a.active_c0);
  }
  if (a._random_state->get_and_update_random() < bias)
    return 1.f / bias;
  else
    return -1.;
}

template <bool is_learn>
void predict_or_learn_simulation(active& a, single_learner& base, example& ec)
{
  base.predict(ec);

  if (is_learn)
  {
    vw& all = *a.all;

    float k = (float)all.sd->t;
    float threshold = 0.f;

    ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
    float importance = query_decision(a, ec.confidence, k);

    if (importance > 0)
    {
      all.sd->queries += 1;
      ec.weight *= importance;
      base.learn(ec);
    }
    else
    {
      ec.l.simple.label = FLT_MAX;
      ec.weight = 0.f;
    }
  }
}

template <bool is_learn>
void predict_or_learn_active(active& a, single_learner& base, example& ec)
{
  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if (ec.l.simple.label == FLT_MAX)
  {
    float threshold = (a.all->sd->max_label + a.all->sd->min_label) * 0.5f;
    ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
  }
}

void active_print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
  {
    std::stringstream ss;
    ss << std::fixed << res;
    if (!print_tag_by_ref(ss, tag))
      ss << ' ';
    if (weight >= 0)
      ss << " " << std::fixed << weight;
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}

void output_and_account_example(vw& all, active& a, example& ec)
{
  label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only)
    all.sd->weighted_labels += ((double)ld.label) * ec.weight;
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ec.weight : 0;

  float ai = -1;
  if (ld.label == FLT_MAX)
    ai = query_decision(a, ec.confidence, (float)all.sd->weighted_unlabeled_examples);

  all.print_by_ref(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
  for (auto i : all.final_prediction_sink)
  {
    active_print_result(i, ec.pred.scalar, ai, ec.tag);
  }

  print_update(all, ec);
}

void return_active_example(vw& all, active& a, example& ec)
{
  output_and_account_example(all, a, ec);
  VW::finish_example(all, ec);
}

base_learner* active_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<active>();

  bool active_option = false;
  bool simulation = false;
  option_group_definition new_options("Active Learning");
  new_options.add(make_option("active", active_option).keep().help("enable active learning"))
      .add(make_option("simulation", simulation).help("active learning simulation mode"))
      .add(make_option("mellowness", data->active_c0)
               .default_value(8.f)
               .help("active learning mellowness parameter c_0. Default 8"));
  options.add_and_parse(new_options);

  if (!active_option)
    return nullptr;

  data->all = &all;
  data->_random_state = all.get_random_state();

  if (options.was_supplied("lda"))
    THROW("error: you can't combine lda and active learning");

  auto base = as_singleline(setup_base(options, all));

  // Create new learner
  learner<active, example>* l;
  if (options.was_supplied("simulation"))
    l = &init_learner(data, base, predict_or_learn_simulation<true>, predict_or_learn_simulation<false>);
  else
  {
    all.active = true;
    l = &init_learner(data, base, predict_or_learn_active<true>, predict_or_learn_active<false>);
    l->set_finish_example(return_active_example);
  }

  return make_base(*l);
}
