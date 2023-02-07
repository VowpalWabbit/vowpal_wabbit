// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
// This is the interface for a learning algorithm

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/debug_log.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_type.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/memory.h"
#include "vw/core/metric_sink.h"
#include "vw/core/polymorphic_ex.h"
#include "vw/core/prediction_type.h"
#include "vw/core/scope_exit.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"
#include "vw/core/vw.h"

#include <functional>
#include <iostream>
#include <memory>

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::LEARNER

namespace VW
{
template <typename DataT, typename ExampleT>
using learner_update_stats_func = void(
    const VW::workspace& all, shared_data& sd, const DataT&, const ExampleT&, VW::io::logger& logger);

template <typename DataT, typename ExampleT>
using learner_output_example_prediction_func = void(
    VW::workspace& all, const DataT&, const ExampleT&, VW::io::logger& logger);

template <typename DataT, typename ExampleT>
using learner_print_update_func = void(
    VW::workspace& all, shared_data&, const DataT&, const ExampleT&, VW::io::logger& logger);

template <typename DataT, typename ExampleT>
using learner_cleanup_example_func = void(DataT&, ExampleT&);

/// \brief Contains the VW::LEARNER::learner object and utilities for
/// interacting with it.
namespace LEARNER
{
void generic_driver(VW::workspace& all);
void generic_driver(const std::vector<VW::workspace*>& alls);
void generic_driver_onethread(VW::workspace& all);
bool ec_is_example_header(example const& ec, label_type_t label_type);

// Check that a learner is multiline or singleline.
// Returns the input learner if the check succeeds, or throws exception if it fails.
learner* require_multiline(learner* l);
learner* require_singleline(learner* l);
std::shared_ptr<learner> require_multiline(std::shared_ptr<learner> l);
std::shared_ptr<learner> require_singleline(std::shared_ptr<learner> l);

// Namespace for type definitions and other implementation details.
namespace details
{
using void_func = std::function<void(void)>;
using example_func = std::function<void(polymorphic_ex ex)>;
using multipredict_func =
    std::function<void(polymorphic_ex ex, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)>;

using sensitivity_func = std::function<float(example& ex)>;
using save_load_func = std::function<void(io_buf&, bool read, bool text)>;
using pre_save_load_func = std::function<void(VW::workspace& all)>;
using save_metric_func = std::function<void(metric_sink& metrics)>;

using finish_example_func = std::function<void(VW::workspace&, polymorphic_ex ex)>;
using update_stats_func =
    std::function<void(const VW::workspace&, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger)>;
using output_example_prediction_func =
    std::function<void(VW::workspace&, const polymorphic_ex ex, VW::io::logger& logger)>;
using print_update_func =
    std::function<void(VW::workspace&, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger)>;
using cleanup_example_func = std::function<void(polymorphic_ex ex)>;

// Merge functions come in two variants, with or without the `all` VW::workspace reference
// Use the version without all for reduction learners, where the workspace reference is not needed
// Use the version with all for bottom learners
using merge_func = std::function<void(
    const std::vector<float>& per_model_weighting, const std::vector<const void*>& all_data, void* output_data)>;
using merge_with_all_func = std::function<void(const std::vector<float>& per_model_weighting,
    const std::vector<const VW::workspace*>& all_workspaces, const std::vector<const void*>& all_data,
    VW::workspace& output_workspace, void* output_data)>;
using add_subtract_func = std::function<void(const void* data1, const void* data2, void* data_out)>;
using add_subtract_with_all_func = std::function<void(const VW::workspace& ws1, const void* data1,
    const VW::workspace& ws2, const void* data2, VW::workspace& ws_out, void* data_out)>;

void debug_increment_depth(polymorphic_ex ex);
void debug_decrement_depth(polymorphic_ex ex);
void increment_offset(polymorphic_ex ex, const size_t increment, const size_t i);
void decrement_offset(polymorphic_ex ex, const size_t increment, const size_t i);

void learner_build_diagnostic(VW::string_view this_name, VW::string_view base_name, prediction_type_t in_pred_type,
    prediction_type_t base_out_pred_type, label_type_t out_label_type, label_type_t base_in_label_type,
    details::merge_func merge_f, details::merge_with_all_func merge_with_all_f);
}  // namespace details

/// \brief Defines the interface for a learning algorithm.
///
/// VW has two types of learners: reduction learners and bottom learners. The
/// same learner class is used to implement both. A reduction stack is created
/// as a chain of learners, starting from a bottom learner at the bottom of
/// the stack. Each learner after it is a reduction learner and holds a shared_ptr
/// to its base, the learner immediately below it in the stack. The final VW
/// workspace holds a shared_ptr to the topmost learner. The difference between a
/// reduction and a bottom learner is that a reduction will recursively call its
/// base, whereas a bottom learner has no base and will simply return its result.
///
/// The learner class is not meant to be inherited from. Instead, it implements a
/// sort of virtual inheritance via std::function objects. Each type of learner
/// can have its own data object that stores the learner's internal state. When a
/// learner is created, the function objects are bound to the data object, and the
/// data object is stored in a std::shared_ptr<void>. The end result is that the
/// data type representing the learner's state is type-erased, thus allowing for
/// arbitrary learner types to be implemented by the same learner class.
///
/// The learner class itself has a private constructor. Learners should be created
/// only through the make_reduction_learner and make_bottom_learner functions
/// and their associated learner builder template classes. The templates enforce
/// consistency of types at compile time, before types-erasure occurs to make the
/// acutal learner object.
class learner final : public std::enable_shared_from_this<learner>
{
private:
  void debug_log_message(polymorphic_ex ex, const std::string& msg);

public:
  size_t weights;  // this stores the number of "weight vectors" required by the learner.
  size_t increment;

  // If true, learn will return a prediction. The framework should
  // not call predict before learn
  bool learn_returns_prediction = false;

  void* get_internal_type_erased_data_pointer_test_use_only() { return _learner_data.get(); }

  /// \brief Will update the model according to the labels and examples supplied.
  /// \param ec The ::example object or ::multi_ex to be operated on. This
  /// object **must** have a valid label set for every ::example in the field
  /// example::l that corresponds to the type this learner expects.
  /// \param i This is the offset used for the weights in this call. If using
  /// multiple regressors/learners you can increment this value for each call.
  /// \returns While some learner may fill the example::pred, this is not
  /// guaranteed and is undefined behavior if accessed.
  void learn(polymorphic_ex ec, size_t i = 0);

  /// \brief Make a prediction for the given example.
  /// \param ec The ::example object or ::multi_ex to be operated on. This
  /// object **must** have a valid prediction allocated in the field
  /// example::pred that corresponds to this learner type.
  /// \param i This is the offset used for the weights in this call. If using
  /// multiple regressors/learners you can increment this value for each call.
  /// \returns The prediction calculated by this learner be set on
  /// example::pred. If the polymorphic_ex is ::multi_ex then the prediction is
  /// set on the 0th item in the list.
  void predict(polymorphic_ex ec, size_t i = 0);

  void multipredict(polymorphic_ex ec, size_t lo, size_t count, polyprediction* pred, bool finalize_predictions);

  void update(polymorphic_ex ec, size_t i = 0);

  float sensitivity(example& ec, size_t i = 0);

  // Called anytime saving or loading needs to happen. Autorecursive.
  void save_load(io_buf& io, const bool read, const bool text);

  // Called to edit the command-line from a learner. Autorecursive
  void pre_save_load(VW::workspace& all);

  // Called when metrics is enabled.  Autorecursive.
  void persist_metrics(metric_sink& metrics);

  // Autorecursive
  void finish();

  // Autorecursive
  void end_pass();

  // Called after parsing of examples is complete.  Autorecursive.
  void end_examples();

  // Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver();

  // Called after learn example for each example.  Explicitly not recursive.
  void finish_example(VW::workspace& all, polymorphic_ex ec);

  void update_stats(const VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ec, VW::io::logger& logger);
  void update_stats(VW::workspace& all, const polymorphic_ex ec);

  void output_example_prediction(VW::workspace& all, const polymorphic_ex ec, VW::io::logger& logger);
  void output_example_prediction(VW::workspace& all, const polymorphic_ex ec);

  void print_update(VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ec, VW::io::logger& logger);
  void print_update(VW::workspace& all, const polymorphic_ex ec);

  void cleanup_example(polymorphic_ex ec);

  void get_enabled_learners(std::vector<std::string>& enabled_learners) const;
  learner* get_learner_by_name_prefix(const std::string& learner_name);

  // The functions merge/add/subtract are NOT auto recursive
  // They are effectively static implementing a trait for this learner type.
  void merge(const std::vector<float>& per_model_weighting, const std::vector<const VW::workspace*>& all_workspaces,
      const std::vector<const learner*>& all_learners, VW::workspace& output_workspace, learner& output_learner);
  void add(const VW::workspace& base_ws, const VW::workspace& delta_ws, const learner* base_l, const learner* delta_l,
      VW::workspace& output_ws, learner* output_l);
  void subtract(const VW::workspace& ws1, const VW::workspace& ws2, const learner* l1, const learner* l2,
      VW::workspace& output_ws, learner* output_l);

  // Get properties of this learner
  VW_ATTR(nodiscard) bool has_legacy_finish() const { return _finish_example_f != nullptr; }
  VW_ATTR(nodiscard) bool has_update_stats() const { return _update_stats_f != nullptr; }
  VW_ATTR(nodiscard) bool has_print_update() const { return _print_update_f != nullptr; }
  VW_ATTR(nodiscard) bool has_output_example_prediction() const { return _output_example_prediction_f != nullptr; }
  VW_ATTR(nodiscard) bool has_cleanup_example() const { return _cleanup_example_f != nullptr; }
  VW_ATTR(nodiscard) bool has_merge() const { return (_merge_with_all_f != nullptr) || (_merge_f != nullptr); }
  VW_ATTR(nodiscard) bool has_add() const { return (_add_with_all_f != nullptr) || (_add_f != nullptr); }
  VW_ATTR(nodiscard) bool has_subtract() const { return (_subtract_with_all_f != nullptr) || (_subtract_f != nullptr); }

  // Get prediction and label types
  VW_ATTR(nodiscard) prediction_type_t get_output_prediction_type() const { return _output_pred_type; }
  VW_ATTR(nodiscard) prediction_type_t get_input_prediction_type() const { return _input_pred_type; }
  VW_ATTR(nodiscard) label_type_t get_output_label_type() const { return _output_label_type; }
  VW_ATTR(nodiscard) label_type_t get_input_label_type() const { return _input_label_type; }

  // Is this learner multiline or singleline?
  VW_ATTR(nodiscard) bool is_multiline() const { return _is_multiline; }

  // Get the learner's name
  VW_ATTR(nodiscard) const std::string& get_name() const { return _name; }

  // Returns a pointer to the base of this learner, which is immediately below this one
  // Returns nullptr if this is the bottom learner of the stack (the bottom-most learner)
  VW_ATTR(nodiscard) const learner* get_base_learner() const { return _base_learner.get(); }
  VW_ATTR(nodiscard) learner* get_base_learner() { return _base_learner.get(); }

  // If true, this specific learner defines a save load function.
  // If false, it simply forwards to the base learner's implementation.
  VW_ATTR(nodiscard) bool learner_defines_own_save_load() { return _save_load_f != nullptr; }

private:
  // Name of the learner. Used in VW_DBG to trace nested learn() and predict() calls.
  std::string _name;

  // Is this a single-line or multi-line learner?
  bool _is_multiline;

  // Input and output data types
  prediction_type_t _output_pred_type;
  prediction_type_t _input_pred_type;
  label_type_t _output_label_type;
  label_type_t _input_label_type;

  // These functions will implement the internal logic of each type of learner.
  details::void_func _init_f;
  details::example_func _learn_f;
  details::example_func _predict_f;
  details::example_func _update_f;
  details::multipredict_func _multipredict_f;
  details::sensitivity_func _sensitivity_f;

  details::finish_example_func _finish_example_f;
  details::update_stats_func _update_stats_f;
  details::output_example_prediction_func _output_example_prediction_f;
  details::print_update_func _print_update_f;
  details::cleanup_example_func _cleanup_example_f;

  details::save_load_func _save_load_f;
  details::void_func _end_pass_f;
  details::void_func _end_examples_f;
  details::pre_save_load_func _pre_save_load_f;
  details::save_metric_func _persist_metrics_f;
  details::void_func _finisher_f;

  // Functions for model merging. Each comes in two variants, with or without the VW::workspace* all pointer.
  // There should only ever be either none or one of these two variants set. Never both.
  details::merge_with_all_func _merge_with_all_f;
  details::merge_func _merge_f;
  details::add_subtract_func _add_f;
  details::add_subtract_with_all_func _add_with_all_f;
  details::add_subtract_func _subtract_f;
  details::add_subtract_with_all_func _subtract_with_all_f;

  // This holds ownership of learner data as a type-erased void pointer.
  // Functions needing access to data should bind a raw pointer when the learner is created, before type erasure.
  // Except for model merging functions, where the void* will get casted back into the original data type.
  std::shared_ptr<void> _learner_data;

  // This holds ownership of this learner's base.
  // It is the learner immediately below this one in the reduction stack.
  // For bottom learners, this will be nullptr.
  std::shared_ptr<learner> _base_learner;

  // Create a copy of this learner. The implementation of this functions determines which of the
  // functions inside the learner are propagated to the new learner, and which are reset to nullptr.
  // The new learner will share ownership of this learner in its _base_learner shared pointer.
  std::shared_ptr<learner> create_learner_above_this();

  // Private constructor/copy/move
  // Should only be able to construct a std::shared_ptr<learner> through make_reduction_learner /
  // make_bottom_learner. All learners must be owned by a std::shared_ptr because we use
  // std::enable_shared_from_this
  learner() = default;
  learner(const learner&) = default;
  learner(learner&&) = default;

  // Can't assign to learner
  learner& operator=(const learner&) = delete;
  learner& operator=(learner&&) = delete;

  // Give builders access to private class members.
  template <class FluentBuilderT, class DataT, class ExampleT>
  friend class common_learner_builder;
  template <class DataT, class ExampleT>
  friend class bottom_learner_builder;
  template <class DataT, class ExampleT>
  friend class reduction_learner_builder;
  template <class ExampleT>
  friend class reduction_no_data_learner_builder;
};

template <bool is_learn>
void multiline_learn_or_predict(learner& base, multi_ex& examples, const uint64_t offset, const uint32_t id = 0)
{
  std::vector<uint64_t> saved_offsets;
  saved_offsets.reserve(examples.size());
  for (auto ec : examples)
  {
    saved_offsets.push_back(ec->ft_offset);
    ec->ft_offset = offset;
  }

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit(
      [&saved_offsets, &examples]
      {
        for (size_t i = 0; i < examples.size(); i++) { examples[i]->ft_offset = saved_offsets[i]; }
      });

  if (is_learn) { base.learn(examples, id); }
  else { base.predict(examples, id); }
}

namespace details
{
inline float recur_sensitivity(learner& base, example& ec) { return base.sensitivity(ec); }

template <typename DataT>
inline float recur_sensitivity(DataT&, learner& base, example& ec)
{
  return base.sensitivity(ec);
}
}  // namespace details

// Helper macro for defining builder functions. This will generate two copies
// of a member function, one lvalue-qualified and one rvalue-qualified.
// The builder class should define a type named FluentBuilderT through either
// a template parameter or a using-statement. This will be used to determine
// the return types of the generated functions.
#define LEARNER_BUILDER_DEFINE(func, ...)                                           \
  FluentBuilderT& func& { __VA_ARGS__ return static_cast<FluentBuilderT&>(*this); } \
  FluentBuilderT&& func&& { __VA_ARGS__ return static_cast<FluentBuilderT&&>(*this); }

template <class FluentBuilderT, class DataT, class ExampleT>
class common_learner_builder
{
  // Compile time check for valid ExampleT
  static_assert(
      VW::is_example_type<ExampleT>::value, "Learner builder can only be used with VW example or multi_ex types");

public:
  // The learner being created by this builder
  std::shared_ptr<learner> learner_ptr = nullptr;
  // Pointer to learner's data before type erasure, used for binding functions
  std::shared_ptr<DataT> learner_data = nullptr;

  common_learner_builder(std::shared_ptr<learner> input_learner, std::unique_ptr<DataT>&& data, const std::string& name)
  {
    learner_data = std::move(data);
    learner_ptr = std::move(input_learner);
    learner_ptr->_name = name;
    learner_ptr->_is_multiline = VW::is_multiline_type<ExampleT>::value;
    learner_ptr->_learner_data = learner_data;
  }

  // delete copy constructors
  common_learner_builder(const common_learner_builder&) = delete;
  common_learner_builder& operator=(const common_learner_builder&) = delete;

  // default move constructors
  common_learner_builder(common_learner_builder&&) noexcept = default;
  common_learner_builder& operator=(common_learner_builder&&) noexcept = default;

  // clang-format off
  LEARNER_BUILDER_DEFINE(set_learn_returns_prediction(bool learn_returns_prediction),
    learner_ptr->learn_returns_prediction = learn_returns_prediction;
  )

  LEARNER_BUILDER_DEFINE(set_save_load(void (*fn_ptr)(DataT&, io_buf&, bool, bool)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_save_load_f = [fn_ptr, data](io_buf& buf, bool read, bool text)
    { fn_ptr(*data, buf, read, text); };
  )

  LEARNER_BUILDER_DEFINE(set_finish(void (*fn_ptr)(DataT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_finisher_f = [fn_ptr, data]() { fn_ptr(*data); };
  )

  LEARNER_BUILDER_DEFINE(set_end_pass(void (*fn_ptr)(DataT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_end_pass_f = [fn_ptr, data]() { fn_ptr(*data); };
  )

  LEARNER_BUILDER_DEFINE(set_end_examples(void (*fn_ptr)(DataT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_end_examples_f = [fn_ptr, data]() { fn_ptr(*data); };
  )

  LEARNER_BUILDER_DEFINE(set_init_driver(void (*fn_ptr)(DataT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_init_f = [fn_ptr, data]() { fn_ptr(*data); };
  )

  LEARNER_BUILDER_DEFINE(set_finish_example(void (*fn_ptr)(VW::workspace& all, DataT&, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_finish_example_f = [fn_ptr, data](VW::workspace& all, polymorphic_ex ex)
    { fn_ptr(all, *data, ex); };
  )

  // Responsibilities of update stats:
  // - Call shared_data::update
  LEARNER_BUILDER_DEFINE(set_update_stats(learner_update_stats_func<DataT, ExampleT>* fn_ptr),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_update_stats_f =
        [fn_ptr, data](const VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger)
    { fn_ptr(all, sd, *data, ex, logger); };
  )

  // Responsibilities of output example prediction:
  // - Output predictions
  LEARNER_BUILDER_DEFINE(set_output_example_prediction(learner_output_example_prediction_func<DataT, ExampleT>* fn_ptr),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_output_example_prediction_f = [fn_ptr, data](VW::workspace& all, const polymorphic_ex ex,
        VW::io::logger& logger) { fn_ptr(all, *data, ex, logger); };
  )

  // Responsibilities of output example prediction:
  // - Call shared_data::print_update
  // Note this is only called when required based on the user specified backoff and logging settings.
  LEARNER_BUILDER_DEFINE(set_print_update(learner_print_update_func<DataT, ExampleT>* fn_ptr),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_print_update_f =
        [fn_ptr, data](VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger)
    { fn_ptr(all, sd, *data, ex, logger); };
  )

  // This call is **optional**, correctness cannot depend on it.
  // However, it can be used to optimistically reuse memory.
  LEARNER_BUILDER_DEFINE(set_cleanup_example(learner_cleanup_example_func<DataT, ExampleT>* fn_ptr),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_cleanup_example_f = [fn_ptr, data](polymorphic_ex ex) { fn_ptr(*data, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_persist_metrics(void (*fn_ptr)(DataT&, metric_sink&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_persist_metrics_f = [fn_ptr, data](metric_sink& metrics) { fn_ptr(*data, metrics); };
  )

  LEARNER_BUILDER_DEFINE(set_pre_save_load(void (*fn_ptr)(VW::workspace& all, DataT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_pre_save_load_f = [fn_ptr, data](VW::workspace& all) { fn_ptr(all, *data); };
  )

  // This is the label type of the example passed into the learn function. This
  // label will be operated on throughout the learn function.
  LEARNER_BUILDER_DEFINE(set_input_label_type(label_type_t label_type),
    this->learner_ptr->_input_label_type = label_type;
  )

  // This is the label type of the example fed into the base in the learn function.
  // This will reference the state of the label after it has been operated on throughout
  // the learn function.
  LEARNER_BUILDER_DEFINE(set_output_label_type(label_type_t label_type),
    this->learner_ptr->_output_label_type = label_type;
  )

  // This is the prediction type received when calling predict from the base at
  // the top of the predict function. Note that the prediction from the example
  // passed directly into the predict function has no defined type.
  LEARNER_BUILDER_DEFINE(set_input_prediction_type(prediction_type_t pred_type),
    this->learner_ptr->_input_pred_type = pred_type;
  )

  // This is the prediction type of the example at the end of the predict function.
  // This prediction will be passed when the learner above it calls predict on its base.
  LEARNER_BUILDER_DEFINE(set_output_prediction_type(prediction_type_t pred_type),
    this->learner_ptr->_output_pred_type = pred_type;
  )
  // clang-format on
};

template <class DataT, class ExampleT>
class reduction_learner_builder
    : public common_learner_builder<reduction_learner_builder<DataT, ExampleT>, DataT, ExampleT>
{
public:
  using FluentBuilderT = reduction_learner_builder<DataT, ExampleT>;
  using super = common_learner_builder<reduction_learner_builder<DataT, ExampleT>, DataT, ExampleT>;
  reduction_learner_builder(std::unique_ptr<DataT>&& data, std::shared_ptr<learner> base, const std::string& name)
      // NOTE: This is a copy of the base learner! The purpose is to copy all of the
      // function data objects so that if this learner does not define a function such as
      // save_load then calling save_load on this object will essentially result in forwarding the
      // call the next learner that actually implements it.
      : common_learner_builder<reduction_learner_builder<DataT, ExampleT>, DataT, ExampleT>(
            base->create_learner_above_this(), std::move(data), name)
  {
    // Default sensitivity calls the base learner's sensitivity recursively
    this->set_sensitivity(details::recur_sensitivity<DataT>);

    set_params_per_weight(1);
    this->set_learn_returns_prediction(false);

    // By default, will produce what the base learner expects
    super::set_input_label_type(base->get_input_label_type());
    super::set_output_label_type(base->get_input_label_type());
    super::set_input_prediction_type(base->get_output_prediction_type());
    super::set_output_prediction_type(base->get_output_prediction_type());
  }

  // clang-format off
  LEARNER_BUILDER_DEFINE(set_predict(void (*fn_ptr)(DataT&, learner&, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_predict_f = [fn_ptr, data, base](polymorphic_ex ex) { fn_ptr(*data, *base, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_learn(void (*fn_ptr)(DataT&, learner&, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_learn_f = [fn_ptr, data, base](polymorphic_ex ex) { fn_ptr(*data, *base, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_multipredict(void (*fn_ptr)(DataT&, learner&, ExampleT&, size_t, size_t, polyprediction*, bool)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_multipredict_f = [fn_ptr, data, base](polymorphic_ex ex, size_t count, size_t step,
        polyprediction* pred, bool finalize_predictions)
    { fn_ptr(*data, *base, ex, count, step, pred, finalize_predictions); };
  )

  LEARNER_BUILDER_DEFINE(set_update(void (*fn_ptr)(DataT& data, learner&, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_update_f = [fn_ptr, data, base](polymorphic_ex ex) { fn_ptr(*data, *base, ex); };
  )

  // used for active learning and confidence to determine how easily predictions are changed
  LEARNER_BUILDER_DEFINE(set_sensitivity(float (*fn_ptr)(DataT& data, learner&, example&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_sensitivity_f = [fn_ptr, data, base](example& ex) { return fn_ptr(*data, *base, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_params_per_weight(size_t params_per_weight),
    this->learner_ptr->weights = params_per_weight;
    this->learner_ptr->increment = this->learner_ptr->_base_learner->increment * this->learner_ptr->weights;
  )

  LEARNER_BUILDER_DEFINE(set_merge(void (*fn_ptr)(const std::vector<float>&, const std::vector<const DataT*>&, DataT&)),
    assert(fn_ptr != nullptr);
    this->learner_ptr->_merge_f = [fn_ptr](const std::vector<float>& per_model_weighting,
        const std::vector<const void*>& all_data, void* output_data)
    {
      fn_ptr(per_model_weighting, reinterpret_cast<const std::vector<const DataT*>&>(all_data),
          *static_cast<DataT*>(output_data));
    };
  )

  LEARNER_BUILDER_DEFINE(set_add(void (*fn_ptr)(const DataT&, const DataT&, DataT&)),
    assert(fn_ptr != nullptr);
    this->learner_ptr->_add_f = [fn_ptr](const void* data1, const void* data2, void* data_out)
    { fn_ptr(*static_cast<const DataT*>(data1), *static_cast<const DataT*>(data2), *static_cast<DataT*>(data_out)); };
  )

  LEARNER_BUILDER_DEFINE(set_subtract(void (*fn_ptr)(const DataT&, const DataT&, DataT&)),
    assert(fn_ptr != nullptr);
    this->learner_ptr->_subtract_f = [fn_ptr](const void* data1, const void* data2, void* data_out)
    { fn_ptr(*static_cast<const DataT*>(data1), *static_cast<const DataT*>(data2), *static_cast<DataT*>(data_out)); };
  )
  // clang-format on

  std::shared_ptr<learner> build()
  {
    prediction_type_t in_pred_type = this->learner_ptr->get_input_prediction_type();
    prediction_type_t base_out_pred_type = this->learner_ptr->get_base_learner()->get_output_prediction_type();
    label_type_t out_label_type = this->learner_ptr->get_output_label_type();
    label_type_t base_in_label_type = this->learner_ptr->get_base_learner()->get_input_label_type();
    details::learner_build_diagnostic(this->learner_ptr->get_name(), this->learner_ptr->get_base_learner()->get_name(),
        in_pred_type, base_out_pred_type, out_label_type, base_in_label_type, this->learner_ptr->_merge_f,
        this->learner_ptr->_merge_with_all_f);

    return this->learner_ptr;
  }
};

template <class ExampleT>
class reduction_no_data_learner_builder
    : public common_learner_builder<reduction_learner_builder<char, ExampleT>, char, ExampleT>
{
public:
  using FluentBuilderT = reduction_no_data_learner_builder<ExampleT>;
  using super = common_learner_builder<reduction_learner_builder<char, ExampleT>, char, ExampleT>;
  reduction_no_data_learner_builder(std::shared_ptr<learner> base, const std::string& name)
      // NOTE: This is a copy of the base learner! The purpose is to copy all of the
      // function data objects so that if this learner does not define a function such as
      // save_load then calling save_load on this object will essentially result in forwarding the
      // call the next learner that actually implements it.

      // For the no data reduction, allocate a placeholder char as its data to avoid nullptr issues
      : common_learner_builder<reduction_learner_builder<char, ExampleT>, char, ExampleT>(
            base->create_learner_above_this(), VW::make_unique<char>(0), name)
  {
    // Default sensitivity calls base learner's sensitivity recursively
    this->set_sensitivity(details::recur_sensitivity);

    set_params_per_weight(1);
    // By default, will produce what the base learner expects
    super::set_input_label_type(base->get_input_label_type());
    super::set_output_label_type(base->get_input_label_type());
    super::set_input_prediction_type(base->get_output_prediction_type());
    super::set_output_prediction_type(base->get_output_prediction_type());
  }

  // clang-format off
  LEARNER_BUILDER_DEFINE(set_predict(void (*fn_ptr)(learner&, ExampleT&)),
    assert(fn_ptr != nullptr);
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_predict_f = [fn_ptr, base](polymorphic_ex ex) { fn_ptr(*base, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_learn(void (*fn_ptr)(learner&, ExampleT&)),
    assert(fn_ptr != nullptr);
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_learn_f = [fn_ptr, base](polymorphic_ex ex) { fn_ptr(*base, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_multipredict(void (*fn_ptr)(learner&, ExampleT&, size_t, size_t, polyprediction*, bool)),
    assert(fn_ptr != nullptr);
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_multipredict_f =
        [fn_ptr, base](polymorphic_ex ex, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)
    { fn_ptr(*base, ex, count, step, pred, finalize_predictions); };
  )

  LEARNER_BUILDER_DEFINE(set_update(void (*fn_ptr)(learner&, ExampleT&)),
    assert(fn_ptr != nullptr);
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_update_f = [fn_ptr, base](polymorphic_ex ex) { fn_ptr(*base, ex); };
  )

  // used for active learning and confidence to determine how easily predictions are changed
  LEARNER_BUILDER_DEFINE(set_sensitivity(float (*fn_ptr)(learner&, example&)),
    assert(fn_ptr != nullptr);
    learner* base = this->learner_ptr->get_base_learner();
    this->learner_ptr->_sensitivity_f = [fn_ptr, base](example& ex) { return fn_ptr(*base, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_params_per_weight(size_t params_per_weight),
    this->learner_ptr->weights = params_per_weight;
    this->learner_ptr->increment = this->learner_ptr->_base_learner->increment * this->learner_ptr->weights;
  )
  // clang-format on

  std::shared_ptr<learner> build() { return this->learner_ptr; }
};

template <class DataT, class ExampleT>
class bottom_learner_builder : public common_learner_builder<bottom_learner_builder<DataT, ExampleT>, DataT, ExampleT>
{
public:
  using FluentBuilderT = bottom_learner_builder<DataT, ExampleT>;
  using super = common_learner_builder<bottom_learner_builder<DataT, ExampleT>, DataT, ExampleT>;
  bottom_learner_builder(std::unique_ptr<DataT>&& data, const std::string& name, prediction_type_t out_pred_type,
      label_type_t in_label_type)
      : common_learner_builder<bottom_learner_builder<DataT, ExampleT>, DataT, ExampleT>(
            std::shared_ptr<learner>(new learner()), std::move(data), name)
  {
    // Default sensitivity function returns zero
    this->learner_ptr->_sensitivity_f = [](example&) { return 0.f; };

    super::set_input_label_type(in_label_type);
    super::set_output_label_type(label_type_t::NOLABEL);
    super::set_input_prediction_type(prediction_type_t::NOPRED);
    super::set_output_prediction_type(out_pred_type);

    set_params_per_weight(1);
  }

  // clang-format off
  LEARNER_BUILDER_DEFINE(set_predict(void (*fn_ptr)(DataT&, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_predict_f = [fn_ptr, data](polymorphic_ex ex) { fn_ptr(*data, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_learn(void (*fn_ptr)(DataT&, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_learn_f = [fn_ptr, data](polymorphic_ex ex) { fn_ptr(*data, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_multipredict( void (*fn_ptr)(DataT&, ExampleT&, size_t, size_t, polyprediction*, bool)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_multipredict_f =
        [fn_ptr, data](polymorphic_ex ex, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)
    { fn_ptr(*data, ex, count, step, pred, finalize_predictions); };
  )

  LEARNER_BUILDER_DEFINE(set_update(void (*fn_ptr)(DataT& data, ExampleT&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_update_f = [fn_ptr, data](polymorphic_ex ex) { fn_ptr(*data, ex); };
  )

  // used for active learning and confidence to determine how easily predictions are changed
  LEARNER_BUILDER_DEFINE(set_sensitivity(float (*fn_ptr)(DataT& data, example&)),
    assert(fn_ptr != nullptr);
    DataT* data = this->learner_data.get();
    this->learner_ptr->_sensitivity_f = [fn_ptr, data](example& ex) { return fn_ptr(*data, ex); };
  )

  LEARNER_BUILDER_DEFINE(set_params_per_weight(size_t params_per_weight),
    this->learner_ptr->weights = 1;
    this->learner_ptr->increment = params_per_weight;
  )

  LEARNER_BUILDER_DEFINE(set_merge_with_all(void (*fn_ptr)(const std::vector<float>&,
      const std::vector<const VW::workspace*>&, const std::vector<const DataT*>&, VW::workspace&, DataT&)),
    assert(fn_ptr != nullptr);
    this->learner_ptr->_merge_with_all_f =
        [fn_ptr](const std::vector<float>& per_model_weighting, const std::vector<const VW::workspace*>& all_workspaces,
            const std::vector<const void*>& all_data, VW::workspace& output_workspace, void* output_data)
    {
      fn_ptr(per_model_weighting, all_workspaces, reinterpret_cast<const std::vector<const DataT*>&>(all_data),
          output_workspace, *static_cast<DataT*>(output_data));
    };
  )

  LEARNER_BUILDER_DEFINE(set_add_with_all(
      void (*fn_ptr)(const VW::workspace&, const DataT&, const VW::workspace&, const DataT&, VW::workspace&, DataT&)),
    assert(fn_ptr != nullptr);
    this->learner_ptr->_add_with_all_f =
        [fn_ptr](const VW::workspace& ws1, const void* data1, const VW::workspace& ws2, const void* data2,
            VW::workspace& ws_out, void* data_out)
    {
      fn_ptr(ws1, *static_cast<const DataT*>(data1), ws2, *static_cast<const DataT*>(data2), ws_out,
          *static_cast<DataT*>(data_out));
    };
  )

  LEARNER_BUILDER_DEFINE(set_subtract_with_all(
      void (*fn_ptr)(const VW::workspace&, const DataT&, const VW::workspace&, const DataT&, VW::workspace&, DataT&)),
    assert(fn_ptr != nullptr);
    this->learner_ptr->_subtract_with_all_f =
        [fn_ptr](const VW::workspace& ws1, const void* data1, const VW::workspace& ws2, const void* data2,
            VW::workspace& ws_out, void* data_out)
    {
      fn_ptr(ws1, *static_cast<const DataT*>(data1), ws2, *static_cast<const DataT*>(data2), ws_out,
          *static_cast<DataT*>(data_out));
    };
  )
  // clang-format on

  std::shared_ptr<learner> build()
  {
    if (this->learner_ptr->_merge_f && this->learner_ptr->_merge_with_all_f)
    {
      THROW("cannot set both merge and merge_with_all");
    }
    return this->learner_ptr;
  }
};

template <class DataT, class ExampleT>
reduction_learner_builder<DataT, ExampleT> make_reduction_learner(std::unique_ptr<DataT>&& data,
    std::shared_ptr<learner> base, void (*learn_fn)(DataT&, learner&, ExampleT&),
    void (*predict_fn)(DataT&, learner&, ExampleT&), const std::string& name)
{
  auto builder = reduction_learner_builder<DataT, ExampleT>(std::move(data), std::move(base), name);
  builder.set_learn(learn_fn);
  builder.set_update(learn_fn);
  builder.set_predict(predict_fn);
  return builder;
}

template <class ExampleT>
reduction_no_data_learner_builder<ExampleT> make_no_data_reduction_learner(std::shared_ptr<learner> base,
    void (*learn_fn)(learner&, ExampleT&), void (*predict_fn)(learner&, ExampleT&), const std::string& name)
{
  auto builder = reduction_no_data_learner_builder<ExampleT>(std::move(base), name);
  builder.set_learn(learn_fn);
  builder.set_update(learn_fn);
  builder.set_predict(predict_fn);
  return builder;
}

template <class DataT, class ExampleT>
bottom_learner_builder<DataT, ExampleT> make_bottom_learner(std::unique_ptr<DataT>&& data,
    void (*learn_fn)(DataT&, ExampleT&), void (*predict_fn)(DataT&, ExampleT&), const std::string& name,
    prediction_type_t out_pred_type, label_type_t in_label_type)
{
  auto builder = bottom_learner_builder<DataT, ExampleT>(std::move(data), name, out_pred_type, in_label_type);
  builder.set_learn(learn_fn);
  builder.set_update(learn_fn);
  builder.set_predict(predict_fn);
  return builder;
}

template <class ExampleT>
bottom_learner_builder<char, ExampleT> make_no_data_bottom_learner(void (*learn_fn)(char&, ExampleT&),
    void (*predict_fn)(char&, ExampleT&), const std::string& name, prediction_type_t out_pred_type,
    label_type_t in_label_type)
{
  // For the no data bottom learner, allocate a placeholder char as its data to avoid nullptr issues
  return make_bottom_learner<char, ExampleT>(
      VW::make_unique<char>(0), learn_fn, predict_fn, name, out_pred_type, in_label_type);
}

}  // namespace LEARNER
}  // namespace VW