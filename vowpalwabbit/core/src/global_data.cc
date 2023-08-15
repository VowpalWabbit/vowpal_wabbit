// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/global_data.h"

#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/reductions/metrics.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include "vw/common/future_compat.h"
#include "vw/common/random.h"
#include "vw/common/string_view.h"
#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/array_parameters.h"
#include "vw/core/kskip_ngram_transformer.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/named_labels.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/parser.h"
#include "vw/core/reduction_stack.h"
#include "vw/core/reductions/metrics.h"
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

#ifdef VW_FEAT_FLATBUFFERS_ENABLED
#  include "vw/fb_parser/parse_example_flatbuffer.h"
#endif

void VW::details::print_result_by_ref(
    VW::io::writer* f, float res, float, const VW::v_array<char>& tag, VW::io::logger& logger)
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
    if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
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
  if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
}

namespace VW
{
void workspace::learn(example& ec)
{
  if (l->is_multiline()) THROW("This learner does not support single-line examples.");

  if (ec.test_only || !runtime_config.training) { VW::LEARNER::require_singleline(l)->predict(ec); }
  else
  {
    if (l->learn_returns_prediction) { VW::LEARNER::require_singleline(l)->learn(ec); }
    else
    {
      VW::LEARNER::require_singleline(l)->predict(ec);
      VW::polyprediction saved_prediction;
      // VW::move_pred_to(ec.pred, saved_prediction, l->get_output_prediction_type());
      // new (&ec.pred) VW::polyprediction();
      VW::LEARNER::require_singleline(l)->learn(ec);
      // VW::move_pred_to(saved_prediction, ec.pred, l->get_output_prediction_type());
    }
  }
}

void workspace::learn(multi_ex& ec)
{
  if (!l->is_multiline()) THROW("This learner does not support multi-line example.");

  if (!runtime_config.training) { VW::LEARNER::require_multiline(l)->predict(ec); }
  else
  {
    if (l->learn_returns_prediction) { VW::LEARNER::require_multiline(l)->learn(ec); }
    else
    {
      VW::LEARNER::require_multiline(l)->predict(ec);
      VW::polyprediction saved_prediction;
      VW::move_pred_to(ec[0]->pred, saved_prediction, l->get_output_prediction_type());
      new (&ec[0]->pred) VW::polyprediction();
      VW::LEARNER::require_multiline(l)->learn(ec);
      VW::move_pred_to(saved_prediction, ec[0]->pred, l->get_output_prediction_type());
    }
  }
}

void workspace::predict(example& ec)
{
  if (l->is_multiline()) THROW("This learner does not support single-line examples.");

  // be called directly in library mode, test_only must be explicitly set here. If the example has a label but is passed
  // to predict it would otherwise be incorrectly labelled as test_only = false.
  ec.test_only = true;
  VW::LEARNER::require_singleline(l)->predict(ec);
}

void workspace::predict(multi_ex& ec)
{
  if (!l->is_multiline()) THROW("This learner does not support multi-line example.");

  // be called directly in library mode, test_only must be explicitly set here. If the example has a label but is passed
  // to predict it would otherwise be incorrectly labelled as test_only = false.
  for (auto& ex : ec) { ex->test_only = true; }

  VW::LEARNER::require_multiline(l)->predict(ec);
}

void workspace::finish_example(example& ec)
{
  if (l->is_multiline()) THROW("This learner does not support single-line examples.");

  VW::LEARNER::require_singleline(l)->finish_example(*this, ec);
}

void workspace::finish_example(multi_ex& ec)
{
  if (!l->is_multiline()) THROW("This learner does not support multi-line example.");

  VW::LEARNER::require_multiline(l)->finish_example(*this, ec);
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
          else { component_object.AddMember("string_value", rapidjson::Value(rapidjson::Type::kNullType), allocator); }
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
  const auto* current = l.get();

  // This could be extended to other base learners reasonably. Since this is new and experimental though keep the scope
  // small.
  while (current->get_base_learner() != nullptr) { current = current->get_base_learner(); }
  if (current->get_name() == "ksvm")
  {
    THROW("dump_weights_to_json is currently only supported for KSVM base learner. The current base learner is "
        << current->get_name());
  }
  if (output_model_config.dump_json_weights_include_feature_names && !output_config.hash_inv)
  {
    THROW("hash_inv == true is required to dump weights to json including feature names");
  }
  if (output_model_config.dump_json_weights_include_extra_online_state && !output_model_config.save_resume)
  {
    THROW("save_resume == true is required to dump weights to json including feature names");
  }
  if (output_model_config.dump_json_weights_include_extra_online_state && current->get_name() != "gd")
  {
    THROW("including extra online state is only allowed with GD as base learner");
  }

  return weights.sparse ? dump_weights_to_json_weight_typed(weights.sparse_weights, output_runtime.index_name_map,
                              weights, output_model_config.dump_json_weights_include_feature_names,
                              output_model_config.dump_json_weights_include_extra_online_state)
                        : dump_weights_to_json_weight_typed(weights.dense_weights, output_runtime.index_name_map,
                              weights, output_model_config.dump_json_weights_include_feature_names,
                              output_model_config.dump_json_weights_include_extra_online_state);
}
}  // namespace VW

void VW::details::compile_limits(std::vector<std::string> limits, std::array<uint32_t, VW::NUM_NAMESPACES>& dest,
    bool /*quiet*/, VW::io::logger& logger)
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
    else if (limit.size() == 1) { logger.out_error("The namespace index must be specified before the n"); }
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
  sd = std::make_shared<shared_data>();
  // Default is stderr.
  output_runtime.trace_message = std::make_shared<std::ostream>(std::cout.rdbuf());

  loss_config.loss = nullptr;

  loss_config.reg_mode = 0;
  passes_config.current_pass = 0;

  reduction_state.bfgs = false;
  loss_config.no_bias = false;
  reduction_state.active = false;
  initial_weights_config.num_bits = 18;
  runtime_config.default_bits = true;
#ifdef VW_FEAT_NETWORKING_ENABLED
  runtime_config.daemon = false;
#endif
  output_model_config.save_resume = true;
  output_model_config.preserve_performance_counters = false;

  initial_weights_config.random_positive_weights = false;

  weights.sparse = false;

  // default set_minmax function
  set_minmax = [this](float label)
  {
    this->sd->min_label = std::min(this->sd->min_label, label);
    if (label != FLT_MAX) { this->sd->max_label = std::max(this->sd->max_label, label); }
  };

  update_rule_config.power_t = 0.5f;
  update_rule_config.eta = 0.5f;  // default learning rate for normalized adaptive updates, this is switched to 10 by
                                  // default for the other updates (see parse_args.cc)
  runtime_config.numpasses = 1;

  print_by_ref = VW::details::print_result_by_ref;
  print_text_by_ref = print_raw_text_by_ref;
  reduction_state.lda = 0;
  initial_weights_config.random_weights = false;
  initial_weights_config.normal_weights = false;
  initial_weights_config.tnormal_weights = false;
  initial_weights_config.per_feature_regularizer_input = "";
  output_model_config.per_feature_regularizer_output = "";
  output_model_config.per_feature_regularizer_text = "";

  output_runtime.stdout_adapter = VW::io::open_stdout();

  reduction_state.searchstr = nullptr;

  // nonormalize = false;
  loss_config.l1_lambda = 0.0;
  loss_config.l2_lambda = 0.0;

  update_rule_config.eta_decay_rate = 1.0;
  initial_weights_config.initial_weight = 0.0;
  feature_tweaks_config.initial_constant = 0.0;

  for (size_t i = 0; i < NUM_NAMESPACES; i++)
  {
    feature_tweaks_config.limit[i] = INT_MAX;
    feature_tweaks_config.affix_features[i] = 0;
    feature_tweaks_config.spelling_features[i] = 0;
  }

  feature_tweaks_config.add_constant = true;

  reduction_state.invariant_updates = true;
  initial_weights_config.normalized_idx = 2;

  output_config.audit = false;
  output_runtime.audit_writer = VW::io::open_stdout();

  runtime_config.pass_length = std::numeric_limits<size_t>::max();
  runtime_state.passes_complete = 0;

  output_model_config.save_per_pass = false;

  runtime_state.do_reset_source = false;
  passes_config.holdout_set_off = true;
  passes_config.holdout_after = 0;
  passes_config.check_holdout_every_n_passes = 1;
  passes_config.early_terminate = false;

  parser_runtime.max_examples = std::numeric_limits<size_t>::max();

  output_config.hash_inv = false;
  output_config.print_invert = false;
  output_config.hexfloat_weights = false;
}
VW_WARNING_STATE_POP

void workspace::finish()
{
  // also update VowpalWabbit::PerformanceStatistics::get() (vowpalwabbit.cpp)
  if (!output_config.quiet && !options->was_supplied("audit_regressor"))
  {
    sd->print_summary(*output_runtime.trace_message, *sd, *loss_config.loss, passes_config.current_pass,
        passes_config.holdout_set_off);
  }

  details::finalize_regressor(*this, output_model_config.final_regressor_name);
  if (options->was_supplied("dump_json_weights_experimental"))
  {
    auto content = dump_weights_to_json_experimental();
    auto writer = VW::io::open_file_writer(output_model_config.json_weights_file_name);
    writer->write(content.c_str(), content.length());
  }
  VW::reductions::output_metrics(*this);
  logger.log_summary();

  if (l != nullptr) { l->finish(); }
}

workspace::~workspace()
{
  // TODO: migrate all finalization into parser destructor
  if (parser_runtime.example_parser != nullptr) { VW::details::free_parser(*this); }
}

}  // namespace VW
