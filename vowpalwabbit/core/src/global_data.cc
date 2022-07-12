// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/global_data.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/common/vw_exception.h"
#include "vw/core/array_parameters.h"
#include "vw/core/kskip_ngram_transformer.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/named_labels.h"
#include "vw/core/parser.h"
#include "vw/core/rand_state.h"
#include "vw/core/reduction_stack.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw_allreduce.h"
#include "vw/io/logger.h"

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cassert>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef BUILD_FLATBUFFERS
#  include "vw/fb_parser/parse_example_flatbuffer.h"
#endif

struct global_prediction
{
  float p;
  float weight;
};

size_t really_read(VW::io::reader* sock, void* in, size_t count)
{
  char* buf = static_cast<char*>(in);
  size_t done = 0;
  ssize_t r = 0;
  while (done < count)
  {
    if ((r = sock->read(buf, static_cast<unsigned int>(count - done))) == 0) { return 0; }
    else if (r < 0)
    {
      THROWERRNO("read(" << sock << "," << count << "-" << done << ")");
    }
    else
    {
      done += r;
      buf += r;
    }
  }
  return done;
}

void get_prediction(VW::io::reader* f, float& res, float& weight)
{
  global_prediction p;
  really_read(f, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
}

void send_prediction(VW::io::writer* f, global_prediction p)
{
  if (f->write(reinterpret_cast<const char*>(&p), sizeof(p)) < static_cast<int>(sizeof(p)))
    THROWERRNO("send_prediction write(unknown socket fd)");
}

void binary_print_result_by_ref(VW::io::writer* f, float res, float weight, const VW::v_array<char>&, VW::io::logger&)
{
  if (f != nullptr)
  {
    global_prediction ps = {res, weight};
    send_prediction(f, ps);
  }
}
namespace VW
{
std::string workspace::get_setupfn_name(reduction_setup_fn setup_fn)
{
  const auto loc = _setup_name_map.find(setup_fn);
  if (loc != _setup_name_map.end()) { return loc->second; }
  return "NA";
}

void workspace::build_setupfn_name_dict(std::vector<std::tuple<std::string, reduction_setup_fn>>& reduction_stack)
{
  for (auto&& setup_tuple : reduction_stack) { _setup_name_map[std::get<1>(setup_tuple)] = std::get<0>(setup_tuple); }
}
}  // namespace VW

void print_result_by_ref(VW::io::writer* f, float res, float, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f != nullptr)
  {
    std::stringstream ss;
    auto saved_precision = ss.precision();
    if (floorf(res) == res) { ss << std::setprecision(0); }
    ss << std::fixed << res << std::setprecision(saved_precision);
    if (!tag.empty()) { ss << " " << VW::string_view{tag.begin(), tag.size()}; }
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = f->write(ss.str().c_str(), static_cast<unsigned int>(len));
    if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
  }
}

void print_raw_text_by_ref(
    VW::io::writer* f, const std::string& s, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f == nullptr) { return; }

  std::stringstream ss;
  ss << s;
  if (!tag.empty()) { ss << " " << VW::string_view{tag.begin(), tag.size()}; }
  ss << '\n';
  ssize_t len = ss.str().size();
  ssize_t t = f->write(ss.str().c_str(), static_cast<unsigned int>(len));
  if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
}

void set_mm(shared_data* sd, float label)
{
  sd->min_label = std::min(sd->min_label, label);
  if (label != FLT_MAX) { sd->max_label = std::max(sd->max_label, label); }
}

void noop_mm(shared_data*, float) {}

namespace VW
{
void workspace::learn(example& ec)
{
  if (l->is_multiline()) THROW("This reduction does not support single-line examples.");

  if (ec.test_only || !training) { VW::LEARNER::as_singleline(l)->predict(ec); }
  else
  {
    if (l->learn_returns_prediction) { VW::LEARNER::as_singleline(l)->learn(ec); }
    else
    {
      VW::LEARNER::as_singleline(l)->predict(ec);
      VW::LEARNER::as_singleline(l)->learn(ec);
    }
  }
}

void workspace::learn(multi_ex& ec)
{
  if (!l->is_multiline()) THROW("This reduction does not support multi-line example.");

  if (!training) { VW::LEARNER::as_multiline(l)->predict(ec); }
  else
  {
    if (l->learn_returns_prediction) { VW::LEARNER::as_multiline(l)->learn(ec); }
    else
    {
      VW::LEARNER::as_multiline(l)->predict(ec);
      VW::LEARNER::as_multiline(l)->learn(ec);
    }
  }
}

void workspace::predict(example& ec)
{
  if (l->is_multiline()) THROW("This reduction does not support single-line examples.");

  // be called directly in library mode, test_only must be explicitly set here. If the example has a label but is passed
  // to predict it would otherwise be incorrectly labelled as test_only = false.
  ec.test_only = true;
  VW::LEARNER::as_singleline(l)->predict(ec);
}

void workspace::predict(multi_ex& ec)
{
  if (!l->is_multiline()) THROW("This reduction does not support multi-line example.");

  // be called directly in library mode, test_only must be explicitly set here. If the example has a label but is passed
  // to predict it would otherwise be incorrectly labelled as test_only = false.
  for (auto& ex : ec) { ex->test_only = true; }

  VW::LEARNER::as_multiline(l)->predict(ec);
}

void workspace::finish_example(example& ec)
{
  if (l->is_multiline()) THROW("This reduction does not support single-line examples.");

  VW::LEARNER::as_singleline(l)->finish_example(*this, ec);
}

void workspace::finish_example(multi_ex& ec)
{
  if (!l->is_multiline()) THROW("This reduction does not support multi-line example.");

  VW::LEARNER::as_multiline(l)->finish_example(*this, ec);
}

template <typename WeightsT>
std::string dump_weights_to_json_weight_typed(const WeightsT& weights,
    const std::map<uint64_t, VW::details::invert_hash_info>& index_name_map, const parameters& parameter_holder,
    bool include_feature_names, bool include_online_state)
{
  rapidjson::Document doc;
  auto& allocator = doc.GetAllocator();
  // define the _document as an object rather than an array
  doc.SetObject();

  rapidjson::Value array(rapidjson::kArrayType);
  doc.AddMember("weights", array, allocator);

  for (auto v = weights.cbegin(); v != weights.cend(); ++v)
  {
    const auto idx = v.index() >> weights.stride_shift();
    if (*v != 0.f)
    {
      rapidjson::Value parameter_object(rapidjson::kObjectType);
      const auto map_it = index_name_map.find(idx);
      if (include_feature_names && map_it != index_name_map.end())
      {
        const VW::details::invert_hash_info& info = map_it->second;
        rapidjson::Value terms_array(rapidjson::kArrayType);
        for (const auto& component : info.weight_components)
        {
          rapidjson::Value component_object(rapidjson::kObjectType);
          rapidjson::Value name_value;
          name_value.SetString(component.name, allocator);
          component_object.AddMember("name", name_value, allocator);

          rapidjson::Value namespace_value;
          namespace_value.SetString(component.ns, allocator);
          component_object.AddMember("namespace", namespace_value, allocator);

          if (!component.str_value.empty())
          {
            rapidjson::Value string_value_value;
            string_value_value.SetString(component.str_value, allocator);
            component_object.AddMember("string_value", string_value_value, allocator);
          }
          else
          {
            component_object.AddMember("string_value", rapidjson::Value(rapidjson::Type::kNullType), allocator);
          }
          terms_array.PushBack(component_object, allocator);
        }
        parameter_object.AddMember("terms", terms_array, allocator);
        rapidjson::Value offset_value(static_cast<uint64_t>(info.offset != 0 ? info.offset >> info.stride_shift : 0));
        parameter_object.AddMember("offset", offset_value, allocator);
      }
      else if (include_feature_names)
      {
        // There is no reverse mapping. We leave nulls in place of terms and offset.
        parameter_object.AddMember("terms", rapidjson::Value(rapidjson::Type::kNullType), allocator);
        parameter_object.AddMember("offset", rapidjson::Value(rapidjson::Type::kNullType), allocator);
      }

      // If include_feature_names is false, we don't add terms or offset

      rapidjson::Value index_value(static_cast<uint64_t>(idx));
      parameter_object.AddMember("index", index_value, allocator);
      rapidjson::Value value_value(static_cast<float>(*v));
      parameter_object.AddMember("value", value_value, allocator);

      const float* current_weight_state = &(*v);
      if (include_online_state)
      {
        rapidjson::Value extra_state_value(rapidjson::kObjectType);
        rapidjson::Value adaptive_value(rapidjson::kNumberType);
        rapidjson::Value normalized_value(rapidjson::kNumberType);

        if (parameter_holder.adaptive && !parameter_holder.normalized)
        {
          adaptive_value = current_weight_state[1];
          normalized_value = rapidjson::kNullType;
        }
        if (!parameter_holder.adaptive && parameter_holder.normalized)
        {
          adaptive_value = rapidjson::kNullType;
          normalized_value = current_weight_state[1];
        }
        if (parameter_holder.adaptive && parameter_holder.normalized)
        {
          adaptive_value = current_weight_state[1];
          normalized_value = current_weight_state[2];
        }

        extra_state_value.AddMember("adaptive", adaptive_value, allocator);
        extra_state_value.AddMember("normalized", normalized_value, allocator);
        parameter_object.AddMember("gd_extra_online_state", extra_state_value, allocator);
      }
      doc["weights"].PushBack(parameter_object, allocator);
    }
  }

  rapidjson::StringBuffer strbuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
  doc.Accept(writer);
  return strbuf.GetString();
}

std::string workspace::dump_weights_to_json_experimental()
{
  assert(l != nullptr);
  const auto* current = l;

  // This could be extended to other base learners reasonably. Since this is new and experimental though keep the scope
  // small.
  while (current->get_learn_base() != nullptr) { current = current->get_learn_base(); }
  if (current->get_name() != "gd")
  {
    THROW("dump_weights_to_json is currently only supported for GD base learners. The current base learner is "
        << current->get_name());
  }
  if (dump_json_weights_include_feature_names && !hash_inv)
  { THROW("hash_inv == true is required to dump weights to json including feature names"); }
  if (dump_json_weights_include_extra_online_state && !save_resume)
  { THROW("save_resume == true is required to dump weights to json including feature names"); }

  return weights.sparse ? dump_weights_to_json_weight_typed(weights.sparse_weights, index_name_map, weights,
                              dump_json_weights_include_feature_names, dump_json_weights_include_extra_online_state)
                        : dump_weights_to_json_weight_typed(weights.dense_weights, index_name_map, weights,
                              dump_json_weights_include_feature_names, dump_json_weights_include_extra_online_state);
}
}  // namespace VW

void compile_limits(
    std::vector<std::string> limits, std::array<uint32_t, NUM_NAMESPACES>& dest, bool /*quiet*/, VW::io::logger& logger)
{
  for (size_t i = 0; i < limits.size(); i++)
  {
    std::string limit = limits[i];
    if (isdigit(limit[0]))
    {
      int n = atoi(limit.c_str());
      logger.err_warn("limiting to {} features for each namespace.", n);
      for (size_t j = 0; j < 256; j++) { dest[j] = n; }
    }
    else if (limit.size() == 1)
    {
      logger.out_error("The namespace index must be specified before the n");
    }
    else
    {
      int n = atoi(limit.c_str() + 1);
      dest[static_cast<uint32_t>(limit[0])] = n;
      logger.err_warn("limiting to {0} for namespaces {1}", n, limit[0]);
    }
  }
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE

namespace VW
{
workspace::workspace(VW::io::logger logger) : options(nullptr, nullptr), logger(std::move(logger))
{
  _random_state_sp = std::make_shared<VW::rand_state>();
  sd = new shared_data();
  // Default is stderr.
  trace_message = VW::make_unique<std::ostream>(std::cout.rdbuf());

  l = nullptr;
  scorer = nullptr;
  cost_sensitive = nullptr;
  loss = nullptr;
  example_parser = nullptr;

  reg_mode = 0;
  current_pass = 0;

  bfgs = false;
  no_bias = false;
  active = false;
  num_bits = 18;
  default_bits = true;
  daemon = false;
  num_children = 10;
  save_resume = true;
  preserve_performance_counters = false;

  random_positive_weights = false;

  weights.sparse = false;

  set_minmax = set_mm;

  power_t = 0.5f;
  eta = 0.5f;  // default learning rate for normalized adaptive updates, this is switched to 10 by default for the other
               // updates (see parse_args.cc)
  numpasses = 1;

  print_by_ref = print_result_by_ref;
  print_text_by_ref = print_raw_text_by_ref;
  lda = 0;
  random_seed = 0;
  random_weights = false;
  normal_weights = false;
  tnormal_weights = false;
  per_feature_regularizer_input = "";
  per_feature_regularizer_output = "";
  per_feature_regularizer_text = "";

  stdout_adapter = VW::io::open_stdout();

  searchstr = nullptr;

  nonormalize = false;
  l1_lambda = 0.0;
  l2_lambda = 0.0;

  eta_decay_rate = 1.0;
  initial_weight = 0.0;
  initial_constant = 0.0;

  all_reduce = nullptr;

  for (size_t i = 0; i < NUM_NAMESPACES; i++)
  {
    limit[i] = INT_MAX;
    affix_features[i] = 0;
    spelling_features[i] = 0;
  }

  invariant_updates = true;
  normalized_idx = 2;

  add_constant = true;
  audit = false;
  audit_writer = VW::io::open_stdout();

  pass_length = std::numeric_limits<size_t>::max();
  passes_complete = 0;

  save_per_pass = false;

  stdin_off = false;
  do_reset_source = false;
  holdout_set_off = true;
  holdout_after = 0;
  check_holdout_every_n_passes = 1;
  early_terminate = false;

  max_examples = std::numeric_limits<size_t>::max();

  hash_inv = false;
  print_invert = false;

  // Set by the '--progress <arg>' option and affect sd->dump_interval
  progress_add = false;  // default is multiplicative progress dumps
  progress_arg = 2.0;    // next update progress dump multiplier
}
VW_WARNING_STATE_POP

workspace::~workspace()
{
  if (l != nullptr)
  {
    l->finish();
    delete l;
  }

  // TODO: migrate all finalization into parser destructor
  if (example_parser != nullptr)
  {
    free_parser(*this);
    delete example_parser;
  }

  const bool seeded = weights.seeded() > 0;
  if (!seeded) { delete sd; }

  delete all_reduce;
}

}  // namespace VW
