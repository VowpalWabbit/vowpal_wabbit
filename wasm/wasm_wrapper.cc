#include "vw/common/text_utils.h"
#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/parse_example.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/prediction_type.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <array>
#include <iostream>
#include <memory>
#include <string>

std::array<std::string, 51> illegal_options = {"feature_mask", "initial_regressor", "input_feature_regularizer",
    "span_server", "unique_id", "total", "node", "span_server_port", "version", "audit", "progress", "limit_output",
    "dry_run", "help", "dictionary", "dictionary_path", "holdout_off", "holdout_period", "holdout_after",
    "early_terminate", "passes", "initial_pass_length", "named_labels", "final_regressor", "readable_model",
    "invert_hash", "save_resume", "output_feature_regularizer_binary", "output_feature_regularizer_text",
    "save_per_pass", "predictions", "raw_predictions", "extra_metrics", "audit_regressor", "sendto", "data", "daemon",
    "foreground", "port", "num_children", "pid_file", "port_file", "cache", "cache_file", "json", "dsjson",
    "kill_cache", "compressed", "no_daemon", "chain_hash", "flatbuffer"};

std::string get_exception_message(int exceptionPtr)
{
  return std::string(reinterpret_cast<std::exception*>(exceptionPtr)->what());
}

void validate_options(const VW::config::options_i& options)
{
  for (const auto& illegal_option : illegal_options)
  {
    if (options.was_supplied(illegal_option)) { THROW("Illegal option passed: " << illegal_option); }
  }

  if (!options.get_positional_tokens().empty()) { THROW("Positional options are not allowed") }
}

struct vw_model;

struct example_ptr
{
  static std::shared_ptr<example_ptr> wrap_pooled_example(example* ex, const std::shared_ptr<vw>& vw_ptr)
  {
    assert(VW::is_ring_example(*vw_ptr, ex));
    return std::make_shared<example_ptr>(ex, vw_ptr);
  }
  std::shared_ptr<example_ptr> clone(const vw_model& vw_ptr) const;

  std::string to_string() const { return "Example"; }

  example* inner_ptr() { return _example; }
  const example* inner_ptr() const { return _example; }

  example* release()
  {
    _released = true;
    return _example;
  }

  example_ptr(example* ex, const std::shared_ptr<vw>& vw_ptr) : _example(ex), weak_vw_ptr(vw_ptr), _released(false) {}
  ~example_ptr()
  {
    if (!_released)
    {
      if (auto strong_vw_ptr = weak_vw_ptr.lock()) { VW::finish_example(*strong_vw_ptr, *_example); }
    }
  }

  example_ptr(const example_ptr&) = delete;
  example_ptr& operator=(const example_ptr&) = delete;
  example_ptr(example_ptr&& other) noexcept = delete;
  example_ptr& operator=(example_ptr&& other) noexcept = delete;

private:
  example* _example;
  std::weak_ptr<vw> weak_vw_ptr;
  bool _released;
};

emscripten::val to_js_type(const v_array<float>& float_array)
{
  return emscripten::val::array(float_array.begin(), float_array.end());
}

emscripten::val to_js_type(const v_array<uint32_t>& array)
{
  return emscripten::val::array(array.begin(), array.end());
}

emscripten::val to_js_type(const v_array<ACTION_SCORE::action_score>& action_scores_array)
{
  return emscripten::val::array(action_scores_array.begin(), action_scores_array.end());
}
emscripten::val to_js_type(const VW::decision_scores_t& decision_scores_array)
{
  auto array = emscripten::val::array();
  for (const auto& action_score : decision_scores_array) { array.call<void>("push", to_js_type(action_score)); }
  return array;
}

emscripten::val prediction_to_val(const polyprediction& pred, prediction_type_t pred_type)
{
  switch (pred_type)
  {
    case VW::prediction_type_t::SCALAR:
      return emscripten::val(pred.scalar);

    case VW::prediction_type_t::SCALARS:
      return to_js_type(pred.scalars);

    case VW::prediction_type_t::ACTION_SCORES:
      return to_js_type(pred.a_s);

    case VW::prediction_type_t::PDF:
      THROW("pdf prediction type is not implemented.");

    case VW::prediction_type_t::ACTION_PROBS:
      return to_js_type(pred.a_s);

    case VW::prediction_type_t::MULTICLASS:
      return emscripten::val(pred.multiclass);

    case VW::prediction_type_t::MULTILABELS:
      return to_js_type(pred.multilabels.label_v);

    case VW::prediction_type_t::PROB:
      return emscripten::val(pred.scalar);

    case VW::prediction_type_t::MULTICLASS_PROBS:
      return to_js_type(pred.scalars);

    case VW::prediction_type_t::DECISION_PROBS:
      return to_js_type(pred.decision_scores);

    case VW::prediction_type_t::ACTION_PDF_VALUE:
      THROW("action_pdf_value prediction type is not implemented.");

    case VW::prediction_type_t::ACTIVE_MULTICLASS:
      THROW("active_multiclass prediction type is not implemented.");

    case VW::prediction_type_t::NOPRED:
      THROW("active_multiclass prediction type is not implemented.");
  }
}

struct vw_model
{
  vw_model(const std::string& args)
  {
    std::string all_args = "--quiet --no_stdin " + args;
    vw_ptr.reset(VW::initialize(all_args));
    validate_options(*vw_ptr->options);
  }

  vw_model(const std::string& args, size_t _bytes, int size)
  {
    std::string all_args = "--quiet --no_stdin " + args;
    char* bytes = (char*)_bytes;
    io_buf io;
    io.add_file(VW::io::create_buffer_view(bytes, size));
    vw_ptr.reset(VW::initialize(all_args, &io));
    validate_options(*vw_ptr->options);
  }

  std::vector<std::shared_ptr<example_ptr>> parse(const std::string& ex_str)
  {
    std::vector<std::shared_ptr<example_ptr>> example_collection;
    VW::string_view trimmed_ex_str = VW::trim_whitespace(VW::string_view(ex_str));
    std::vector<example*> examples;

    std::vector<VW::string_view> lines;
    // expensive but safe find()
    VW::tokenize_expensive('\n', trimmed_ex_str, lines);
    for (size_t i = 0; i < lines.size(); i++)
    {
      // Check if a new empty example needs to be added.
      if (examples.size() < i + 1) { examples.push_back(&VW::get_unused_example(vw_ptr.get())); }
      VW::parsers::text::read_line(*vw_ptr.get(), examples[i], lines[i]);
    }

    example_collection.reserve(example_collection.size() + examples.size());
    for (auto* ex : examples)
    {
      VW::setup_example(*vw_ptr, ex);
      example_collection.push_back(example_ptr::wrap_pooled_example(ex, vw_ptr));
    }

    return example_collection;
  }

  emscripten::val predict(std::vector<std::shared_ptr<example_ptr>>& example_list)
  {
    assert(!example_list.empty());
    if (example_list.size() == 1)
    {
      auto* ex = example_list[0]->inner_ptr();
      auto prev_test_only_value = ex->test_only;
      vw_ptr->predict(*ex);
      ex->test_only = prev_test_only_value;
      return prediction_to_val(ex->pred, vw_ptr->l->get_output_prediction_type());
    }
    else
    {
      std::vector<example*> raw_examples;
      std::vector<bool> prev_test_only_value;
      raw_examples.reserve(example_list.size());
      for (auto& ex : example_list)
      {
        raw_examples.push_back(ex->inner_ptr());
        prev_test_only_value.push_back(ex->inner_ptr()->test_only);
      }
      vw_ptr->predict(raw_examples);
      for (int i = 0; i < example_list.size(); i++) { raw_examples[i]->test_only = prev_test_only_value[i]; }
      return prediction_to_val(raw_examples[0]->pred, vw_ptr->l->get_output_prediction_type());
    }
  }

  void learn(std::vector<std::shared_ptr<example_ptr>>& example_list)
  {
    assert(!example_list.empty());
    if (example_list.size() == 1)
    {
      auto* ex = example_list[0]->inner_ptr();
      vw_ptr->learn(*ex);
    }
    else
    {
      std::vector<example*> raw_examples;
      raw_examples.reserve(example_list.size());
      for (auto& ex : example_list) { raw_examples.push_back(ex->inner_ptr()); }
      vw_ptr->learn(raw_examples);
    }
  }

  void finish_example(std::vector<std::shared_ptr<example_ptr>>& example_list)
  {
    assert(!example_list.empty());
    if (example_list.size() == 1)
    {
      auto* ex = example_list[0]->inner_ptr();
      vw_ptr->finish_example(*ex);
    }
    else
    {
      std::vector<example*> raw_examples;
      raw_examples.reserve(example_list.size());
      for (auto& ex : example_list) { raw_examples.push_back(ex->inner_ptr()); }
      vw_ptr->finish_example(raw_examples);
    }

    for (auto& ex_ptr : example_list) { ex_ptr->release(); }
  }

  float sum_loss() const { return vw_ptr->sd->sum_loss; }
  float weighted_labeled_examples() const { return vw_ptr->sd->weighted_labeled_examples; }

  prediction_type_t get_prediction_type() const { return vw_ptr->l->get_output_prediction_type(); }

  std::shared_ptr<vw> vw_ptr;
};

std::shared_ptr<example_ptr> example_ptr::clone(const vw_model& vw_ptr) const
{
  auto* new_ex = &VW::get_unused_example(vw_ptr.vw_ptr.get());
  VW::copy_example_data_with_label(new_ex, _example);
  return wrap_pooled_example(new_ex, vw_ptr.vw_ptr);
}

EMSCRIPTEN_BINDINGS(vwwasm)
{
  emscripten::function("getExceptionMessage", &get_exception_message);

  emscripten::value_object<ACTION_SCORE::action_score>("ActionScore")
      .field("action", &ACTION_SCORE::action_score::action)
      .field("score", &ACTION_SCORE::action_score::score);

  emscripten::class_<example_ptr>("Example")
      .function("clone", &example_ptr::clone)
      .function("toString", &example_ptr::to_string)
      .smart_ptr<std::shared_ptr<example_ptr>>("example_ptr");

  emscripten::enum_<prediction_type_t>("PredictionType")
      .value("scalar", prediction_type_t::scalar)
      .value("scalars", prediction_type_t::scalars)
      .value("action_scores", prediction_type_t::action_scores)
      .value("pdf", prediction_type_t::pdf)
      .value("action_probs", prediction_type_t::action_probs)
      .value("multiclass", prediction_type_t::multiclass)
      .value("multilabels", prediction_type_t::multilabels)
      .value("prob", prediction_type_t::prob)
      .value("multiclassprobs", prediction_type_t::multiclassprobs)
      .value("decision_probs", prediction_type_t::decision_probs)
      .value("actionPDFValue", prediction_type_t::action_pdf_value)
      .value("activeMulticlass", prediction_type_t::active_multiclass);

  // Currently this is structured such that parse returns a vector of example but to JS that is opaque.
  // All the caller can do is pass this opaque object to the other functions. Is it possible to convert this to a JS
  // array but it involves copying the contents of the array whenever going to/from js/c++ For now it is opaque as the
  // protoyype doesnt support operations on the example itself.
  emscripten::class_<vw_model>("VWModel")
      .constructor<std::string>()
      .constructor<std::string, size_t, int>()
      .function("parse", &vw_model::parse)
      .function("predict", &vw_model::predict)
      .function("learn", &vw_model::learn)
      .function("finishExample", &vw_model::finish_example)
      .property("sumLoss", &vw_model::sum_loss)
      .property("weightedLabeledExamples", &vw_model::weighted_labeled_examples)
      .property("predictionType", &vw_model::get_prediction_type);

  emscripten::register_vector<std::shared_ptr<example_ptr>>("ExamplePtrVector");
};
