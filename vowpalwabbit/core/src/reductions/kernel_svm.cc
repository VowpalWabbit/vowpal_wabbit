// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/kernel_svm.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/accumulate.h"
#include "vw/core/constant.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/version.h"
#include "vw/core/vw.h"
#include "vw/core/vw_allreduce.h"
#include "vw/core/vw_versions.h"
#include "vw/io/logger.h"

#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>

#define SVM_KER_LIN 0
#define SVM_KER_RBF 1
#define SVM_KER_POLY 2

using namespace VW::LEARNER;
using namespace VW::config;

using std::endl;

namespace
{
class svm_params;

static size_t num_kernel_evals = 0;
static size_t num_cache_evals = 0;

class svm_example
{
public:
  VW::v_array<float> krow;
  VW::flat_example ex;

  ~svm_example();
  void init_svm_example(VW::flat_example* fec);
  int compute_kernels(svm_params& params);
  int clear_kernels();
};

class svm_model
{
public:
  size_t num_support;
  VW::v_array<svm_example*> support_vec;
  VW::v_array<float> alpha;
  VW::v_array<float> delta;
};

void free_svm_model(svm_model* model)
{
  for (size_t i = 0; i < model->num_support; i++)
  {
    model->support_vec[i]->~svm_example();
    // Warning C6001 is triggered by the following:
    // example is allocated using (a) '&VW::details::calloc_or_throw<svm_example>()' and freed using (b)
    // 'free(model->support_vec[i])'
    //
    // When the call to allocation is replaced by (a) 'new svm_example()' and deallocated using (b) 'operator delete
    // (model->support_vect[i])', the warning goes away. Disable SDL warning.
    //    #pragma warning(disable:6001)
    free(model->support_vec[i]);
    //  #pragma warning(default:6001)

    model->support_vec[i] = nullptr;
  }

  model->~svm_model();
  free(model);
}

class svm_params
{
public:
  size_t current_pass = 0;
  bool active = false;
  bool active_pool_greedy = false;
  bool para_active = false;
  double active_c = 0.0;

  size_t pool_size = 0;
  size_t pool_pos = 0;
  size_t subsample = 0;  // NOTE: Eliminating subsample to only support 1/pool_size
  uint64_t reprocess = 0;

  svm_model* model = nullptr;
  size_t maxcache = 0;
  // size_t curcache;

  svm_example** pool = nullptr;
  float lambda = 0.f;

  void* kernel_params = nullptr;
  size_t kernel_type = 0;

  size_t local_begin = 0;
  size_t local_end = 0;
  size_t current_t = 0;

  float loss_sum = 0.f;

  VW::workspace* all = nullptr;  // flatten, parallel
  std::shared_ptr<VW::rand_state> random_state;

  ~svm_params()
  {
    free(pool);
    if (model) { free_svm_model(model); }
    free(kernel_params);
  }
};

void svm_example::init_svm_example(VW::flat_example* fec)
{
  ex = std::move(*fec);
  free(fec);
}

svm_example::~svm_example()
{
  // free flatten example contents
  // VW::flat_example* fec = &VW::details::calloc_or_throw<VW::flat_example>();
  //*fec = ex;
  // free_flatten_example(fec);  // free contents of flat example and frees fec.
}

float kernel_function(const VW::flat_example* fec1, const VW::flat_example* fec2, void* params, size_t kernel_type);

int svm_example::compute_kernels(svm_params& params)
{
  int alloc = 0;
  svm_model* model = params.model;
  size_t n = model->num_support;

  if (krow.size() < n)
  {
    // computing new kernel values and caching them
    // if(params->curcache + n > params->maxcache)
    // trim_cache(params);
    num_kernel_evals += krow.size();
    for (size_t i = krow.size(); i < n; i++)
    {
      svm_example* sec = model->support_vec[i];
      float kv = kernel_function(&ex, &(sec->ex), params.kernel_params, params.kernel_type);
      krow.push_back(kv);
      alloc += 1;
    }
  }
  else { num_cache_evals += n; }
  return alloc;
}

int svm_example::clear_kernels()
{
  int rowsize = static_cast<int>(krow.size());
  krow.clear();
  return -rowsize;
}

static int make_hot_sv(svm_params& params, size_t svi)
{
  svm_model* model = params.model;
  size_t n = model->num_support;
  if (svi >= model->num_support) { params.all->logger.err_error("Internal error at {}:{}", __FILE__, __LINE__); }
  // rotate params fields
  svm_example* svi_e = model->support_vec[svi];
  int alloc = svi_e->compute_kernels(params);
  float svi_alpha = model->alpha[svi];
  float svi_delta = model->delta[svi];
  for (size_t i = svi; i > 0; --i)
  {
    model->support_vec[i] = model->support_vec[i - 1];
    model->alpha[i] = model->alpha[i - 1];
    model->delta[i] = model->delta[i - 1];
  }
  model->support_vec[0] = svi_e;
  model->alpha[0] = svi_alpha;
  model->delta[0] = svi_delta;
  // rotate cache
  for (size_t j = 0; j < n; j++)
  {
    svm_example* e = model->support_vec[j];
    size_t rowsize = e->krow.size();
    if (svi < rowsize)
    {
      float kv = e->krow[svi];
      for (size_t i = svi; i > 0; --i) { e->krow[i] = e->krow[i - 1]; }
      e->krow[0] = kv;
    }
    else
    {
      float kv = svi_e->krow[j];
      e->krow.push_back(0);
      alloc += 1;
      for (size_t i = e->krow.size() - 1; i > 0; --i) { e->krow[i] = e->krow[i - 1]; }
      e->krow[0] = kv;
    }
  }
  return alloc;
}

static int trim_cache(svm_params& params)
{
  int sz = static_cast<int>(params.maxcache);
  svm_model* model = params.model;
  size_t n = model->num_support;
  int alloc = 0;
  for (size_t i = 0; i < n; i++)
  {
    svm_example* e = model->support_vec[i];
    sz -= static_cast<int>(e->krow.size());
    if (sz < 0) { alloc += e->clear_kernels(); }
  }
  return alloc;
}

void save_load_svm_model(svm_params& params, VW::io_buf& model_file, bool read, bool text)
{
  svm_model* model = params.model;
  // TODO: check about initialization

  if (model_file.num_files() == 0) { return; }
  std::stringstream msg;
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&(model->num_support)), sizeof(model->num_support), read, msg, text);

  if (read) { model->support_vec.reserve(model->num_support); }

  for (uint32_t i = 0; i < model->num_support; i++)
  {
    if (read)
    {
      auto fec = VW::make_unique<VW::flat_example>();
      auto* tmp = &VW::details::calloc_or_throw<svm_example>();
      VW::model_utils::read_model_field(model_file, *fec, params.all->example_parser->lbl_parser);
      tmp->ex = *fec;
      model->support_vec.push_back(tmp);
    }
    else
    {
      VW::model_utils::write_model_field(model_file, model->support_vec[i]->ex, "_flat_example", false,
          params.all->example_parser->lbl_parser, params.all->parse_mask);
    }
  }

  if (read) { model->alpha.resize(model->num_support); }
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(model->alpha.data()),
      static_cast<uint32_t>(model->num_support) * sizeof(float), read, msg, text);
  if (read) { model->delta.resize(model->num_support); }
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(model->delta.data()),
      static_cast<uint32_t>(model->num_support) * sizeof(float), read, msg, text);
}

void save_load(svm_params& params, VW::io_buf& model_file, bool read, bool text)
{
  if (text)
  {
    *params.all->trace_message << "Not supporting readable model for kernel svm currently" << endl;
    return;
  }
  else if (params.all->model_file_ver > VW::version_definitions::EMPTY_VERSION_FILE &&
      params.all->model_file_ver < VW::version_definitions::VERSION_FILE_WITH_FLAT_EXAMPLE_TAG_FIX)
  {
    THROW("Models using ksvm from before version 9.6 are not compatable with this version of VW.")
  }

  save_load_svm_model(params, model_file, read, text);
}

float linear_kernel(const VW::flat_example* fec1, const VW::flat_example* fec2)
{
  float dotprod = 0;

  auto& fs_1 = const_cast<VW::features&>(fec1->fs);
  auto& fs_2 = const_cast<VW::features&>(fec2->fs);
  if (fs_2.indices.size() == 0) { return 0.f; }

  for (size_t idx1 = 0, idx2 = 0; idx1 < fs_1.size() && idx2 < fs_2.size(); idx1++)
  {
    uint64_t ec1pos = fs_1.indices[idx1];
    uint64_t ec2pos = fs_2.indices[idx2];
    // params.all->opts_n_args.trace_message<<ec1pos<<" "<<ec2pos<<" "<<idx1<<" "<<idx2<<" "<<f->x<<" "<<ec2f->x<< endl;
    if (ec1pos < ec2pos) { continue; }

    while (ec1pos > ec2pos && ++idx2 < fs_2.size()) { ec2pos = fs_2.indices[idx2]; }

    if (ec1pos == ec2pos)
    {
      dotprod += fs_1.values[idx1] * fs_2.values[idx2];
      ++idx2;
    }
  }
  return dotprod;
}

float poly_kernel(const VW::flat_example* fec1, const VW::flat_example* fec2, int power)
{
  float dotprod = linear_kernel(fec1, fec2);
  return static_cast<float>(std::pow(1 + dotprod, power));
}

float rbf_kernel(const VW::flat_example* fec1, const VW::flat_example* fec2, float bandwidth)
{
  float dotprod = linear_kernel(fec1, fec2);
  return expf(-(fec1->total_sum_feat_sq + fec2->total_sum_feat_sq - 2 * dotprod) * bandwidth);
}

float kernel_function(const VW::flat_example* fec1, const VW::flat_example* fec2, void* params, size_t kernel_type)
{
  switch (kernel_type)
  {
    case SVM_KER_RBF:
      return rbf_kernel(fec1, fec2, *(static_cast<float*>(params)));
    case SVM_KER_POLY:
      return poly_kernel(fec1, fec2, *(static_cast<int*>(params)));
    case SVM_KER_LIN:
      return linear_kernel(fec1, fec2);
  }
  return 0;
}

float dense_dot(float* v1, const VW::v_array<float>& v2, size_t n)
{
  float dot_prod = 0.;
  for (size_t i = 0; i < n; i++) { dot_prod += v1[i] * v2[i]; }
  return dot_prod;
}

void predict(svm_params& params, svm_example** ec_arr, float* scores, size_t n)
{
  svm_model* model = params.model;
  for (size_t i = 0; i < n; i++)
  {
    ec_arr[i]->compute_kernels(params);
    // std::cout<<"size of krow = "<<ec_arr[i]->krow.size()<< endl;
    if (ec_arr[i]->krow.size() > 0)
    {
      scores[i] = dense_dot(ec_arr[i]->krow.begin(), model->alpha, model->num_support) / params.lambda;
    }
    else { scores[i] = 0; }
  }
}

void predict(svm_params& params, VW::example& ec)
{
  VW::flat_example* fec = VW::flatten_sort_example(*(params.all), &ec);
  if (fec)
  {
    svm_example* sec = &VW::details::calloc_or_throw<svm_example>();
    sec->init_svm_example(fec);
    float score;
    predict(params, &sec, &score, 1);
    ec.pred.scalar = score;
    sec->~svm_example();
    free(sec);
  }
}

size_t suboptimality(svm_model* model, double* subopt)
{
  size_t max_pos = 0;
  double max_val = 0;
  for (size_t i = 0; i < model->num_support; i++)
  {
    float tmp = model->alpha[i] * model->support_vec[i]->ex.l.simple.label;
    const auto& simple_red_features =
        model->support_vec[i]->ex.ex_reduction_features.template get<VW::simple_label_reduction_features>();
    if ((tmp < simple_red_features.weight && model->delta[i] < 0) || (tmp > 0 && model->delta[i] > 0))
    {
      subopt[i] = fabs(model->delta[i]);
    }
    else { subopt[i] = 0; }

    if (subopt[i] > max_val)
    {
      max_val = subopt[i];
      max_pos = i;
    }
  }
  return max_pos;
}

int remove(svm_params& params, size_t svi)
{
  svm_model* model = params.model;
  if (svi >= model->num_support) { params.all->logger.err_error("Internal error at {}:{}", __FILE__, __LINE__); }
  // shift params fields
  svm_example* svi_e = model->support_vec[svi];
  for (size_t i = svi; i < model->num_support - 1; ++i)
  {
    model->support_vec[i] = model->support_vec[i + 1];
    model->alpha[i] = model->alpha[i + 1];
    model->delta[i] = model->delta[i + 1];
  }
  svi_e->~svm_example();
  free(svi_e);
  model->support_vec.pop_back();
  model->alpha.pop_back();
  model->delta.pop_back();
  model->num_support--;
  // shift cache
  int alloc = 0;
  for (size_t j = 0; j < model->num_support; j++)
  {
    svm_example* e = model->support_vec[j];
    size_t rowsize = e->krow.size();
    if (svi < rowsize)
    {
      for (size_t i = svi; i < rowsize - 1; i++) { e->krow[i] = e->krow[i + 1]; }
      e->krow.pop_back();
      alloc -= 1;
    }
  }
  return alloc;
}

int add(svm_params& params, svm_example* fec)
{
  svm_model* model = params.model;
  model->num_support++;
  model->support_vec.push_back(fec);
  model->alpha.push_back(0.);
  model->delta.push_back(0.);
  return static_cast<int>(model->support_vec.size() - 1);
}

bool update(svm_params& params, size_t pos)
{
  svm_model* model = params.model;
  bool overshoot = false;
  svm_example* fec = model->support_vec[pos];
  auto& ld = fec->ex.l.simple;
  fec->compute_kernels(params);
  float* inprods = fec->krow.begin();
  float alphaKi = dense_dot(inprods, model->alpha, model->num_support);  // NOLINT
  model->delta[pos] = alphaKi * ld.label / params.lambda - 1;
  float alpha_old = model->alpha[pos];
  alphaKi -= model->alpha[pos] * inprods[pos];
  model->alpha[pos] = 0.;

  float proj = alphaKi * ld.label;
  float ai = (params.lambda - proj) / inprods[pos];

  const auto& simple_red_features = fec->ex.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  if (ai > simple_red_features.weight) { ai = simple_red_features.weight; }
  else if (ai < 0) { ai = 0; }

  ai *= ld.label;
  float diff = ai - alpha_old;

  if (std::fabs(diff) > 1.0e-06) { overshoot = true; }

  if (std::fabs(diff) > 1.)
  {
    diff = static_cast<float>(diff > 0) - (diff < 0);
    ai = alpha_old + diff;
  }

  for (size_t i = 0; i < model->num_support; i++)
  {
    auto& ldi = model->support_vec[i]->ex.l.simple;
    model->delta[i] += diff * inprods[i] * ldi.label / params.lambda;
  }

  if (std::fabs(ai) <= 1.0e-10) { remove(params, pos); }
  else { model->alpha[pos] = ai; }

  return overshoot;
}

void copy_char(char& c1, const char& c2) noexcept
{
  if (c2 != '\0') { c1 = c2; }
}

void add_size_t(size_t& t1, const size_t& t2) noexcept { t1 += t2; }

void sync_queries(VW::workspace& all, svm_params& params, bool* train_pool)
{
  VW::io_buf* b = new VW::io_buf();

  char* queries;
  VW::flat_example* fec = nullptr;

  for (size_t i = 0; i < params.pool_pos; i++)
  {
    if (!train_pool[i]) { continue; }

    fec = &(params.pool[i]->ex);
    VW::model_utils::write_model_field(
        *b, *fec, "_flat_example", false, all.example_parser->lbl_parser, all.parse_mask);
    delete params.pool[i];
  }

  size_t* sizes = VW::details::calloc_or_throw<size_t>(all.all_reduce->total);
  sizes[all.all_reduce->node] = b->unflushed_bytes_count();
  VW::details::all_reduce<size_t, add_size_t>(all, sizes, all.all_reduce->total);

  size_t prev_sum = 0, total_sum = 0;
  for (size_t i = 0; i < all.all_reduce->total; i++)
  {
    if (i <= (all.all_reduce->node - 1)) { prev_sum += sizes[i]; }
    total_sum += sizes[i];
  }

  if (total_sum > 0)
  {
    queries = VW::details::calloc_or_throw<char>(total_sum);
    size_t bytes_copied = b->copy_to(queries + prev_sum, total_sum - prev_sum);
    if (bytes_copied < b->unflushed_bytes_count()) THROW("kernel_svm: Failed to alloc enough space.");

    VW::details::all_reduce<char, copy_char>(all, queries, total_sum);
    b->replace_buffer(queries, total_sum);

    size_t num_read = 0;
    params.pool_pos = 0;

    for (size_t i = 0; i < params.pool_size; i++)
    {
      if (!VW::model_utils::read_model_field(*b, *fec, all.example_parser->lbl_parser))
      {
        params.pool[i] = &VW::details::calloc_or_throw<svm_example>();
        params.pool[i]->init_svm_example(fec);
        train_pool[i] = true;
        params.pool_pos++;
      }
      else { break; }

      num_read += b->unflushed_bytes_count();
      if (num_read == prev_sum) { params.local_begin = i + 1; }
      if (num_read == prev_sum + sizes[all.all_reduce->node]) { params.local_end = i; }
    }
  }
  if (fec) { free(fec); }
  free(sizes);
  delete b;
}

void train(svm_params& params)
{
  bool* train_pool = VW::details::calloc_or_throw<bool>(params.pool_size);
  for (size_t i = 0; i < params.pool_size; i++) { train_pool[i] = false; }

  float* scores = VW::details::calloc_or_throw<float>(params.pool_pos);
  predict(params, params.pool, scores, params.pool_pos);

  if (params.active)
  {
    if (params.active_pool_greedy)
    {
      std::multimap<double, size_t> scoremap;
      for (size_t i = 0; i < params.pool_pos; i++)
      {
        scoremap.insert(std::pair<const double, const size_t>(std::fabs(scores[i]), i));
      }

      std::multimap<double, size_t>::iterator iter = scoremap.begin();
      iter = scoremap.begin();

      for (size_t train_size = 1; iter != scoremap.end() && train_size <= params.subsample; train_size++)
      {
        train_pool[iter->second] = 1;
        iter++;
      }
    }
    else
    {
      for (size_t i = 0; i < params.pool_pos; i++)
      {
        float queryp = 2.0f /
            (1.0f +
                expf(static_cast<float>(params.active_c * std::fabs(scores[i])) *
                    static_cast<float>(pow(params.pool[i]->ex.example_counter, 0.5f))));
        if (params.random_state->get_and_update_random() < queryp)
        {
          svm_example* fec = params.pool[i];
          auto& simple_red_features = fec->ex.ex_reduction_features.template get<VW::simple_label_reduction_features>();
          simple_red_features.weight *= 1 / queryp;
          train_pool[i] = 1;
        }
      }
    }
    // free(scores);
  }

  if (params.para_active)
  {
    for (size_t i = 0; i < params.pool_pos; i++)
    {
      if (!train_pool[i]) { delete params.pool[i]; }
    }
    sync_queries(*(params.all), params, train_pool);
  }

  if (params.all->training)
  {
    svm_model* model = params.model;

    for (size_t i = 0; i < params.pool_pos; i++)
    {
      int model_pos = -1;
      if (params.active)
      {
        if (train_pool[i]) { model_pos = add(params, params.pool[i]); }
      }
      else { model_pos = add(params, params.pool[i]); }

      if (model_pos >= 0)
      {
        bool overshoot = update(params, model_pos);

        double* subopt = VW::details::calloc_or_throw<double>(model->num_support);
        for (size_t j = 0; j < params.reprocess; j++)
        {
          if (model->num_support == 0) { break; }
          int randi = 1;
          if (params.random_state->get_and_update_random() < 0.5) { randi = 0; }
          if (randi)
          {
            size_t max_pos = suboptimality(model, subopt);
            if (subopt[max_pos] > 0)
            {
              if (!overshoot && max_pos == static_cast<size_t>(model_pos) && max_pos > 0 && j == 0)
              {
                *params.all->trace_message << "Shouldn't reprocess right after process." << endl;
              }
              if (max_pos * model->num_support <= params.maxcache) { make_hot_sv(params, max_pos); }
              update(params, max_pos);
            }
          }
          else
          {
            size_t rand_pos =
                static_cast<size_t>(floorf(params.random_state->get_and_update_random() * model->num_support));
            update(params, rand_pos);
          }
        }
        free(subopt);
      }
    }
  }
  else
  {
    for (size_t i = 0; i < params.pool_pos; i++) { delete params.pool[i]; }
  }

  free(scores);
  free(train_pool);
}

void learn(svm_params& params, VW::example& ec)
{
  VW::flat_example* fec = VW::flatten_sort_example(*(params.all), &ec);
  if (fec)
  {
    svm_example* sec = &VW::details::calloc_or_throw<svm_example>();
    sec->init_svm_example(fec);
    float score = 0;
    predict(params, &sec, &score, 1);
    ec.pred.scalar = score;
    ec.loss = std::max(0.f, 1.f - score * ec.l.simple.label);
    params.loss_sum += ec.loss;
    if (params.all->training && ec.example_counter % 100 == 0) { trim_cache(params); }
    if (params.all->training && ec.example_counter % 1000 == 0 && ec.example_counter >= 2)
    {
      *params.all->trace_message << "Number of support vectors = " << params.model->num_support << endl;
      *params.all->trace_message << "Number of kernel evaluations = " << num_kernel_evals << " "
                                 << "Number of cache queries = " << num_cache_evals << " loss sum = " << params.loss_sum
                                 << " " << params.model->alpha[params.model->num_support - 1] << " "
                                 << params.model->alpha[params.model->num_support - 2] << endl;
    }
    params.pool[params.pool_pos] = sec;
    params.pool_pos++;

    if (params.pool_pos == params.pool_size)
    {
      train(params);
      params.pool_pos = 0;
    }
  }
}

void finish_kernel_svm(svm_params& params)
{
  if (params.all != nullptr)
  {
    *(params.all->trace_message) << "Num support = " << params.model->num_support << endl;
    *(params.all->trace_message) << "Number of kernel evaluations = " << num_kernel_evals << " "
                                 << "Number of cache queries = " << num_cache_evals << endl;
    *(params.all->trace_message) << "Total loss = " << params.loss_sum << endl;
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::kernel_svm_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto params = VW::make_unique<svm_params>();
  std::string kernel_type;
  float bandwidth = 1.f;
  int degree = 2;
  uint64_t pool_size;
  uint64_t reprocess;
  uint64_t subsample;

  bool ksvm = false;

  option_group_definition new_options("[Reduction] Kernel SVM");
  new_options.add(make_option("ksvm", ksvm).keep().necessary().help("Kernel svm"))
      .add(make_option("reprocess", reprocess).default_value(1).help("Number of reprocess steps for LASVM"))
      .add(make_option("pool_greedy", params->active_pool_greedy).help("Use greedy selection on mini pools"))
      .add(make_option("para_active", params->para_active).help("Do parallel active learning"))
      .add(make_option("pool_size", pool_size).default_value(1).help("Size of pools for active learning"))
      .add(make_option("subsample", subsample).default_value(1).help("Number of items to subsample from the pool"))
      .add(make_option("kernel", kernel_type)
               .keep()
               .default_value("linear")
               .one_of({"linear", "rbf", "poly"})
               .help("Type of kernel"))
      .add(make_option("bandwidth", bandwidth).keep().default_value(1.f).help("Bandwidth of rbf kernel"))
      .add(make_option("degree", degree).keep().default_value(2).help("Degree of poly kernel"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  params->pool_size = VW::cast_to_smaller_type<size_t>(pool_size);
  params->reprocess = VW::cast_to_smaller_type<size_t>(reprocess);
  params->subsample = VW::cast_to_smaller_type<size_t>(subsample);

  std::string loss_function = "hinge";
  float loss_parameter = 0.0;
  all.loss = get_loss_function(all, loss_function, loss_parameter);

  params->model = &VW::details::calloc_or_throw<svm_model>();
  new (params->model) svm_model();
  params->model->num_support = 0;
  params->maxcache = 1024 * 1024 * 1024;
  params->loss_sum = 0.;
  params->all = &all;
  params->random_state = all.get_random_state();

  // This param comes from the active reduction.
  // During options refactor: this changes the semantics a bit - now this will only be true if --active was supplied and
  // NOT --simulation
  if (all.active) { params->active = true; }
  if (params->active) { params->active_c = 1.; }

  params->pool = VW::details::calloc_or_throw<svm_example*>(params->pool_size);
  params->pool_pos = 0;

  if (!options.was_supplied("subsample") && params->para_active)
  {
    params->subsample = static_cast<size_t>(ceil(params->pool_size / all.all_reduce->total));
  }

  params->lambda = all.l2_lambda;
  if (params->lambda == 0.) { params->lambda = 1.; }
  *params->all->trace_message << "Lambda = " << params->lambda << endl;
  *params->all->trace_message << "Kernel = " << kernel_type << endl;

  if (kernel_type.compare("rbf") == 0)
  {
    params->kernel_type = SVM_KER_RBF;
    *params->all->trace_message << "bandwidth = " << bandwidth << endl;
    params->kernel_params = &VW::details::calloc_or_throw<double>();
    *(static_cast<float*>(params->kernel_params)) = bandwidth;
  }
  else if (kernel_type.compare("poly") == 0)
  {
    params->kernel_type = SVM_KER_POLY;
    *params->all->trace_message << "degree = " << degree << endl;
    params->kernel_params = &VW::details::calloc_or_throw<int>();
    *(static_cast<int*>(params->kernel_params)) = degree;
  }
  else { params->kernel_type = SVM_KER_LIN; }

  params->all->weights.stride_shift(0);

  auto l = make_bottom_learner(std::move(params), learn, predict, stack_builder.get_setupfn_name(kernel_svm_setup),
      VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
               .set_save_load(save_load)
               .set_finish(finish_kernel_svm)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<svm_params>)
               .set_update_stats(VW::details::update_stats_simple_label<svm_params>)
               .set_print_update(VW::details::print_update_simple_label<svm_params>)
               .build();

  return l;
}
