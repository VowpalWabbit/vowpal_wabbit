// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

/*
 * Implementation of online boosting algorithms from
 *    Beygelzimer, Kale, Luo: Optimal and adaptive algorithms for online boosting,
 *    ICML-2015.
 */

#include "vw/core/reductions/boosting.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/correctedMath.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw_math.h"
#include "vw/io/logger.h"

#include <fmt/core.h>

#include <cfloat>
#include <climits>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace VW::LEARNER;
using namespace VW::config;
using namespace VW::reductions;

using std::endl;

namespace
{

class boosting
{
public:
  int N = 0;  // NOLINT
  float gamma = 0.f;
  std::string alg = "";
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> random_state;
  std::vector<std::vector<int64_t> > C;  // NOLINT
  std::vector<float> alpha;
  std::vector<float> v;
  int t = 0;
  VW::io::logger logger;

  explicit boosting(VW::io::logger logger) : logger(std::move(logger)) {}
};

//---------------------------------------------------
// Online Boost-by-Majority (BBM)
// --------------------------------------------------
template <bool is_learn>
void predict_or_learn(boosting& o, VW::LEARNER::learner& base, VW::example& ec)
{
  auto& ld = ec.l.simple;

  float final_prediction = 0;

  float s = 0;
  float u = ec.weight;

  if (is_learn) { o.t++; }

  for (int i = 0; i < o.N; i++)
  {
    if (is_learn)
    {
      float k = floorf((o.N - i - s) / 2);
      int64_t c;
      if (o.N - (i + 1) < 0) { c = 0; }
      else if (k > o.N - (i + 1)) { c = 0; }
      else if (k < 0) { c = 0; }
      else if (o.C[o.N - (i + 1)][static_cast<int64_t>(k)] != -1) { c = o.C[o.N - (i + 1)][static_cast<int64_t>(k)]; }
      else
      {
        c = VW::math::choose(o.N - (i + 1), static_cast<int64_t>(k));
        o.C[o.N - (i + 1)][static_cast<int64_t>(k)] = c;
      }

      float w = c * static_cast<float>(pow((0.5 + o.gamma), static_cast<double>(k))) *
          static_cast<float>(pow((double)0.5 - o.gamma, static_cast<double>(o.N - (i + 1) - k)));

      // update ec.weight, weight for learner i (starting from 0)
      ec.weight = u * w;

      base.predict(ec, i);

      // ec.pred.scalar is now the i-th learner prediction on this example
      s += ld.label * ec.pred.scalar;

      final_prediction += ec.pred.scalar;

      base.learn(ec, i);
    }
    else
    {
      base.predict(ec, i);
      final_prediction += ec.pred.scalar;
    }
  }

  ec.weight = u;
  ec.partial_prediction = final_prediction;
  ec.pred.scalar = VW::math::sign(final_prediction);

  if (ld.label == ec.pred.scalar) { ec.loss = 0.; }
  else { ec.loss = ec.weight; }
}

//-----------------------------------------------------------------
// Logistic boost
//-----------------------------------------------------------------
template <bool is_learn>
void predict_or_learn_logistic(boosting& o, VW::LEARNER::learner& base, VW::example& ec)
{
  auto& ld = ec.l.simple;

  float final_prediction = 0;

  float s = 0;
  float u = ec.weight;

  if (is_learn) { o.t++; }
  float eta = 4.f / sqrtf(static_cast<float>(o.t));

  for (int i = 0; i < o.N; i++)
  {
    if (is_learn)
    {
      float w = 1 / (1 + VW::details::correctedExp(s));

      ec.weight = u * w;

      base.predict(ec, i);
      float z;
      z = ld.label * ec.pred.scalar;

      s += z * o.alpha[i];

      // if ld.label * ec.pred.scalar < 0, learner i made a mistake

      final_prediction += ec.pred.scalar * o.alpha[i];

      // update alpha
      o.alpha[i] += eta * z / (1 + VW::details::correctedExp(s));
      if (o.alpha[i] > 2.) { o.alpha[i] = 2; }
      if (o.alpha[i] < -2.) { o.alpha[i] = -2; }

      base.learn(ec, i);
    }
    else
    {
      base.predict(ec, i);
      final_prediction += ec.pred.scalar * o.alpha[i];
    }
  }

  ec.weight = u;
  ec.partial_prediction = final_prediction;
  ec.pred.scalar = VW::math::sign(final_prediction);

  if (ld.label == ec.pred.scalar) { ec.loss = 0.; }
  else { ec.loss = ec.weight; }
}

template <bool is_learn>
void predict_or_learn_adaptive(boosting& o, VW::LEARNER::learner& base, VW::example& ec)
{
  auto& ld = ec.l.simple;

  float final_prediction = 0, partial_prediction = 0;

  float s = 0;
  float v_normalization = 0, v_partial_sum = 0;
  float u = ec.weight;

  if (is_learn) { o.t++; }
  float eta = 4.f / sqrtf(static_cast<float>(o.t));

  float stopping_point = o.random_state->get_and_update_random();

  for (int i = 0; i < o.N; i++)
  {
    if (is_learn)
    {
      float w = 1 / (1 + VW::details::correctedExp(s));

      ec.weight = u * w;

      base.predict(ec, i);
      float z;

      z = ld.label * ec.pred.scalar;

      s += z * o.alpha[i];

      if (v_partial_sum <= stopping_point) { final_prediction += ec.pred.scalar * o.alpha[i]; }

      partial_prediction += ec.pred.scalar * o.alpha[i];

      v_partial_sum += o.v[i];

      // update v, exp(-1) = 0.36788
      if (ld.label * partial_prediction < 0) { o.v[i] *= 0.36788f; }
      v_normalization += o.v[i];

      // update alpha
      o.alpha[i] += eta * z / (1 + VW::details::correctedExp(s));
      if (o.alpha[i] > 2.) { o.alpha[i] = 2; }
      if (o.alpha[i] < -2.) { o.alpha[i] = -2; }

      base.learn(ec, i);
    }
    else
    {
      base.predict(ec, i);
      if (v_partial_sum <= stopping_point) { final_prediction += ec.pred.scalar * o.alpha[i]; }
      else
      {
        // stopping at learner i
        break;
      }
      v_partial_sum += o.v[i];
    }
  }

  // normalize v vector in training
  if (is_learn)
  {
    for (int i = 0; i < o.N; i++)
    {
      if (v_normalization) { o.v[i] /= v_normalization; }
    }
  }

  ec.weight = u;
  ec.partial_prediction = final_prediction;
  ec.pred.scalar = VW::math::sign(final_prediction);

  if (ld.label == ec.pred.scalar) { ec.loss = 0.; }
  else { ec.loss = ec.weight; }
}

void save_load_sampling(boosting& o, VW::io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  std::stringstream os;
  os << "boosts " << o.N << endl;
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&(o.N)), sizeof(o.N), read, os, text);

  if (read)
  {
    o.alpha.resize(o.N);
    o.v.resize(o.N);
  }

  for (int i = 0; i < o.N; i++)
  {
    if (read)
    {
      float f;
      model_file.bin_read_fixed(reinterpret_cast<char*>(&f), sizeof(f));
      o.alpha[i] = f;
    }
    else
    {
      std::stringstream os2;
      os2 << "alpha " << o.alpha[i] << endl;
      VW::details::bin_text_write_fixed(
          model_file, reinterpret_cast<char*>(&(o.alpha[i])), sizeof(o.alpha[i]), os2, text);
    }
  }

  for (int i = 0; i < o.N; i++)
  {
    if (read)
    {
      float f;
      model_file.bin_read_fixed(reinterpret_cast<char*>(&f), sizeof(f));
      o.v[i] = f;
    }
    else
    {
      std::stringstream os2;
      os2 << "v " << o.v[i] << endl;
      VW::details::bin_text_write_fixed(model_file, reinterpret_cast<char*>(&(o.v[i])), sizeof(o.v[i]), os2, text);
    }
  }

  // avoid making syscalls multiple times
  fmt::memory_buffer buffer;
  if (read) { fmt::format_to(std::back_inserter(buffer), "Loading alpha and v: \n"); }
  else
  {
    fmt::format_to(std::back_inserter(buffer), "Saving alpha and v, current weighted_examples = {}\n",
        o.all->sd->weighted_labeled_examples + o.all->sd->weighted_unlabeled_examples);
  }

  for (int i = 0; i < o.N; i++) { fmt::format_to(std::back_inserter(buffer), "{0} {1}\n", o.alpha[i], o.v[i]); }
  o.logger.err_info("{}", fmt::to_string(buffer));
}

void save_load(boosting& o, VW::io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  std::stringstream os;
  os << "boosts " << o.N << endl;
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&(o.N)), sizeof(o.N), read, os, text);

  if (read) { o.alpha.resize(o.N); }

  for (int i = 0; i < o.N; i++)
  {
    if (read)
    {
      float f;
      model_file.bin_read_fixed(reinterpret_cast<char*>(&f), sizeof(f));
      o.alpha[i] = f;
    }
    else
    {
      std::stringstream os2;
      os2 << "alpha " << o.alpha[i] << endl;
      VW::details::bin_text_write_fixed(
          model_file, reinterpret_cast<char*>(&(o.alpha[i])), sizeof(o.alpha[i]), os2, text);
    }
  }

  if (!o.all->quiet)
  {
    // avoid making syscalls multiple times
    fmt::memory_buffer buffer;
    if (read) { fmt::format_to(std::back_inserter(buffer), "Loading alpha: \n"); }
    else
    {
      fmt::format_to(
          std::back_inserter(buffer), "Saving alpha, current weighted_examples = {}\n", o.all->sd->weighted_examples());
    }

    for (int i = 0; i < o.N; i++) { fmt::format_to(std::back_inserter(buffer), "{}\n", o.alpha[i]); }
    o.logger.err_info("{}", fmt::to_string(buffer));
  }
}

void save_load_boosting_noop(boosting&, VW::io_buf&, bool, bool) {}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::boosting_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<boosting>(all.logger);
  option_group_definition new_options("[Reduction] Boosting");
  new_options.add(make_option("boosting", data->N).keep().necessary().help("Online boosting with <N> weak learners"))
      .add(make_option("gamma", data->gamma)
               .default_value(0.1f)
               .help("Weak learner's edge (=0.1), used only by online BBM"))
      .add(
          make_option("alg", data->alg)
              .keep()
              .default_value("BBM")
              .one_of({"BBM", "logistic", "adaptive"})
              .help("Specify the boosting algorithm: BBM (default), logistic (AdaBoost.OL.W), adaptive (AdaBoost.OL)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Description of options:
  // "BBM" implements online BBM (Algorithm 1 in BLK'15)
  // "logistic" implements AdaBoost.OL.W (importance weighted version
  // 	    of Algorithm 2 in BLK'15)
  // "adaptive" implements AdaBoost.OL (Algorithm 2 in BLK'15,
  // 	    using sampling rather than importance weighting)

  all.logger.err_info("Number of weak learners = {}", data->N);
  all.logger.err_info("Gamma = {}", data->gamma);

  data->C = std::vector<std::vector<int64_t> >(data->N, std::vector<int64_t>(data->N, -1));
  data->t = 0;
  data->all = &all;
  data->random_state = all.get_random_state();
  data->alpha = std::vector<float>(data->N, 0);
  data->v = std::vector<float>(data->N, 1);

  size_t ws = data->N;
  std::string name_addition;
  void (*learn_ptr)(boosting&, VW::LEARNER::learner&, VW::example&);
  void (*pred_ptr)(boosting&, VW::LEARNER::learner&, VW::example&);
  void (*save_load_fn)(boosting&, io_buf&, bool, bool);

  if (data->alg == "BBM")
  {
    name_addition = "";
    learn_ptr = predict_or_learn<true>;
    pred_ptr = predict_or_learn<false>;
    save_load_fn = save_load_boosting_noop;
  }
  else if (data->alg == "logistic")
  {
    name_addition = "-logistic";
    learn_ptr = predict_or_learn_logistic<true>;
    pred_ptr = predict_or_learn_logistic<false>;
    save_load_fn = save_load;
  }
  else if (data->alg == "adaptive")
  {
    name_addition = "-adaptive";
    learn_ptr = predict_or_learn_adaptive<true>;
    pred_ptr = predict_or_learn_adaptive<false>;
    save_load_fn = save_load_sampling;
  }
  else { THROW("Unrecognized boosting algorithm: \'" << data->alg << "\'."); }

  auto l = make_reduction_learner(std::move(data), require_singleline(stack_builder.setup_base_learner()), learn_ptr,
      pred_ptr, stack_builder.get_setupfn_name(boosting_setup) + name_addition)
               .set_params_per_weight(ws)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(VW::prediction_type_t::SCALAR)
               .set_input_label_type(VW::label_type_t::SIMPLE)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_save_load(save_load_fn)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<boosting>)
               .set_update_stats(VW::details::update_stats_simple_label<boosting>)
               .set_print_update(VW::details::print_update_simple_label<boosting>)
               .build();

  return l;
}
