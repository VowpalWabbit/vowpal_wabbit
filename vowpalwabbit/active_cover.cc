// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cmath>
#include <cerrno>
#include <memory>
#include "reductions.h"
#include "rand48.h"
#include "float.h"
#include "vw.h"

using namespace LEARNER;
using namespace VW::config;

inline float sign(float w)
{
  if (w <= 0.f)
    return -1.f;
  else
    return 1.f;
}

struct active_cover
{
  // active learning algorithm parameters
  float active_c0;
  float alpha;
  float beta_scale;
  bool oracular;
  size_t cover_size;

  float* lambda_n;
  float* lambda_d;

  vw* all;  // statistics, loss
  std::shared_ptr<rand_state> _random_state;
  LEARNER::base_learner* l;

  ~active_cover()
  {
    delete[] lambda_n;
    delete[] lambda_d;
  }
};

bool dis_test(vw& all, example& ec, single_learner& base, float /* prediction */, float threshold)
{
  if (all.sd->t + ec.weight <= 3)
  {
    return true;
  }

  // Get loss difference
  float middle = 0.f;
  ec.confidence = fabsf(ec.pred.scalar - middle) / base.sensitivity(ec);

  float k = (float)all.sd->t;
  float loss_delta = ec.confidence / k;

  bool result = (loss_delta <= threshold);

  return result;
}

float get_threshold(float sum_loss, float t, float c0, float alpha)
{
  if (t < 3.f)
  {
    return 1.f;
  }
  else
  {
    float avg_loss = sum_loss / t;
    float threshold = std::sqrt(c0 * avg_loss / t) + fmax(2.f * alpha, 4.f) * c0 * log(t) / t;
    return threshold;
  }
}

float get_pmin(float sum_loss, float t)
{
  // t = ec.example_t - 1
  if (t <= 2.f)
  {
    return 1.f;
  }

  float avg_loss = sum_loss / t;
  float pmin = fmin(1.f / (std::sqrt(t * avg_loss) + log(t)), 0.5f);
  return pmin;  // treating n*eps_n = 1
}

float query_decision(active_cover& a, single_learner& l, example& ec, float prediction, float pmin, bool in_dis)
{
  if (a.all->sd->t + ec.weight <= 3)
  {
    return 1.f;
  }

  if (!in_dis)
  {
    return -1.f;
  }

  if (a.oracular)
  {
    return 1.f;
  }

  float p, q2 = 4.f * pmin * pmin;

  for (size_t i = 0; i < a.cover_size; i++)
  {
    l.predict(ec, i + 1);
    q2 += ((float)(sign(ec.pred.scalar) != sign(prediction))) * (a.lambda_n[i] / a.lambda_d[i]);
  }

  p = std::sqrt(q2) / (1 + std::sqrt(q2));

  if (std::isnan(p))
  {
    p = 1.f;
  }

  if (a._random_state->get_and_update_random() <= p)
  {
    return 1.f / p;
  }
  else
  {
    return -1.f;
  }
}

template <bool is_learn>
void predict_or_learn_active_cover(active_cover& a, single_learner& base, example& ec)
{
  base.predict(ec, 0);

  if (is_learn)
  {
    vw& all = *a.all;

    float prediction = ec.pred.scalar;
    float t = (float)a.all->sd->t;
    float ec_input_weight = ec.weight;
    float ec_input_label = ec.l.simple.label;

    // Compute threshold defining allowed set A
    float threshold = get_threshold((float)all.sd->sum_loss, t, a.active_c0, a.alpha);
    bool in_dis = dis_test(all, ec, base, prediction, threshold);
    float pmin = get_pmin((float)all.sd->sum_loss, t);
    float importance = query_decision(a, base, ec, prediction, pmin, in_dis);

    // Query (or not)
    if (!in_dis)  // Use predicted label
    {
      ec.l.simple.label = sign(prediction);
      ec.weight = ec_input_weight;
      base.learn(ec, 0);
    }
    else if (importance > 0)  // Use importance-weighted example
    {
      all.sd->queries += 1;
      ec.weight = ec_input_weight * importance;
      ec.l.simple.label = ec_input_label;
      base.learn(ec, 0);
    }
    else  // skipped example
    {
      // Make sure the loss computation does not include
      // skipped examples
      ec.l.simple.label = FLT_MAX;
      ec.weight = 0;
    }

    // Update the learners in the cover and their weights
    float q2 = 4.f * pmin * pmin;
    float p, s, cost, cost_delta = 0;
    float ec_output_label = ec.l.simple.label;
    float ec_output_weight = ec.weight;
    float r = 2.f * threshold * t * a.alpha / a.active_c0 / a.beta_scale;

    // Set up costs
    // cost = cost of predicting erm's prediction
    // cost_delta = cost - cost of predicting the opposite label
    if (in_dis)
    {
      cost = r * (fmax(importance, 0.f)) * ((float)(sign(prediction) != sign(ec_input_label)));
    }
    else
    {
      cost = 0.f;
      cost_delta = -r;
    }

    for (size_t i = 0; i < a.cover_size; i++)
    {
      // Update cost
      if (in_dis)
      {
        p = std::sqrt(q2) / (1.f + std::sqrt(q2));
        s = 2.f * a.alpha * a.alpha - 1.f / p;
        cost_delta = 2.f * cost - r * (fmax(importance, 0.f)) - s;
      }

      // Choose min-cost label as the label
      // Set importance weight to be the cost difference
      ec.l.simple.label = -1.f * sign(cost_delta) * sign(prediction);
      ec.weight = ec_input_weight * fabs(cost_delta);

      // Update learner
      base.learn(ec, i + 1);
      base.predict(ec, i + 1);

      // Update numerator of lambda
      a.lambda_n[i] += 2.f * ((float)(sign(ec.pred.scalar) != sign(prediction))) * cost_delta;
      a.lambda_n[i] = fmax(a.lambda_n[i], 0.f);

      // Update denominator of lambda
      a.lambda_d[i] += ((float)(sign(ec.pred.scalar) != sign(prediction) && in_dis)) / (float)pow(q2, 1.5);

      // Accumulating weights of learners in the cover
      q2 += ((float)(sign(ec.pred.scalar) != sign(prediction))) * (a.lambda_n[i] / a.lambda_d[i]);
    }

    // Restoring the weight, the label, and the prediction
    ec.weight = ec_output_weight;
    ec.l.simple.label = ec_output_label;
    ec.pred.scalar = prediction;
  }
}

base_learner* active_cover_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<active_cover>();
  option_group_definition new_options("Active Learning with Cover");

  bool active_cover_option = false;
  new_options.add(make_option("active_cover", active_cover_option).keep().help("enable active learning with cover"))
      .add(make_option("mellowness", data->active_c0)
               .default_value(8.f)
               .help("active learning mellowness parameter c_0. Default 8."))
      .add(make_option("alpha", data->alpha)
               .default_value(1.f)
               .help("active learning variance upper bound parameter alpha. Default 1."))
      .add(make_option("beta_scale", data->beta_scale)
               .default_value(sqrtf(10.f))
               .help("active learning variance upper bound parameter beta_scale. Default std::sqrt(10)."))
      .add(make_option("cover", data->cover_size).keep().default_value(12).help("cover size. Default 12."))
      .add(make_option("oracular", data->oracular).help("Use Oracular-CAL style query or not. Default false."));
  options.add_and_parse(new_options);

  if (!active_cover_option)
    return nullptr;

  data->all = &all;
  data->_random_state = all.get_random_state();
  data->beta_scale *= data->beta_scale;

  if (data->oracular)
    data->cover_size = 0;

  if (options.was_supplied("lda"))
    THROW("error: you can't combine lda and active learning");

  if (options.was_supplied("active"))
    THROW("error: you can't use --active_cover and --active at the same time");

  auto base = as_singleline(setup_base(options, all));

  data->lambda_n = new float[data->cover_size];
  data->lambda_d = new float[data->cover_size];

  for (size_t i = 0; i < data->cover_size; i++)
  {
    data->lambda_n[i] = 0.f;
    data->lambda_d[i] = 1.f / 8.f;
  }

  // Create new learner
  learner<active_cover, example>& l = init_learner(
      data, base, predict_or_learn_active_cover<true>, predict_or_learn_active_cover<false>, data->cover_size + 1);

  return make_base(l);
}
