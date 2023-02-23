// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/oja_newton.h"

#include "vw/common/random.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/memory.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/prediction_type.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"

#include <cmath>
#include <memory>
#include <string>

using namespace VW::LEARNER;
using namespace VW::config;

#define NORM2 (m + 1)

namespace
{
class OjaNewton;
class oja_n_update_data
{
public:
  OjaNewton* oja_newton_ptr = nullptr;
  float g = 0.f;
  float sketch_cnt = 0.f;
  float norm2_x = 0.f;
  std::vector<float> Zx;   // NOLINT
  std::vector<float> AZx;  // NOLINT
  std::vector<float> delta;
  float bdelta = 0.f;
  float prediction = 0.f;
};

class OjaNewton
{
public:
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> random_state;
  int m = 0;
  uint64_t epoch_size = 0;
  float alpha = 0.f;
  uint64_t cnt = 0;
  int t = 0;

  std::vector<float> ev;
  std::vector<float> b;
  std::vector<float> D;               // NOLINT
  std::vector<std::vector<float>> A;  // NOLINT
  std::vector<std::vector<float>> K;  // NOLINT

  std::vector<float> zv;
  std::vector<float> vv;
  std::vector<float> tmp;

  std::vector<VW::example*> buffer;
  // If the epoch size is greater than 1, the examples in the batch need to be saved somewhere.
  std::vector<std::unique_ptr<VW::example>> saved_batch_examples;

  std::vector<float> weight_buffer;
  oja_n_update_data data;

  float learning_rate_cnt = 0.f;
  bool normalize = false;
  bool random_init = false;

  void initialize_Z(VW::parameters& weights)  // NOLINT
  {
    uint32_t length = 1 << all->num_bits;
    if (normalize)  // initialize normalization part
    {
      for (uint32_t i = 0; i < length; i++) { (&(weights.strided_index(i)))[NORM2] = 0.1f; }
    }
    if (!random_init)
    {
      // simple initialization
      for (int i = 1; i <= m; i++) { (&(weights.strided_index(i)))[i] = 1.f; }
    }
    else
    {
      // more complicated initialization: orthgonal basis of a random matrix

      static constexpr double PI2 = 2.f * 3.1415927f;

      for (uint32_t i = 0; i < length; i++)
      {
        VW::weight& w = weights.strided_index(i);
        float r1;
        float r2;
        for (int j = 1; j <= m; j++)
        {
          // box-muller tranform: https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
          // redraw until r1 should be strictly positive
          do {
            r1 = random_state->get_and_update_random();
            r2 = random_state->get_and_update_random();
          } while (r1 == 0.f);

          (&w)[j] = std::sqrt(-2.f * std::log(r1)) * static_cast<float>(cos(PI2 * r2));
        }
      }
    }

    // Gram-Schmidt
    for (int j = 1; j <= m; j++)
    {
      for (int k = 1; k <= j - 1; k++)
      {
        double temp = 0;

        for (uint32_t i = 0; i < length; i++)
        {
          temp += (static_cast<double>((&(weights.strided_index(i)))[j])) * (&(weights.strided_index(i)))[k];
        }
        for (uint32_t i = 0; i < length; i++)
        {
          (&(weights.strided_index(i)))[j] -= static_cast<float>(temp) * (&(weights.strided_index(i)))[k];
        }
      }
      double norm = 0;
      for (uint32_t i = 0; i < length; i++)
      {
        norm += (static_cast<double>((&(weights.strided_index(i)))[j])) * (&(weights.strided_index(i)))[j];
      }
      norm = std::sqrt(norm);
      for (uint32_t i = 0; i < length; i++) { (&(weights.strided_index(i)))[j] /= static_cast<float>(norm); }
    }
  }

  void compute_AZx()  // NOLINT
  {
    for (int i = 1; i <= m; i++)
    {
      data.AZx[i] = 0;
      for (int j = 1; j <= i; j++) { data.AZx[i] += A[i][j] * data.Zx[j]; }
    }
  }

  void update_eigenvalues()
  {
    for (int i = 1; i <= m; i++)
    {
      float gamma = std::fmin(learning_rate_cnt / t, 1.f);
      float temp = data.AZx[i] * data.sketch_cnt;

      if (t == 1) { ev[i] = gamma * temp * temp; }
      else { ev[i] = (1 - gamma) * t * ev[i] / (t - 1) + gamma * t * temp * temp; }
    }
  }

  void compute_delta()
  {
    data.bdelta = 0;
    for (int i = 1; i <= m; i++)
    {
      float gamma = std::fmin(learning_rate_cnt / t, 1.f);

      // if different learning rates are used
      /*data.delta[i] = gamma * data.AZx[i] * data.sketch_cnt;
      for (int j = 1; j < i; j++) {
          data.delta[i] -= A[i][j] * data.delta[j];
      }
      data.delta[i] /= A[i][i];*/

      // if a same learning rate is used
      data.delta[i] = gamma * data.Zx[i] * data.sketch_cnt;

      data.bdelta += data.delta[i] * b[i];
    }
  }

  void update_K()  // NOLINT
  {
    float temp = data.norm2_x * data.sketch_cnt * data.sketch_cnt;
    for (int i = 1; i <= m; i++)
    {
      for (int j = 1; j <= m; j++)
      {
        K[i][j] += data.delta[i] * data.Zx[j] * data.sketch_cnt;
        K[i][j] += data.delta[j] * data.Zx[i] * data.sketch_cnt;
        K[i][j] += data.delta[i] * data.delta[j] * temp;
      }
    }
  }

  void update_A()  // NOLINT
  {
    for (int i = 1; i <= m; i++)
    {
      for (int j = 1; j < i; j++)
      {
        zv[j] = 0;
        for (int k = 1; k <= i; k++) { zv[j] += A[i][k] * K[k][j]; }
      }

      for (int j = 1; j < i; j++)
      {
        vv[j] = 0;
        for (int k = 1; k <= j; k++) { vv[j] += A[j][k] * zv[k]; }
      }

      for (int j = 1; j < i; j++)
      {
        for (int k = j; k < i; k++) { A[i][j] -= vv[k] * A[k][j]; }
      }

      float norm = 0;
      for (int j = 1; j <= i; j++)
      {
        float temp = 0;
        for (int k = 1; k <= i; k++) { temp += K[j][k] * A[i][k]; }
        norm += A[i][j] * temp;
      }
      norm = sqrtf(norm);

      for (int j = 1; j <= i; j++) { A[i][j] /= norm; }
    }
  }

  void update_b()
  {
    for (int j = 1; j <= m; j++)
    {
      float temp = 0;
      for (int i = j; i <= m; i++) { temp += ev[i] * data.AZx[i] * A[i][j] / (alpha * (alpha + ev[i])); }
      b[j] += temp * data.g;
    }
  }

  void update_D()  // NOLINT
  {
    for (int j = 1; j <= m; j++)
    {
      float scale = std::fabs(A[j][j]);
      for (int i = j + 1; i <= m; i++) { scale = std::fmin(std::fabs(A[i][j]), scale); }
      if (scale < 1e-10) { continue; }
      for (int i = 1; i <= m; i++)
      {
        A[i][j] /= scale;
        K[j][i] *= scale;
        K[i][j] *= scale;
      }
      b[j] /= scale;
      D[j] *= scale;
      // printf("D[%d] = %f\n", j, D[j]);
    }
  }

  void check()
  {
    double max_norm = 0;
    for (int i = 1; i <= m; i++)
    {
      for (int j = i; j <= m; j++) { max_norm = fmax(max_norm, std::fabs(K[i][j])); }
    }
    // printf("|K| = %f\n", max_norm);
    if (max_norm < 1e7) { return; }

    // implicit -> explicit representation
    // printf("begin conversion: t = %d, norm(K) = %f\n", t, max_norm);

    // first step: K <- AKA'

    // K <- AK
    for (int j = 1; j <= m; j++)
    {
      std::fill(tmp.begin(), tmp.end(), 0.f);

      for (int i = 1; i <= m; i++)
      {
        for (int h = 1; h <= m; h++) { tmp[i] += A[i][h] * K[h][j]; }
      }

      for (int i = 1; i <= m; i++) { K[i][j] = tmp[i]; }
    }
    // K <- KA'
    for (int i = 1; i <= m; i++)
    {
      std::fill(tmp.begin(), tmp.end(), 0.f);

      for (int j = 1; j <= m; j++)
      {
        for (int h = 1; h <= m; h++) { tmp[j] += K[i][h] * A[j][h]; }
      }

      for (int j = 1; j <= m; j++) { K[i][j] = tmp[j]; }
    }

    // second step: w[0] <- w[0] + (DZ)'b, b <- 0.

    uint32_t length = 1 << all->num_bits;
    for (uint32_t i = 0; i < length; i++)
    {
      VW::weight& w = all->weights.strided_index(i);
      for (int j = 1; j <= m; j++) { w += (&w)[j] * b[j] * D[j]; }
    }

    std::fill(b.begin(), b.end(), 0.f);

    // third step: Z <- ADZ, A, D <- Identity

    // double norm = 0;
    for (uint32_t i = 0; i < length; ++i)
    {
      std::fill(tmp.begin(), tmp.end(), 0.f);
      VW::weight& w = all->weights.strided_index(i);
      for (int j = 1; j <= m; j++)
      {
        for (int h = 1; h <= m; ++h) { tmp[j] += A[j][h] * D[h] * (&w)[h]; }
      }
      for (int j = 1; j <= m; ++j)
      {
        // norm = std::max(norm, fabs(tmp[j]));
        (&w)[j] = tmp[j];
      }
    }
    // printf("|Z| = %f\n", norm);

    for (int i = 1; i <= m; i++)
    {
      std::fill(A[i].begin(), A[i].end(), 0.f);
      D[i] = 1;
      A[i][i] = 1;
    }
  }
};

void make_pred(oja_n_update_data& data, float x, float& wref)
{
  int m = data.oja_newton_ptr->m;
  float* w = &wref;

  if (data.oja_newton_ptr->normalize) { x /= std::sqrt(w[NORM2]); }

  data.prediction += w[0] * x;
  for (int i = 1; i <= m; i++) { data.prediction += w[i] * x * data.oja_newton_ptr->D[i] * data.oja_newton_ptr->b[i]; }
}

void predict(OjaNewton& oja_newton_ptr, VW::example& ec)
{
  oja_newton_ptr.data.prediction = 0;
  VW::foreach_feature<oja_n_update_data, make_pred>(*oja_newton_ptr.all, ec, oja_newton_ptr.data);
  ec.partial_prediction = oja_newton_ptr.data.prediction;
  ec.pred.scalar =
      VW::details::finalize_prediction(*oja_newton_ptr.all->sd, oja_newton_ptr.all->logger, ec.partial_prediction);
}

void update_Z_and_wbar(oja_n_update_data& data, float x, float& wref)  // NOLINT
{
  float* w = &wref;
  int m = data.oja_newton_ptr->m;
  if (data.oja_newton_ptr->normalize) { x /= std::sqrt(w[NORM2]); }
  float s = data.sketch_cnt * x;

  for (int i = 1; i <= m; i++) { w[i] += data.delta[i] * s / data.oja_newton_ptr->D[i]; }
  w[0] -= s * data.bdelta;
}

void compute_Zx_and_norm(oja_n_update_data& data, float x, float& wref)  // NOLINT
{
  float* w = &wref;
  int m = data.oja_newton_ptr->m;
  if (data.oja_newton_ptr->normalize) { x /= std::sqrt(w[NORM2]); }

  for (int i = 1; i <= m; i++) { data.Zx[i] += w[i] * x * data.oja_newton_ptr->D[i]; }
  data.norm2_x += x * x;
}

void update_wbar_and_Zx(oja_n_update_data& data, float x, float& wref)  // NOLINT
{
  float* w = &wref;
  int m = data.oja_newton_ptr->m;
  if (data.oja_newton_ptr->normalize) { x /= std::sqrt(w[NORM2]); }

  float g = data.g * x;

  for (int i = 1; i <= m; i++) { data.Zx[i] += w[i] * x * data.oja_newton_ptr->D[i]; }
  w[0] -= g / data.oja_newton_ptr->alpha;
}

void update_normalization(oja_n_update_data& data, float x, float& wref)
{
  float* w = &wref;
  int m = data.oja_newton_ptr->m;

  w[NORM2] += x * x * data.g * data.g;
}

void learn(OjaNewton& oja_newton_ptr, VW::example& ec)
{
  // predict
  predict(oja_newton_ptr, ec);

  oja_n_update_data& data = oja_newton_ptr.data;
  data.g = oja_newton_ptr.all->loss->first_derivative(oja_newton_ptr.all->sd.get(), ec.pred.scalar, ec.l.simple.label) *
      ec.weight;
  data.g /= 2;  // for half square loss

  if (oja_newton_ptr.normalize)
  {
    VW::foreach_feature<oja_n_update_data, update_normalization>(*oja_newton_ptr.all, ec, data);
  }

  VW::example* next_in_batch = nullptr;
  // If the size is 1, we can just use the example directly.
  if (oja_newton_ptr.epoch_size == 1) { next_in_batch = &ec; }
  else
  {
    // If the batch size is greater than 1, we must make a copy of the example as its lifetime doesn't go beyond this
    // function.
    VW::copy_example_data_with_label(oja_newton_ptr.saved_batch_examples[oja_newton_ptr.cnt].get(), &ec);
    next_in_batch = oja_newton_ptr.saved_batch_examples[oja_newton_ptr.cnt].get();
  }
  assert(next_in_batch != nullptr);
  oja_newton_ptr.weight_buffer[oja_newton_ptr.cnt] = data.g / 2;
  oja_newton_ptr.buffer[oja_newton_ptr.cnt] = next_in_batch;
  oja_newton_ptr.cnt++;

  // If we've reached the batch size then perform the batch processing.
  if (oja_newton_ptr.cnt == oja_newton_ptr.epoch_size)
  {
    assert(oja_newton_ptr.buffer.size() == oja_newton_ptr.epoch_size);
    assert(oja_newton_ptr.weight_buffer.size() == oja_newton_ptr.epoch_size);
    assert(oja_newton_ptr.epoch_size == 1 || (oja_newton_ptr.saved_batch_examples.size() == oja_newton_ptr.epoch_size));

    for (uint64_t k = 0; k < oja_newton_ptr.epoch_size; k++, oja_newton_ptr.t++)
    {
      VW::example& ex = *(oja_newton_ptr.buffer[k]);
      data.sketch_cnt = oja_newton_ptr.weight_buffer[k];

      data.norm2_x = 0;
      std::fill(data.Zx.begin(), data.Zx.end(), 0.f);
      VW::foreach_feature<oja_n_update_data, compute_Zx_and_norm>(*oja_newton_ptr.all, ex, data);
      oja_newton_ptr.compute_AZx();

      oja_newton_ptr.update_eigenvalues();
      oja_newton_ptr.compute_delta();

      oja_newton_ptr.update_K();

      VW::foreach_feature<oja_n_update_data, update_Z_and_wbar>(*oja_newton_ptr.all, ex, data);
    }

    oja_newton_ptr.update_A();
    // oja_newton_ptr.update_D();

    // Reset count for next batch.
    oja_newton_ptr.cnt = 0;
  }

  std::fill(data.Zx.begin(), data.Zx.end(), 0.f);
  VW::foreach_feature<oja_n_update_data, update_wbar_and_Zx>(*oja_newton_ptr.all, ec, data);
  oja_newton_ptr.compute_AZx();

  oja_newton_ptr.update_b();
  oja_newton_ptr.check();
}

void save_load(OjaNewton& oja_newton_ptr, VW::io_buf& model_file, bool read, bool text)
{
  VW::workspace& all = *oja_newton_ptr.all;
  if (read)
  {
    VW::details::initialize_regressor(all);
    oja_newton_ptr.initialize_Z(all.weights);
  }

  if (model_file.num_files() > 0)
  {
    bool resume = all.save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&resume), sizeof(resume), read, msg, text);

    double temp = 0.;
    double temp_normalized_sum_norm_x = 0.;
    if (resume)
    {
      VW::details::save_load_online_state_gd(all, model_file, read, text, temp, temp_normalized_sum_norm_x);
    }
    else { VW::details::save_load_regressor_gd(all, model_file, read, text); }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::oja_newton_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto oja_newton_ptr = VW::make_unique<OjaNewton>();

  bool oja_newton;
  float alpha_inverse;

  // These two are the only two boolean options that default to true. For now going to do this hack
  // as the infrastructure doesn't easily support this possibility at the same time providing the
  // ease of bool switches elsewhere. It seems that the switch behavior is more critical because
  // of the positional data argument.
  std::string normalize = "true";
  std::string random_init = "true";
  option_group_definition new_options("[Reduction] OjaNewton");
  new_options.add(make_option("OjaNewton", oja_newton).keep().necessary().help("Online Newton with Oja's Sketch"))
      .add(make_option("sketch_size", oja_newton_ptr->m).default_value(10).help("Size of sketch"))
      .add(make_option("epoch_size", oja_newton_ptr->epoch_size).default_value(1).help("Size of epoch"))
      .add(make_option("alpha", oja_newton_ptr->alpha).default_value(1.f).help("Mutiplicative constant for indentiy"))
      .add(make_option("alpha_inverse", alpha_inverse).help("One over alpha, similar to learning rate"))
      .add(make_option("learning_rate_cnt", oja_newton_ptr->learning_rate_cnt)
               .default_value(2.f)
               .help("Constant for the learning rate 1/t"))
      .add(make_option("normalize", normalize).help("Normalize the features or not"))
      .add(make_option("random_init", random_init).help("Randomize initialization of Oja or not"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  oja_newton_ptr->all = &all;
  oja_newton_ptr->random_state = all.get_random_state();

  oja_newton_ptr->normalize = normalize == "true";
  oja_newton_ptr->random_init = random_init == "true";

  if (options.was_supplied("alpha_inverse")) { oja_newton_ptr->alpha = 1.f / alpha_inverse; }

  const size_t buffer_size = oja_newton_ptr->m + 1;
  oja_newton_ptr->cnt = 0;
  oja_newton_ptr->t = 1;
  oja_newton_ptr->ev = std::vector<float>(buffer_size, 0.f);
  oja_newton_ptr->b = std::vector<float>(buffer_size, 0.f);
  oja_newton_ptr->D = std::vector<float>(buffer_size, 1.f);
  oja_newton_ptr->A = std::vector<std::vector<float>>(buffer_size, std::vector<float>(buffer_size, 0.f));
  oja_newton_ptr->K = std::vector<std::vector<float>>(buffer_size, std::vector<float>(buffer_size, 0.f));
  for (int i = 1; i <= oja_newton_ptr->m; i++)
  {
    oja_newton_ptr->A[i][i] = 1.f;
    oja_newton_ptr->K[i][i] = 1.f;
  }

  oja_newton_ptr->buffer = std::vector<VW::example*>(oja_newton_ptr->epoch_size, nullptr);
  oja_newton_ptr->weight_buffer = std::vector<float>(oja_newton_ptr->epoch_size, 0.f);
  if (oja_newton_ptr->epoch_size > 1)
  {
    oja_newton_ptr->saved_batch_examples.reserve(oja_newton_ptr->epoch_size);
    for (uint64_t i = 0; i < oja_newton_ptr->epoch_size; i++)
    {
      oja_newton_ptr->saved_batch_examples.emplace_back(VW::make_unique<VW::example>());
    }
  }

  oja_newton_ptr->zv = std::vector<float>(buffer_size, 0.f);
  oja_newton_ptr->vv = std::vector<float>(buffer_size, 0.f);
  oja_newton_ptr->tmp = std::vector<float>(buffer_size, 0.f);

  oja_newton_ptr->data.oja_newton_ptr = oja_newton_ptr.get();
  oja_newton_ptr->data.Zx = std::vector<float>(buffer_size, 0.f);
  oja_newton_ptr->data.AZx = std::vector<float>(buffer_size, 0.f);
  oja_newton_ptr->data.delta = std::vector<float>(buffer_size, 0.f);

  all.weights.stride_shift(static_cast<uint32_t>(std::ceil(std::log2(oja_newton_ptr->m + 2))));

  auto l = make_bottom_learner(std::move(oja_newton_ptr), learn, predict,
      stack_builder.get_setupfn_name(oja_newton_setup), VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
               .set_params_per_weight(all.weights.stride())
               .set_save_load(save_load)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<OjaNewton>)
               .set_update_stats(VW::details::update_stats_simple_label<OjaNewton>)
               .set_print_update(VW::details::print_update_simple_label<OjaNewton>)
               .build();

  return l;
}
