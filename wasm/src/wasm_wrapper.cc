#include "vw/common/text_utils.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/parse_example.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/parser.h"
#include "vw/core/prediction_type.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/explore/explore.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

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

struct vw_model_basic;

template <typename MixIn = vw_model_basic>
struct vw_model;

struct example_ptr
{
  static std::shared_ptr<example_ptr> wrap_pooled_example(VW::example* ex, const std::shared_ptr<VW::workspace>& vw_ptr)
  {
    assert(VW::is_ring_example(*vw_ptr, ex));
    return std::make_shared<example_ptr>(ex, vw_ptr);
  }
  std::shared_ptr<example_ptr> clone(const vw_model<>& vw_ptr) const;

  std::string to_string() const { return "Example"; }

  VW::example* inner_ptr() { return _example; }
  const VW::example* inner_ptr() const { return _example; }

  VW::example* release()
  {
    _released = true;
    return _example;
  }

  example_ptr(VW::example* ex, const std::shared_ptr<VW::workspace>& vw_ptr)
      : _example(ex), weak_vw_ptr(vw_ptr), _released(false)
  {
  }
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
  VW::example* _example;
  std::weak_ptr<VW::workspace> weak_vw_ptr;
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

emscripten::val to_js_type(const v_array<VW::action_score>& action_scores_array)
{
  return emscripten::val::array(action_scores_array.begin(), action_scores_array.end());
}
emscripten::val to_js_type(const VW::decision_scores_t& decision_scores_array)
{
  auto array = emscripten::val::array();
  for (const auto& action_score : decision_scores_array) { array.call<void>("push", to_js_type(action_score)); }
  return array;
}

emscripten::val prediction_to_val(const VW::polyprediction& pred, VW::prediction_type_t pred_type)
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

struct vw_model_basic
{
  vw_model_basic(const std::string& args_)
  {
    args = "--quiet --no_stdin " + args_;
    vw_ptr = VW::initialize(VW::make_unique<VW::config::options_cli>(VW::split_command_line(args)));
    validate_options(*vw_ptr->options);
  }

  vw_model_basic(const std::string& args_, size_t bytes_, int size)
  {
    args = "--quiet --no_stdin " + args_;
    char* bytes = (char*)bytes_;
    VW::io_buf io;
    io.add_file(VW::io::create_buffer_view(bytes, size));
    vw_ptr = VW::initialize(VW::make_unique<VW::config::options_cli>(VW::split_command_line(args)), &io);
    validate_options(*vw_ptr->options);
  }

  virtual ~vw_model_basic() = default;

  void load_model_from_buffer(size_t _bytes, int size)
  {
    char* bytes = (char*)_bytes;
    VW::io_buf io;
    io.add_file(VW::io::create_buffer_view(bytes, size));
    vw_ptr = VW::initialize(VW::make_unique<VW::config::options_cli>(VW::split_command_line(args)), &io);
    validate_options(*vw_ptr->options);
  }

  std::vector<char> get_model()
  {
    VW::io_buf io_temp;
    auto buffer = std::make_shared<std::vector<char>>();
    io_temp.add_file(VW::io::create_vector_writer(buffer));
    VW::details::dump_regressor(*vw_ptr, io_temp, false);
    auto vec = *buffer.get();
    buffer.reset();
    return vec;
  }

  float sum_loss() const { return vw_ptr->sd->sum_loss; }
  float weighted_labeled_examples() const { return vw_ptr->sd->weighted_labeled_examples; }

  VW::prediction_type_t get_prediction_type() const { return vw_ptr->l->get_output_prediction_type(); }

  std::vector<std::shared_ptr<example_ptr>> create_example_from_dense_features(
      const emscripten::val& features, const std::string& label)
  {
    std::vector<std::shared_ptr<example_ptr>> example_collection;
    auto* ex = &VW::get_unused_example(this->vw_ptr.get());

    emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", features);
    int length = keys["length"].as<int>();

    for (int i = 0; i < length; ++i)
    {
      auto key = keys[i].as<std::string>();
      if (features.hasOwnProperty(key.c_str()))
      {
        auto values = emscripten::convertJSArrayToNumberVector<float>(features[key]);
        auto namespace_hash = VW::hash_space(*this->vw_ptr, key);
        auto namespace_slot = key.length() > 0 ? key[0] : ' ';
        auto anon_index = 0;
        auto& feature_group = ex->feature_space[namespace_slot];
        auto it = std::find(ex->indices.begin(), ex->indices.end(), namespace_slot);
        if (it == ex->indices.end()) { ex->indices.push_back(namespace_slot); }

        feature_group.indices.reserve(feature_group.indices.size() + values.size());
        feature_group.values.reserve(feature_group.values.size() + values.size());
        for (auto v : values)
        {
          feature_group.indices.push_back(anon_index++);
          feature_group.values.push_back(v);
        }
      }
    }

    this->vw_ptr->parser_runtime.example_parser->lbl_parser.default_label(ex->l);
    this->vw_ptr->parser_runtime.example_parser->words.clear();
    VW::tokenize(' ', label, this->vw_ptr->parser_runtime.example_parser->words);
    this->vw_ptr->parser_runtime.example_parser->lbl_parser.parse_label(ex->l, ex->ex_reduction_features,
        this->vw_ptr->parser_runtime.example_parser->parser_memory_to_reuse, this->vw_ptr->sd->ldict.get(),
        this->vw_ptr->parser_runtime.example_parser->words, this->vw_ptr->logger);
    VW::setup_example(*this->vw_ptr, ex);
    return {example_ptr::wrap_pooled_example(ex, this->vw_ptr)};
  }

  std::shared_ptr<VW::workspace> vw_ptr;
  std::string args;
};

template <typename MixIn>
struct vw_model : MixIn
{
  vw_model(const std::string& args) : MixIn(args) {}

  vw_model(const std::string& args, size_t _bytes, int size) : MixIn(args, _bytes, size) {}

  std::vector<std::shared_ptr<example_ptr>> parse(const std::string& ex_str)
  {
    std::vector<std::shared_ptr<example_ptr>> example_collection;
    VW::string_view trimmed_ex_str = VW::trim_whitespace(VW::string_view(ex_str));
    std::vector<VW::example*> examples;

    this->vw_ptr->parser_runtime.example_parser->text_reader(this->vw_ptr.get(), trimmed_ex_str, examples);

    example_collection.reserve(example_collection.size() + examples.size());
    for (auto* ex : examples)
    {
      VW::setup_example(*this->vw_ptr, ex);
      example_collection.push_back(example_ptr::wrap_pooled_example(ex, this->vw_ptr));
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
      this->vw_ptr->predict(*ex);
      ex->test_only = prev_test_only_value;
      return prediction_to_val(ex->pred, this->vw_ptr->l->get_output_prediction_type());
    }
    else
    {
      std::vector<VW::example*> raw_examples;
      std::vector<bool> prev_test_only_value;
      raw_examples.reserve(example_list.size());
      for (auto& ex : example_list)
      {
        raw_examples.push_back(ex->inner_ptr());
        prev_test_only_value.push_back(ex->inner_ptr()->test_only);
      }
      this->vw_ptr->predict(raw_examples);
      for (int i = 0; i < example_list.size(); i++) { raw_examples[i]->test_only = prev_test_only_value[i]; }
      return prediction_to_val(raw_examples[0]->pred, this->vw_ptr->l->get_output_prediction_type());
    }
  }

  void learn(std::vector<std::shared_ptr<example_ptr>>& example_list)
  {
    assert(!example_list.empty());
    if (example_list.size() == 1)
    {
      auto* ex = example_list[0]->inner_ptr();
      this->vw_ptr->learn(*ex);
    }
    else
    {
      std::vector<VW::example*> raw_examples;
      raw_examples.reserve(example_list.size());
      for (auto& ex : example_list) { raw_examples.push_back(ex->inner_ptr()); }
      this->vw_ptr->learn(raw_examples);
    }
  }

  void finish_example(std::vector<std::shared_ptr<example_ptr>>& example_list)
  {
    assert(!example_list.empty());
    if (example_list.size() == 1)
    {
      auto* ex = example_list[0]->inner_ptr();
      this->vw_ptr->finish_example(*ex);
    }
    else
    {
      std::vector<VW::example*> raw_examples;
      raw_examples.reserve(example_list.size());
      for (auto& ex : example_list) { raw_examples.push_back(ex->inner_ptr()); }
      this->vw_ptr->finish_example(raw_examples);
    }

    for (auto& ex_ptr : example_list) { ex_ptr->release(); }
  }
};

template <typename MixIn = vw_model_basic>
struct cb_vw_model : MixIn
{
  cb_vw_model(const std::string& args) : MixIn(args)
  {
    _pmf_sampling_seed = this->vw_ptr->get_random_state()->get_current_state();
  }

  cb_vw_model(const std::string& args, size_t _bytes, int size) : MixIn(args, _bytes, size)
  {
    _pmf_sampling_seed = this->vw_ptr->get_random_state()->get_current_state();
  }

  void set_sampling_seed(uint32_t seed) { _pmf_sampling_seed = seed; }

  uint32_t sample_pmf_internal(std::vector<float>& pmf, uint64_t seed)
  {
    uint32_t chosen_index = 0;
    int ret_val = VW::explore::sample_after_normalizing(seed, pmf.begin(), pmf.end(), chosen_index);

    if (ret_val != 0) { THROW("sample_after_normalizing failed"); }

    return chosen_index;
  }

  emscripten::val sample_pmf(const emscripten::val& a_s, const std::string& uid)
  {
    int length = 0;
    if (!a_s.hasOwnProperty("length") || (length = a_s["length"].as<int>()) <= 0)
    {
      THROW("sample_pmf expects an array of {action, score} pairs");
    }

    std::vector<float> pmf(length, 0);

    for (int i = 0; i < length; i++)
    {
      if (!a_s[i].hasOwnProperty("action") || !a_s[i].hasOwnProperty("score"))
      {
        THROW("sample_pmf expects an array of {action, score} pairs");
      }
      pmf[a_s[i]["action"].as<uint32_t>()] = a_s[i]["score"].as<float>();
    }

    const uint64_t seed = VW::uniform_hash(uid.data(), uid.size(), 0) + _pmf_sampling_seed;
    auto chosen_index = sample_pmf_internal(pmf, seed);

    auto chosen_a_s = emscripten::val::object();
    chosen_a_s.set("action", chosen_index);
    chosen_a_s.set("score", pmf[chosen_index]);

    return chosen_a_s;
  }

  emscripten::val predict(const emscripten::val& example_input)
  {
    auto example_list = parse(example_input);

    for (auto* ex : example_list) { VW::setup_example(*this->vw_ptr, ex); }

    this->vw_ptr->predict(example_list);
    auto ret = prediction_to_val(example_list[0]->pred, this->vw_ptr->l->get_output_prediction_type());
    finish_example(example_list);
    return ret;
  }

  emscripten::val predict_and_sample(const emscripten::val& example_input, const std::string& uid)
  {
    auto example_list = parse(example_input);

    for (auto* ex : example_list) { VW::setup_example(*this->vw_ptr, ex); }

    this->vw_ptr->predict(example_list);

    std::vector<float> pmf(example_list[0]->pred.a_s.size(), 0);
    for (const auto& as : example_list[0]->pred.a_s) { pmf[as.action] = as.score; }

    const uint64_t seed = VW::uniform_hash(uid.data(), uid.size(), 0) + _pmf_sampling_seed;
    auto chosen_index = this->sample_pmf_internal(pmf, seed);

    auto chosen_a_s = emscripten::val::object();
    chosen_a_s.set("action", chosen_index);
    chosen_a_s.set("score", pmf[chosen_index]);

    finish_example(example_list);
    return chosen_a_s;
  }

  void learn(const emscripten::val& example_input)
  {
    auto example_list = parse(example_input);

    unsigned int length = 0;
    if (!example_input.hasOwnProperty("labels") || (length = example_input["labels"]["length"].as<unsigned int>()) <= 0)
    {
      THROW("labels is missing or empty, can not learn without a label");
    }

    size_t index_offset = 0;
    if (VW::LEARNER::ec_is_example_header(
            *example_list[0], this->vw_ptr->parser_runtime.example_parser->lbl_parser.label_type))
    {
      index_offset = 1;
    }

    for (unsigned int i = 0; i < length; i++)
    {
      const auto& js_object = example_input["labels"][i];
      auto action = js_object["action"].as<uint32_t>();

      if (action + index_offset >= example_list.size()) { THROW("action index out of bounds: " << action); }

      example_list[action + index_offset]->l.cb.costs.push_back(
          {js_object["cost"].as<float>(), js_object["action"].as<uint32_t>(), js_object["probability"].as<float>()});
    }

    for (auto* ex : example_list) { VW::setup_example(*this->vw_ptr, ex); }

    this->vw_ptr->learn(example_list);

    finish_example(example_list);
  }

  void learn_from_string(const emscripten::val& example_input)
  {
    auto example_list = parse(example_input);

    for (auto* ex : example_list) { VW::setup_example(*this->vw_ptr, ex); }

    this->vw_ptr->learn(example_list);

    finish_example(example_list);
  }

private:
  std::vector<VW::example*> parse(const emscripten::val& example_input)
  {
    std::string context;
    if (example_input.hasOwnProperty("text_context")) { context = example_input["text_context"].as<std::string>(); }
    // TODO else if json_context else if embeddings vector of some kind, else throw

    VW::string_view trimmed_ex_str = VW::trim_whitespace(VW::string_view(context));
    std::vector<VW::example*> examples;

    this->vw_ptr->parser_runtime.example_parser->text_reader(this->vw_ptr.get(), trimmed_ex_str, examples);
    assert(!examples.empty());
    return examples;
  }

  void finish_example(std::vector<VW::example*>& example_list)
  {
    assert(!example_list.empty());
    this->vw_ptr->finish_example(example_list);
  }

  uint64_t _pmf_sampling_seed;
};

std::shared_ptr<example_ptr> example_ptr::clone(const vw_model<>& vw_ptr) const
{
  auto* new_ex = &VW::get_unused_example(vw_ptr.vw_ptr.get());
  VW::copy_example_data_with_label(new_ex, _example);
  return wrap_pooled_example(new_ex, vw_ptr.vw_ptr);
}

EMSCRIPTEN_BINDINGS(vwwasm)
{
  emscripten::function("getExceptionMessage", &get_exception_message);

  emscripten::value_object<VW::action_score>("ActionScore")
      .field("action", &VW::action_score::action)
      .field("score", &VW::action_score::score);

  emscripten::class_<example_ptr>("Example")
      .function("clone", &example_ptr::clone)
      .function("toString", &example_ptr::to_string)
      .smart_ptr<std::shared_ptr<example_ptr>>("example_ptr");

  emscripten::enum_<VW::prediction_type_t>("PredictionType")
      .value("scalar", VW::prediction_type_t::SCALAR)
      .value("scalars", VW::prediction_type_t::SCALARS)
      .value("action_scores", VW::prediction_type_t::ACTION_SCORES)
      .value("pdf", VW::prediction_type_t::PDF)
      .value("action_probs", VW::prediction_type_t::ACTION_PROBS)
      .value("multiclass", VW::prediction_type_t::MULTICLASS)
      .value("multilabels", VW::prediction_type_t::MULTILABELS)
      .value("prob", VW::prediction_type_t::PROB)
      .value("multiclassprobs", VW::prediction_type_t::MULTICLASS_PROBS)
      .value("decision_probs", VW::prediction_type_t::DECISION_PROBS)
      .value("actionPDFValue", VW::prediction_type_t::ACTION_PDF_VALUE)
      .value("activeMulticlass", VW::prediction_type_t::ACTIVE_MULTICLASS);

  emscripten::class_<vw_model_basic>("Basic")
      .constructor<std::string>()
      .constructor<std::string, size_t, int>()
      .function("loadModelFromBuffer", &vw_model_basic::load_model_from_buffer)
      .function("getModel", &vw_model_basic::get_model)
      .function("sumLoss", &vw_model_basic::sum_loss)
      .function("weightedLabeledExamples", &vw_model_basic::weighted_labeled_examples)
      .function("predictionType", &vw_model_basic::get_prediction_type)
      .function("createExampleFromDense", &vw_model_basic::create_example_from_dense_features);

  // Currently this is structured such that parse returns a vector of example but to JS that is opaque.
  // All the caller can do is pass this opaque object to the other functions. Is it possible to convert this to a JS
  // array but it involves copying the contents of the array whenever going to/from js/c++ For now it is opaque as the
  // protoyype doesnt support operations on the example itself.
  emscripten::class_<vw_model<>, emscripten::base<vw_model_basic>>("VWModel")
      .constructor<std::string>()
      .constructor<std::string, size_t, int>()
      .function("parse", &vw_model<>::parse)
      .function("predict", &vw_model<>::predict)
      .function("learn", &vw_model<>::learn)
      .function("finishExample", &vw_model<>::finish_example);

  emscripten::class_<cb_vw_model<>, emscripten::base<vw_model_basic>>("VWCBModel")
      .constructor<std::string>()
      .constructor<std::string, size_t, int>()
      .function("predict", &cb_vw_model<>::predict)
      .function("learn", &cb_vw_model<>::learn)
      .function("learnFromString", &cb_vw_model<>::learn_from_string)
      .function("_predictAndSample", &cb_vw_model<>::predict_and_sample)
      .function("_samplePmf", &cb_vw_model<>::sample_pmf)
      .function("setSamplingSeed", &cb_vw_model<>::set_sampling_seed);

  emscripten::register_vector<std::shared_ptr<example_ptr>>("ExamplePtrVector");
  emscripten::register_vector<char>("CharVector");
};
