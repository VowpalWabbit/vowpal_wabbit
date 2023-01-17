// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
// This is the interface for a learning algorithm

#include "vw/core/learner_fwd.h"
#include "vw/core/polymorphic_ex.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"

#include <iostream>
#include <memory>

#ifdef _WIN32
#  pragma warning(push)
#  pragma warning(disable : 4635)
// Warnings emitted from this header are unrelated to this project.
//     format.h(3525): warning C4635: XML document comment applied to
//     'fmt.v7.format_system_error(fmt.v7.detail.buffer<System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte>*!System.Runtime.CompilerServices.IsImplicitlyDereferenced,System.Int32,fmt.v7.basic_string_view<System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte>)':
//     badly-formed XML: Invalid at the top level of the document.
#endif
#include "fmt/core.h"
#ifdef _WIN32
#  pragma warning(pop)
#endif

#include "vw/core/vw_string_view_fmt.h"

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/debug_log.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_type.h"
#include "vw/core/memory.h"
#include "vw/core/metric_sink.h"
#include "vw/core/prediction_type.h"
#include "vw/core/scope_exit.h"
#include "vw/core/vw.h"

#include <functional>

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

namespace details
{
using void_func = std::function<void (void)>;
using base_example_func = std::function<void (learner& base, polymorphic_ex ex)>;
using multipredict_func = std::function<void (learner& base, polymorphic_ex ex, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)>;

using sensitivity_func = std::function<float (learner& base, example& ex)>;
using save_load_func = std::function<void (io_buf&, bool read, bool text)>;
using pre_save_load_func = std::function<void (VW::workspace& all)>;
using save_metric_func = std::function<void (metric_sink& metrics)>;

using finish_example_func = std::function<void (VW::workspace&, polymorphic_ex ex)>;
using update_stats_func = std::function<void (const VW::workspace&, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger)>;
using output_example_prediction_func = std::function<void (VW::workspace&, const polymorphic_ex ex, VW::io::logger& logger)>;
using print_update_func = std::function<void (VW::workspace&, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger)>;
using cleanup_example_func = std::function<void (polymorphic_ex ex)>;

using merge_with_all_func = std::function<void (const std::vector<float>& per_model_weighting,
    const std::vector<const VW::workspace*>& all_workspaces, const std::vector<const void*>& all_data,
    VW::workspace& output_workspace, void* output_data)>;
using merge_with_all_fptr = void(*)(const std::vector<float>& per_model_weighting,
    const std::vector<const VW::workspace*>& all_workspaces, const std::vector<const void*>& all_data,
    VW::workspace& output_workspace, void* output_data);
// When the workspace reference is not needed this signature should definitely be used.
using merge_func = std::function<void (
    const std::vector<float>& per_model_weighting, const std::vector<const void*>& all_data, void* output_data)>;
using merge_fptr = void(*)(const std::vector<float>& per_model_weighting, const std::vector<const void*>& all_data, void* output_data);

using add_subtract_func = std::function<void (const void* data_1, const void* data_2, void* data_out)>;
using add_subtract_fptr = void(*)(const void* data_1, const void* data_2, void* data_out);
using add_subtract_with_all_func = std::function<void (const VW::workspace& ws1, const void* data1, const VW::workspace& ws2,
    const void* data2, VW::workspace& ws_out, void* data_out)>;
using add_subtract_with_all_fptr = void(*)(const VW::workspace& ws1, const void* data1, const VW::workspace& ws2,
    const void* data2, VW::workspace& ws_out, void* data_out);

void debug_increment_depth(polymorphic_ex ex);
void debug_decrement_depth(polymorphic_ex ex);
void increment_offset(polymorphic_ex ex, const size_t increment, const size_t i);
void decrement_offset(polymorphic_ex ex, const size_t increment, const size_t i);

void learner_build_diagnostic(VW::string_view this_name, VW::string_view base_name, prediction_type_t in_pred_type,
    prediction_type_t base_out_pred_type, label_type_t out_label_type, label_type_t base_in_label_type,
    details::merge_func merge_func_ptr, details::merge_with_all_func merge_with_all_func_ptr);
}  // namespace details

/// \brief Defines the interface for a learning algorithm.
///
/// Learner is implemented as a class of pointers, and associated methods. It
/// implements a sort of virtual inheritance through the use of bundling
/// function pointers with the associated objects to call them with. A reduction
/// will recursively call the base given to it, whereas a base learner will not
/// recurse and will simply return the result. Learner is not intended to be
/// inherited from. Instead it is used through composition, and created through
/// the make_reduction_learner and make_base_learner. The state of this
/// learner, or reduction, is stored in the learner_data field. A
/// <code>std::shared_pointer<void></code> is used as this class uses type
/// erasure to allow for an arbitrary reduction to be implemented. It is
/// extremely important that the function pointers given to the class match the
/// expected types of the object. If the learner is constructed using
/// make_reduction_learner or make_base_learner and assembled before it is
/// transformed into a VW::LEARNER::learner with VW::LEARNER::make_base then
/// the usage of the templated functions should ensure types are correct.
///
/// \tparam T Type of the reduction data object stored. This allows this
/// specific reduction to have it's own state.
/// \tparam E Example type this reduction supports. Must be one of ::example or
/// ::multi_ex
class learner
{
  /// \private
  void debug_log_message(polymorphic_ex ex, const std::string& msg)
  {
    if (ex.is_multiline()) {
      VW_DBG(*static_cast<VW::multi_ex&>(ex)[0]) << "[" << _name << "." << msg << "]" << std::endl;
    } else {
      VW_DBG(static_cast<VW::example&>(ex)) << "[" << _name << "." << msg << "]" << std::endl;
    }
  }

  // Used as a hook to intercept incorrect calls to the base learner.
  void debug_log_message(const char& /* ec */, const std::string& msg)
  {
    auto message =
        fmt::format("Learner: '{}', function: '{}' was called without first being cast to singleline or multiline.",
            get_name(), msg);
    THROW(message);
  }

public:
  size_t weights;  // this stores the number of "weight vectors" required by the learner.
  size_t increment;

  // learn will return a prediction.  The framework should
  // not call predict before learn
  bool learn_returns_prediction = false;

  void* get_internal_type_erased_data_pointer_test_use_only() { return _learner_data.get(); }

  // For all functions here that invoke stored function pointers,
  // NO_SANITIZE_UNDEFINED is needed because the function pointer's type may be
  // cast to something different from the original function's signature.
  // This will throw an error in UndefinedBehaviorSanitizer even when the
  // function can be correctly called through the pointer.

  /// \brief Will update the model according to the labels and examples supplied.
  /// \param ec The ::example object or ::multi_ex to be operated on. This
  /// object **must** have a valid label set for every ::example in the field
  /// example::l that corresponds to the type this reduction expects.
  /// \param i This is the offset used for the weights in this call. If using
  /// multiple regressors/learners you can increment this value for each call.
  /// \returns While some reductions may fill the example::pred, this is not
  /// guaranteed and is undefined behavior if accessed.
  inline void learn(polymorphic_ex ec, size_t i = 0)
  {
    assert(is_multiline() == ec.is_multiline());
    details::increment_offset(ec, increment, i);
    debug_log_message(ec, "learn");
    _learn_f(*_base_learner, ec);
    details::decrement_offset(ec, increment, i);
  }

  /// \brief Make a prediction for the given example.
  /// \param ec The ::example object or ::multi_ex to be operated on. This
  /// object **must** have a valid prediction allocated in the field
  /// example::pred that corresponds to this reduction type.
  /// \param i This is the offset used for the weights in this call. If using
  /// multiple regressors/learners you can increment this value for each call.
  /// \returns The prediction calculated by this reduction be set on
  /// example::pred. If <code>E</code> is ::multi_ex then the prediction is set
  /// on the 0th item in the list.
  inline void predict(polymorphic_ex ec, size_t i = 0)
  {
    assert(is_multiline() == ec.is_multiline());
    details::increment_offset(ec, increment, i);
    debug_log_message(ec, "predict");
    _predict_f(*_base_learner, ec);
    details::decrement_offset(ec, increment, i);
  }

  inline void multipredict(
      polymorphic_ex ec, size_t lo, size_t count, polyprediction* pred, bool finalize_predictions)
  {
    assert(is_multiline() == ec.is_multiline());
    if (_multipredict_f == nullptr)
    {
      details::increment_offset(ec, increment, lo);
      debug_log_message(ec, "multipredict");
      for (size_t c = 0; c < count; c++)
      {
        _predict_f(*_base_learner, ec);
        if (finalize_predictions)
        {
          pred[c] = std::move(static_cast<VW::example&>(ec).pred);  // TODO: this breaks for complex labels because = doesn't do deep copy! (XXX we
                                         // "fix" this by moving)
        }
        else { pred[c].scalar = static_cast<VW::example&>(ec).partial_prediction; }
        // pred[c].scalar = finalize_prediction ec.partial_prediction; // TODO: this breaks for complex labels because =
        // doesn't do deep copy! // note works if ec.partial_prediction, but only if finalize_prediction is run????
        details::increment_offset(ec, increment, 1);
      }
      details::decrement_offset(ec, increment, lo + count);
    }
    else
    {
      details::increment_offset(ec, increment, lo);
      debug_log_message(ec, "multipredict");
      _multipredict_f(*_base_learner, ec, count, increment, pred, finalize_predictions);
      details::decrement_offset(ec, increment, lo);
    }
  }

  inline void update(polymorphic_ex ec, size_t i = 0)
  {
    assert(is_multiline() == ec.is_multiline());
    details::increment_offset(ec, increment, i);
    debug_log_message(ec, "update");
    _update_f(*_base_learner, ec);
    details::decrement_offset(ec, increment, i);
  }

  inline float sensitivity(example& ec, size_t i = 0)
  {
    details::increment_offset(ec, increment, i);
    debug_log_message(ec, "sensitivity");
    const float ret = _sensitivity_f(*_base_learner, ec);
    details::decrement_offset(ec, increment, i);
    return ret;
  }

  // called anytime saving or loading needs to happen. Autorecursive.
  inline void save_load(io_buf& io, const bool read, const bool text)
  {
    if (_save_load_f)
    {
      try
      {
        _save_load_f(io, read, text);
      }
      catch (VW::vw_exception& vwex)
      {
        std::stringstream better_msg;
        better_msg << "model " << std::string(read ? "load" : "save") << " failed. Error Details: " << vwex.what();
        throw VW::save_load_model_exception(vwex.filename(), vwex.line_number(), better_msg.str());
      }
    }
    if (_base_learner) { _base_learner->save_load(io, read, text); }
  }

  // called to edit the command-line from a reduction. Autorecursive
  inline void pre_save_load(VW::workspace& all)
  {
    if (_pre_save_load_f)
    {
      _pre_save_load_f(all);
    }
    if (_base_learner) { _base_learner->pre_save_load(all); }
  }

  // called when metrics is enabled.  Autorecursive.
  void persist_metrics(metric_sink& metrics)
  {
    if (_persist_metrics_f) { _persist_metrics_f(metrics); }
    if (_base_learner) { _base_learner->persist_metrics(metrics); }
  }

  // Autorecursive
  inline void finish()
  {
    // TODO: ensure that finish does not actually manage memory but just does driver finalization.
    // Then move the call to finish from the destructor of workspace to driver_finalize
    if (_finisher_f) { _finisher_f(); }
    if (_base_learner) { _base_learner->finish(); }
  }

  // Autorecursive
  void end_pass()
  {
    if (_end_pass_f) { _end_pass_f(); }
    if (_base_learner) { _base_learner->end_pass(); }
  }

  // called after parsing of examples is complete.  Autorecursive.
  void end_examples()
  {
    if (_end_examples_f) { _end_examples_f(); }
    if (_base_learner) { _base_learner->end_examples(); }
  }

  // Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver() { if (_init_f) { _init_f(); } }

  // called after learn example for each example.  Explicitly not recursive.
  inline void finish_example(VW::workspace& all, polymorphic_ex ec)
  {
    debug_log_message(ec, "finish_example");
    // If the current learner implements finish - that takes priority.
    // Else, we call the new style functions.
    // Else, we forward to the base if a base exists.

    if (has_legacy_finish())
    {
      _finish_example_f(all, ec);
      return;
    }

    if (has_update_stats()) { update_stats(all, ec); }

    if (has_output_example_prediction()) { output_example_prediction(all, ec); }

    if (has_print_update()) { print_update(all, ec); }

    if (has_cleanup_example()) { cleanup_example(ec); }

    const auto has_at_least_one_new_style_func =
        has_update_stats() || has_output_example_prediction() || has_print_update() || has_cleanup_example();
    if (has_at_least_one_new_style_func)
    {
      if (ec.is_multiline()) {
        VW::finish_example(all, static_cast<VW::multi_ex&>(ec));
      } else {
        VW::finish_example(all, static_cast<VW::example&>(ec));
      }
      return;
    }

    // Finish example used to utilize the copy forwarding semantics.
    // Traverse until first hit to mimic this but with greater type safety.
    if (_base_learner)
    {
      if (is_multiline() != _base_learner->is_multiline())
      {
        THROW("Cannot forward finish_example call across multiline/singleline boundary.");
      }
      _base_learner->finish_example(all, ec);
    }
    else { THROW("No finish functions were registered in the stack."); }
  }

  inline void update_stats(
      const VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ec, VW::io::logger& logger)
  {
    debug_log_message(ec, "update_stats");
    if (!has_update_stats()) { THROW("fatal: learner did not register update_stats fn: " + _name); }
    _update_stats_f(all, sd, ec, logger);
  }
  inline void update_stats(VW::workspace& all, const polymorphic_ex ec)
  {
    update_stats(all, *all.sd, ec, all.logger);
  }

  inline void output_example_prediction(VW::workspace& all, const polymorphic_ex ec, VW::io::logger& logger)
  {
    debug_log_message(ec, "output_example_prediction");
    if (!has_output_example_prediction()) { THROW("fatal: learner did not register output_example fn: " + _name); }
    _output_example_prediction_f(all, ec, logger);
  }
  inline void output_example_prediction(VW::workspace& all, const polymorphic_ex ec)
  {
    output_example_prediction(all, ec, all.logger);
  }

  inline void print_update(
      VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ec, VW::io::logger& logger)
  {
    debug_log_message(ec, "print_update");
    if (!has_print_update()) { THROW("fatal: learner did not register print_update fn: " + _name); }
    _print_update_f(all, sd, ec, logger);
  }
  inline void print_update(VW::workspace& all, const polymorphic_ex ec)
  {
    print_update(all, *all.sd, ec, all.logger);
  }

  inline void cleanup_example(polymorphic_ex ec)
  {
    debug_log_message(ec, "cleanup_example");
    if (!has_cleanup_example()) { THROW("fatal: learner did not register cleanup_example fn: " + _name); }
    _cleanup_example_f(ec);
  }

  void get_enabled_reductions(std::vector<std::string>& enabled_reductions) const
  {
    if (_base_learner) { _base_learner->get_enabled_reductions(enabled_reductions); }
    enabled_reductions.push_back(_name);
  }

  learner* get_learner_by_name_prefix(const std::string& reduction_name)
  {
    if (_name.find(reduction_name) != std::string::npos) { return this; }
    else
    {
      if (_base_learner) { return _base_learner->get_learner_by_name_prefix(reduction_name); }
      else { THROW("fatal: could not find in learner chain: " << reduction_name); }
    }
  }

  // This is effectively static implementing a trait for this learner type.
  // NOT auto recursive
  void merge(const std::vector<float>& per_model_weighting,
      const std::vector<const VW::workspace*>& all_workspaces, const std::vector<const learner*>& all_learners,
      VW::workspace& output_workspace, learner& output_learner)
  {
    assert(per_model_weighting.size() == all_workspaces.size());
    assert(per_model_weighting.size() == all_learners.size());

#ifndef NDEBUG
    // All learners should refer to the same learner 'type'
    assert(!all_learners.empty());
    const auto& name = all_learners[0]->get_name();
    for (const auto& learner : all_learners) { assert(learner->get_name() == name); }
#endif

    std::vector<const void*> all_data;
    all_data.reserve(all_learners.size());
    for (const auto& learner : all_learners) { all_data.push_back(learner->_learner_data.get()); }

    if (_merge_with_all_f)
    {
      _merge_with_all_f(
          per_model_weighting, all_workspaces, all_data, output_workspace, output_learner._learner_data.get());
    }
    else if (_merge_f) { _merge_f(per_model_weighting, all_data, output_learner._learner_data.get()); }
    else { THROW("learner " << _name << " does not support merging."); }
  }

  void add(const VW::workspace& base_ws, const VW::workspace& delta_ws,
      const learner* base_l, const learner* delta_l, VW::workspace& output_ws, learner* output_l)
  {
    auto name = output_l->get_name();
    assert(name == base_l->get_name());
    assert(name == delta_l->get_name());
    if (_add_with_all_f)
    {
      _add_with_all_f(base_ws, base_l->_learner_data.get(), delta_ws, delta_l->_learner_data.get(), output_ws,
          output_l->_learner_data.get());
    }
    else if (_add_f)
    {
      _add_f(base_l->_learner_data.get(), delta_l->_learner_data.get(), output_l->_learner_data.get());
    }
    else { THROW("learner " << name << " does not support adding a delta."); }
  }

  void subtract(const VW::workspace& ws1, const VW::workspace& ws2, const learner* l1,
      const learner* l2, VW::workspace& output_ws, learner* output_l)
  {
    auto name = output_l->get_name();
    assert(name == l1->get_name());
    assert(name == l2->get_name());
    if (_subtract_with_all_f)
    {
      _subtract_with_all_f(
          ws1, l1->_learner_data.get(), ws2, l2->_learner_data.get(), output_ws, output_l->_learner_data.get());
    }
    else if (_subtract_f)
    {
      _subtract_f(l1->_learner_data.get(), l2->_learner_data.get(), output_l->_learner_data.get());
    }
    else { THROW("learner " << name << " does not support subtraction to generate a delta."); }
  }

  VW_ATTR(nodiscard) bool has_legacy_finish() const { return _finish_example_f != nullptr; }
  VW_ATTR(nodiscard) bool has_update_stats() const { return _update_stats_f != nullptr; }
  VW_ATTR(nodiscard) bool has_print_update() const { return _print_update_f != nullptr; }
  VW_ATTR(nodiscard) bool has_output_example_prediction() const
  {
    return _output_example_prediction_f != nullptr;
  }
  VW_ATTR(nodiscard) bool has_cleanup_example() const { return _cleanup_example_f != nullptr; }
  VW_ATTR(nodiscard) bool has_merge() const { return (_merge_with_all_f != nullptr) || (_merge_f != nullptr); }
  VW_ATTR(nodiscard) bool has_add() const { return (_add_with_all_f != nullptr) || (_add_f != nullptr); }
  VW_ATTR(nodiscard) bool has_subtract() const
  {
    return (_subtract_with_all_f != nullptr) || (_subtract_f != nullptr);
  }
  VW_ATTR(nodiscard) prediction_type_t get_output_prediction_type() const { return _output_pred_type; }
  VW_ATTR(nodiscard) prediction_type_t get_input_prediction_type() const { return _input_pred_type; }
  VW_ATTR(nodiscard) label_type_t get_output_label_type() const { return _output_label_type; }
  VW_ATTR(nodiscard) label_type_t get_input_label_type() const { return _input_label_type; }
  VW_ATTR(nodiscard) bool is_multiline() const { return _is_multiline; }
  VW_ATTR(nodiscard) const std::string& get_name() const { return _name; }
  VW_ATTR(nodiscard) const learner* get_learn_base() const { return _base_learner.get(); }
  VW_ATTR(nodiscard) learner* get_learn_base() { return _base_learner.get(); }
  /// If true, this specific learner defines a save load function. If false, it simply forwards to a base
  /// implementation.
  VW_ATTR(nodiscard) bool learner_defines_own_save_load() { return _save_load_f != nullptr; }

private:
  template <class FluentBuilderT, class DataT, class ExampleT, class BaseLearnerT>
  friend class common_learner_builder;
  template <class DataT, class ExampleT>
  friend class base_learner_builder;
  template <class DataT, class ExampleT, class BaseLearnerT>
  friend class reduction_learner_builder;
  template <class ExampleT, class BaseLearnerT>
  friend class reduction_no_data_learner_builder;

  details::void_func _init_f;
  details::base_example_func _learn_f;
  details::base_example_func _predict_f;
  details::base_example_func _update_f;
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

  std::string _name;  // Name of the reduction.  Used in VW_DBG to trace nested learn() and predict() calls
  prediction_type_t _output_pred_type;
  prediction_type_t _input_pred_type;
  label_type_t _output_label_type;
  label_type_t _input_label_type;
  bool _is_multiline;  // Is this a single-line or multi-line reduction?

  // There should only only ever be either none, or one of these two set. Never both.
  details::merge_with_all_func _merge_with_all_f;
  details::merge_func _merge_f;
  details::add_subtract_func _add_f;
  details::add_subtract_with_all_func _add_with_all_f;
  details::add_subtract_func _subtract_f;
  details::add_subtract_with_all_func _subtract_with_all_f;

  std::shared_ptr<void> _learner_data;
  std::shared_ptr<learner> _base_learner;

  learner() = default;  // Should only be able to construct a learner through make_reduction_learner / make_base_learner
};

inline learner* as_multiline(learner* l)
{
  if (l->is_multiline()) { return l; }
  auto message = fmt::format("Tried to use a singleline reduction as a multiline reduction Name: {}", l->get_name());
  THROW(message);
}

inline learner* as_singleline(learner* l)
{
  if (!l->is_multiline()) { return l; }
  auto message = fmt::format("Tried to use a multiline reduction as a singleline reduction. Name: {}", l->get_name());
  THROW(message);
}

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
template <typename DataT>
float recur_sensitivity(DataT&, learner& base, example& ec)
{
  return base.sensitivity(ec);
}
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_CAST_FUNC_TYPE
template <class FluentBuilderT, class DataT, class ExampleT, class BaseLearnerT>
class common_learner_builder
{
public:
  // The learner being created by this builder
  std::unique_ptr<learner> learner_ptr = nullptr;
  // Pointer to learner's data before type erasure, used for binding functions
  std::shared_ptr<DataT> learner_data = nullptr;

  common_learner_builder(
      std::unique_ptr<learner>&& input_learner, std::unique_ptr<DataT>&& data, const std::string& name)
  {
    learner_data = std::move(data);
    learner_ptr = std::move(input_learner);
    learner_ptr->_name = name;
    learner_ptr->_is_multiline = std::is_same<multi_ex, ExampleT>::value;
    learner_ptr->_learner_data = learner_data;
  }

  common_learner_builder(std::unique_ptr<DataT>&& data, const std::string& name)
      : common_learner_builder(std::unique_ptr<learner>(new learner()), std::move(data), name)
  {
  }

  // delete copy constructors
  common_learner_builder(const common_learner_builder&) = delete;
  common_learner_builder& operator=(const common_learner_builder&) = delete;

  // default move constructors
  common_learner_builder(common_learner_builder&&) noexcept = default;
  common_learner_builder& operator=(common_learner_builder&&) noexcept = default;

  FluentBuilderT& set_predict(void (*fn_ptr)(DataT&, BaseLearnerT&, ExampleT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_predict_f = [fn_ptr, data](learner& base, polymorphic_ex ex) { fn_ptr(*data, base, ex); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_predict(void (*fn_ptr)(DataT&, BaseLearnerT&, ExampleT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_predict_f = [fn_ptr, data](learner& base, polymorphic_ex ex) { fn_ptr(*data, base, ex); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_learn(void (*fn_ptr)(DataT&, BaseLearnerT&, ExampleT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_learn_f = [fn_ptr, data](learner& base, polymorphic_ex ex) { fn_ptr(*data, base, ex); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_learn(void (*fn_ptr)(DataT&, BaseLearnerT&, ExampleT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_learn_f = [fn_ptr, data](learner& base, polymorphic_ex ex) { fn_ptr(*data, base, ex); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_multipredict(
      void (*fn_ptr)(DataT&, BaseLearnerT&, ExampleT&, size_t, size_t, polyprediction*, bool)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_multipredict_f = [fn_ptr, data](learner& base, polymorphic_ex ex, size_t count, size_t step, polyprediction* pred, bool finalize_predictions) { fn_ptr(*data, base, ex, count, step, pred, finalize_predictions); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_multipredict(
      void (*fn_ptr)(DataT&, BaseLearnerT&, ExampleT&, size_t, size_t, polyprediction*, bool)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_multipredict_f = [fn_ptr, data](learner& base, polymorphic_ex ex, size_t count, size_t step, polyprediction* pred, bool finalize_predictions) { fn_ptr(*data, base, ex, count, step, pred, finalize_predictions); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_update(void (*fn_ptr)(DataT& data, BaseLearnerT& base, ExampleT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_update_f = [fn_ptr, data](learner& base, polymorphic_ex ex) { fn_ptr(*data, base, ex); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_update(void (*fn_ptr)(DataT& data, BaseLearnerT& base, ExampleT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_update_f = [fn_ptr, data](learner& base, polymorphic_ex ex) { fn_ptr(*data, base, ex); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // used for active learning and confidence to determine how easily predictions are changed
  FluentBuilderT& set_sensitivity(float (*fn_ptr)(DataT& data, learner& base, example&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_sensitivity_f = [fn_ptr, data](learner& base, example& ex) { return fn_ptr(*data, base, ex); };
    return *static_cast<FluentBuilderT*>(this);
  }

  // used for active learning and confidence to determine how easily predictions are changed
  FluentBuilderT&& set_sensitivity(float (*fn_ptr)(DataT& data, learner& base, example&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_sensitivity_f = [fn_ptr, data](learner& base, example& ex) { return fn_ptr(*data, base, ex); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_learn_returns_prediction(bool learn_returns_prediction) &
  {
    learner_ptr->learn_returns_prediction = learn_returns_prediction;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_learn_returns_prediction(bool learn_returns_prediction) &&
  {
    learner_ptr->learn_returns_prediction = learn_returns_prediction;
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_save_load(void (*fn_ptr)(DataT&, io_buf&, bool, bool)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_save_load_f = [fn_ptr, data](io_buf& buf, bool read, bool text) { fn_ptr(*data, buf, read, text); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_save_load(void (*fn_ptr)(DataT&, io_buf&, bool, bool)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_save_load_f = [fn_ptr, data](io_buf& buf, bool read, bool text) { fn_ptr(*data, buf, read, text); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_finish(void (*fn_ptr)(DataT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_finisher_f = [fn_ptr, data]() { fn_ptr(*data); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_finish(void (*fn_ptr)(DataT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_finisher_f = [fn_ptr, data]() { fn_ptr(*data); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_end_pass(void (*fn_ptr)(DataT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_end_pass_f = [fn_ptr, data]() { fn_ptr(*data); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_end_pass(void (*fn_ptr)(DataT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_end_pass_f = [fn_ptr, data]() { fn_ptr(*data); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_end_examples(void (*fn_ptr)(DataT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_end_examples_f = [fn_ptr, data]() { fn_ptr(*data); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_end_examples(void (*fn_ptr)(DataT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_end_examples_f = [fn_ptr, data]() { fn_ptr(*data); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_init_driver(void (*fn_ptr)(DataT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_init_f = [fn_ptr, data]() { fn_ptr(*data); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_init_driver(void (*fn_ptr)(DataT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_init_f = [fn_ptr, data]() { fn_ptr(*data); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_finish_example(void (*fn_ptr)(VW::workspace& all, DataT&, ExampleT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_finish_example_f = [fn_ptr, data](VW::workspace& all, polymorphic_ex ex) { fn_ptr(all, *data, ex); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_finish_example(void (*fn_ptr)(VW::workspace& all, DataT&, ExampleT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_finish_example_f = [fn_ptr, data](VW::workspace& all, polymorphic_ex ex) { fn_ptr(all, *data, ex); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // Responsibilities of update stats:
  // - Call shared_data::update
  FluentBuilderT& set_update_stats(learner_update_stats_func<DataT, ExampleT>* fn_ptr) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_update_stats_f = [fn_ptr, data](const VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger) { fn_ptr(all, sd, *data, ex, logger); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_update_stats(learner_update_stats_func<DataT, ExampleT>* fn_ptr) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_update_stats_f = [fn_ptr, data](const VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger) { fn_ptr(all, sd, *data, ex, logger); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // Responsibilities of output example prediction:
  // - Output predictions
  FluentBuilderT& set_output_example_prediction(learner_output_example_prediction_func<DataT, ExampleT>* fn_ptr) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_output_example_prediction_f = [fn_ptr, data](VW::workspace& all, const polymorphic_ex ex, VW::io::logger& logger) { fn_ptr(all, *data, ex, logger); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_output_example_prediction(learner_output_example_prediction_func<DataT, ExampleT>* fn_ptr) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_output_example_prediction_f = [fn_ptr, data](VW::workspace& all, const polymorphic_ex ex, VW::io::logger& logger) { fn_ptr(all, *data, ex, logger); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // Responsibilities of output example prediction:
  // - Call shared_data::print_update
  // Note this is only called when required based on the user specified backoff and logging settings.
  FluentBuilderT& set_print_update(learner_print_update_func<DataT, ExampleT>* fn_ptr) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_print_update_f = [fn_ptr, data](VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger) { fn_ptr(all, sd, *data, ex, logger); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_print_update(learner_print_update_func<DataT, ExampleT>* fn_ptr) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_print_update_f = [fn_ptr, data](VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ex, VW::io::logger& logger) { fn_ptr(all, sd, *data, ex, logger); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // This call is **optional**, correctness cannot depend on it.
  // However, it can be used to optimistically reuse memory.
  FluentBuilderT& set_cleanup_example(learner_cleanup_example_func<DataT, ExampleT>* fn_ptr) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_cleanup_example_f = [fn_ptr, data](polymorphic_ex ex) { fn_ptr(*data, ex); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_cleanup_example(learner_cleanup_example_func<DataT, ExampleT>* fn_ptr) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_cleanup_example_f = [fn_ptr, data](polymorphic_ex ex) { fn_ptr(*data, ex); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_persist_metrics(void (*fn_ptr)(DataT&, metric_sink&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_persist_metrics_f = [fn_ptr, data](metric_sink& metrics) { fn_ptr(*data, metrics); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_persist_metrics(void (*fn_ptr)(DataT&, metric_sink&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_persist_metrics_f = [fn_ptr, data](metric_sink& metrics) { fn_ptr(*data, metrics); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  FluentBuilderT& set_pre_save_load(void (*fn_ptr)(VW::workspace& all, DataT&)) &
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_pre_save_load_f = [fn_ptr, data](VW::workspace& all) { fn_ptr(all, *data); };
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_pre_save_load(void (*fn_ptr)(VW::workspace& all, DataT&)) &&
  {
    DataT* data = this->learner_data.get();
    this->learner_ptr->_pre_save_load_f = [fn_ptr, data](VW::workspace& all) { fn_ptr(all, *data); };
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // This is the label type of the example passed into the learn function. This
  // label will be operated on throughout the learn function.
  FluentBuilderT& set_input_label_type(label_type_t label_type) &
  {
    this->learner_ptr->_input_label_type = label_type;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_input_label_type(label_type_t label_type) &&
  {
    this->learner_ptr->_input_label_type = label_type;
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // This is the label type of the example fed into the base in the learn function.
  // This will reference the state of the label after it has been operated on throughout
  // the learn function.
  FluentBuilderT& set_output_label_type(label_type_t label_type) &
  {
    this->learner_ptr->_output_label_type = label_type;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_output_label_type(label_type_t label_type) &&
  {
    this->learner_ptr->_output_label_type = label_type;
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // This is the prediction type received when calling predict from the base at
  // the top of the predict function. Note that the prediction from the example
  // passed directly into the predict function has no defined type.
  FluentBuilderT& set_input_prediction_type(prediction_type_t pred_type) &
  {
    this->learner_ptr->_input_pred_type = pred_type;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_input_prediction_type(prediction_type_t pred_type) &&
  {
    this->learner_ptr->_input_pred_type = pred_type;
    return std::move(*static_cast<FluentBuilderT*>(this));
  }

  // This is the prediction type of the example at the end of the predict function.
  // This prediction will be passed when the reduction above it calls predict on its base.
  FluentBuilderT& set_output_prediction_type(prediction_type_t pred_type) &
  {
    this->learner_ptr->_output_pred_type = pred_type;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT&& set_output_prediction_type(prediction_type_t pred_type) &&
  {
    this->learner_ptr->_output_pred_type = pred_type;
    return std::move(*static_cast<FluentBuilderT*>(this));
  }
};

template <class DataT, class ExampleT, class BaseLearnerT>
class reduction_learner_builder
    : public common_learner_builder<reduction_learner_builder<DataT, ExampleT, BaseLearnerT>, DataT, ExampleT,
          BaseLearnerT>
{
public:
  using super =
      common_learner_builder<reduction_learner_builder<DataT, ExampleT, BaseLearnerT>, DataT, ExampleT, BaseLearnerT>;
  reduction_learner_builder(std::unique_ptr<DataT>&& data, BaseLearnerT* base, const std::string& name)
      // NOTE: This is a copy of the base! The purpose is to copy all of the
      // function data objects so that if this reduction does not define a function such as
      // save_load then calling save_load on this object will essentially result in forwarding the
      // call the next reduction that actually implements it.
      : common_learner_builder<reduction_learner_builder<DataT, ExampleT, BaseLearnerT>, DataT, ExampleT, BaseLearnerT>(
            std::unique_ptr<learner>(new learner(*base)), std::move(data), name)
  {
    // Give ownership of base to this learner
    this->learner_ptr->_base_learner.reset(base);

    // We explicitly overwrite the copy of the base's finish_example functions.
    // This is to allow us to determine if the current reduction implements finish
    // and in what way.
    this->learner_ptr->_finish_example_f = nullptr;
    this->learner_ptr->_update_stats_f = nullptr;
    this->learner_ptr->_output_example_prediction_f = nullptr;
    this->learner_ptr->_print_update_f = nullptr;
    this->learner_ptr->_cleanup_example_f = nullptr;

    // Default sensitivity calls base's sensitivity recursively
    super::set_sensitivity(details::recur_sensitivity<DataT>);

    // Defaults here are undefined
    this->learner_ptr->_finisher_f = nullptr;
    this->learner_ptr->_multipredict_f = nullptr;

    // Don't propagate merge functions
    this->learner_ptr->_merge_f = nullptr;
    this->learner_ptr->_merge_with_all_f = nullptr;
    this->learner_ptr->_add_f = nullptr;
    this->learner_ptr->_add_with_all_f = nullptr;
    this->learner_ptr->_subtract_f = nullptr;
    this->learner_ptr->_subtract_with_all_f = nullptr;

    set_params_per_weight(1);
    this->set_learn_returns_prediction(false);

    // By default, will produce what the base expects
    super::set_input_label_type(base->get_input_label_type());
    // By default, will produce what the base expects
    super::set_output_label_type(base->get_input_label_type());
    // By default, will produce what the base produces
    super::set_input_prediction_type(base->get_output_prediction_type());
    // By default, will produce what the base produces
    super::set_output_prediction_type(base->get_output_prediction_type());
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>& set_params_per_weight(size_t params_per_weight) &
  {
    this->learner_ptr->weights = params_per_weight;
    this->learner_ptr->increment = this->learner_ptr->_base_learner->increment * this->learner_ptr->weights;
    return *this;
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>&& set_params_per_weight(size_t params_per_weight) &&
  {
    this->learner_ptr->weights = params_per_weight;
    this->learner_ptr->increment = this->learner_ptr->_base_learner->increment * this->learner_ptr->weights;
    return std::move(*this);
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>& set_merge(void (*merge_fn)(
      const std::vector<float>& per_model_weighting, const std::vector<const DataT*>& all_data, DataT& output_data)) &
  {
    this->learner_ptr->_merge_f = reinterpret_cast<details::merge_fptr>(merge_fn);
    return *this;
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>&& set_merge(void (*merge_fn)(
      const std::vector<float>& per_model_weighting, const std::vector<const DataT*>& all_data, DataT& output_data)) &&
  {
    this->learner_ptr->_merge_f = reinterpret_cast<details::merge_fptr>(merge_fn);
    return std::move(*this);
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>& set_add(
      void (*add_fn)(const DataT& data1, const DataT& data2, DataT& data_out)) &
  {
    this->learner_ptr->_add_f = reinterpret_cast<details::add_subtract_fptr>(add_fn);
    return *this;
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>&& set_add(
      void (*add_fn)(const DataT& data1, const DataT& data2, DataT& data_out)) &&
  {
    this->learner_ptr->_add_f = reinterpret_cast<details::add_subtract_fptr>(add_fn);
    return std::move(*this);
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>& set_subtract(
      void (*subtract_fn)(const DataT& data1, const DataT& data2, DataT& data_out)) &
  {
    this->learner_ptr->_subtract_f = reinterpret_cast<details::add_subtract_fptr>(subtract_fn);
    return *this;
  }

  reduction_learner_builder<DataT, ExampleT, BaseLearnerT>&& set_subtract(
      void (*subtract_fn)(const DataT& data1, const DataT& data2, DataT& data_out)) &&
  {
    this->learner_ptr->_subtract_f = reinterpret_cast<details::add_subtract_fptr>(subtract_fn);
    return std::move(*this);
  }

  learner* build()
  {
    prediction_type_t in_pred_type = this->learner_ptr->get_input_prediction_type();
    prediction_type_t base_out_pred_type = this->learner_ptr->get_learn_base()->get_output_prediction_type();
    label_type_t out_label_type = this->learner_ptr->get_output_label_type();
    label_type_t base_in_label_type = this->learner_ptr->get_learn_base()->get_input_label_type();
    details::learner_build_diagnostic(this->learner_ptr->get_name(), this->learner_ptr->get_learn_base()->get_name(),
        in_pred_type, base_out_pred_type, out_label_type, base_in_label_type, this->learner_ptr->_merge_f,
        this->learner_ptr->_merge_with_all_f);

    return this->learner_ptr.release();
  }
};

template <class ExampleT, class BaseLearnerT>
class reduction_no_data_learner_builder
    : public common_learner_builder<reduction_learner_builder<char, ExampleT, BaseLearnerT>, char, ExampleT,
          BaseLearnerT>
{
public:
  using super =
      common_learner_builder<reduction_learner_builder<char, ExampleT, BaseLearnerT>, char, ExampleT, BaseLearnerT>;
  reduction_no_data_learner_builder(BaseLearnerT* base, const std::string& name)
      // NOTE: This is a copy of the base! The purpose is to copy all of the
      // function data objects so that if this reduction does not define a function such as
      // save_load then calling save_load on this object will essentially result in forwarding the
      // call the next reduction that actually implements it.
      : common_learner_builder<reduction_learner_builder<char, ExampleT, BaseLearnerT>, char, ExampleT, BaseLearnerT>(
            std::unique_ptr<learner>(new learner(*base)), nullptr, name)
  {
    // Give ownership of base to this learner
    this->learner_ptr->_base_learner.reset(base);

    // We explicitly overwrite the copy of the base's finish_example functions.
    // This is to allow us to determine if the current reduction implements finish
    // and in what way.
    this->learner_ptr->_finish_example_f = nullptr;
    this->learner_ptr->_update_stats_f = nullptr;
    this->learner_ptr->_output_example_prediction_f = nullptr;
    this->learner_ptr->_print_update_f = nullptr;
    this->learner_ptr->_cleanup_example_f = nullptr;

    // Default sensitivity calls base's sensitivity recursively
    super::set_sensitivity(details::recur_sensitivity);

    // Defaults here are undefined
    this->learner_ptr->_finisher_f = nullptr;
    this->learner_ptr->_multipredict_f = nullptr;

    // Don't propagate merge functions
    this->learner_ptr->_merge_f = nullptr;
    this->learner_ptr->_merge_with_all_f = nullptr;
    this->learner_ptr->_add_f = nullptr;
    this->learner_ptr->_add_with_all_f = nullptr;
    this->learner_ptr->_subtract_f = nullptr;
    this->learner_ptr->_subtract_with_all_f = nullptr;

    set_params_per_weight(1);
    // By default, will produce what the base expects
    super::set_input_label_type(base->get_input_label_type());
    // By default, will produce what the base expects
    super::set_output_label_type(base->get_input_label_type());
    // By default, will produce what the base produces
    super::set_input_prediction_type(base->get_output_prediction_type());
    // By default, will produce what the base produces
    super::set_output_prediction_type(base->get_output_prediction_type());
  }

  reduction_no_data_learner_builder<ExampleT, BaseLearnerT>& set_params_per_weight(size_t params_per_weight) &
  {
    this->learner_ptr->weights = params_per_weight;
    this->learner_ptr->increment = this->learner_ptr->_base_learner->increment * this->learner_ptr->weights;
    return *this;
  }

  reduction_no_data_learner_builder<ExampleT, BaseLearnerT>&& set_params_per_weight(size_t params_per_weight) &&
  {
    this->learner_ptr->weights = params_per_weight;
    this->learner_ptr->increment = this->learner_ptr->_base_learner->increment * this->learner_ptr->weights;
    return std::move(*this);
  }

  learner* build() { return this->learner_ptr.release(); }
};

template <class DataT, class ExampleT>
class base_learner_builder
    : public common_learner_builder<base_learner_builder<DataT, ExampleT>, DataT, ExampleT, learner>
{
public:
  using super = common_learner_builder<base_learner_builder<DataT, ExampleT>, DataT, ExampleT, learner>;
  base_learner_builder(std::unique_ptr<DataT>&& data, const std::string& name, prediction_type_t out_pred_type,
      label_type_t in_label_type)
      : common_learner_builder<base_learner_builder<DataT, ExampleT>, DataT, ExampleT, learner>(
            std::move(data), name)
  {
    this->learner_ptr->_persist_metrics_f = nullptr;
    this->learner_ptr->_end_pass_f = nullptr;
    this->learner_ptr->_end_examples_f = nullptr;
    this->learner_ptr->_init_f = nullptr;
    this->learner_ptr->_save_load_f = nullptr;
    this->learner_ptr->_finisher_f = nullptr;
    this->learner_ptr->_sensitivity_f = [](learner&, example&) { return 0.f; };

    super::set_input_label_type(in_label_type);
    super::set_output_label_type(label_type_t::NOLABEL);
    super::set_input_prediction_type(prediction_type_t::NOPRED);
    super::set_output_prediction_type(out_pred_type);

    set_params_per_weight(1);
  }

  base_learner_builder<DataT, ExampleT>& set_params_per_weight(size_t params_per_weight) &
  {
    this->learner_ptr->weights = 1;
    this->learner_ptr->increment = params_per_weight;
    return *this;
  }

  base_learner_builder<DataT, ExampleT>&& set_params_per_weight(size_t params_per_weight) &&
  {
    this->learner_ptr->weights = 1;
    this->learner_ptr->increment = params_per_weight;
    return std::move(*this);
  }

  base_learner_builder<DataT, ExampleT>& set_merge_with_all(void (*merge_with_all_fn)(
      const std::vector<float>& per_model_weighting, const std::vector<const VW::workspace*>& all_workspaces,
      const std::vector<DataT*>& all_data, VW::workspace& output_workspace, DataT& output_data)) &
  {
    this->learner_ptr->_merge_with_all_f = reinterpret_cast<details::merge_with_all_fptr>(merge_with_all_fn);
    return *this;
  }

  base_learner_builder<DataT, ExampleT>&& set_merge_with_all(void (*merge_with_all_fn)(
      const std::vector<float>& per_model_weighting, const std::vector<const VW::workspace*>& all_workspaces,
      const std::vector<DataT*>& all_data, VW::workspace& output_workspace, DataT& output_data)) &&
  {
    this->learner_ptr->_merge_with_all_f = reinterpret_cast<details::merge_with_all_fptr>(merge_with_all_fn);
    return std::move(*this);
  }

  base_learner_builder<DataT, ExampleT>& set_add_with_all(void (*add_with_all_fn)(const VW::workspace& ws1,
      const DataT& data1, const VW::workspace& ws2, DataT& data2, VW::workspace& ws_out, DataT& data_out)) &
  {
    this->learner_ptr->_add_with_all_f = reinterpret_cast<details::add_subtract_with_all_fptr>(add_with_all_fn);
    return *this;
  }
  base_learner_builder<DataT, ExampleT>&& set_add_with_all(void (*add_with_all_fn)(const VW::workspace& ws1,
      const DataT& data1, const VW::workspace& ws2, DataT& data2, VW::workspace& ws_out, DataT& data_out)) &&
  {
    this->learner_ptr->_add_with_all_f = reinterpret_cast<details::add_subtract_with_all_fptr>(add_with_all_fn);
    return std::move(*this);
  }

  base_learner_builder<DataT, ExampleT>& set_subtract_with_all(void (*subtract_with_all_fn)(const VW::workspace& ws1,
      const DataT& data1, const VW::workspace& ws2, DataT& data2, VW::workspace& ws_out, DataT& data_out)) &
  {
    this->learner_ptr->_subtract_with_all_f =
        reinterpret_cast<details::add_subtract_with_all_fptr>(subtract_with_all_fn);
    return *this;
  }

  base_learner_builder<DataT, ExampleT>&& set_subtract_with_all(void (*subtract_with_all_fn)(const VW::workspace& ws1,
      const DataT& data1, const VW::workspace& ws2, DataT& data2, VW::workspace& ws_out, DataT& data_out)) &&
  {
    this->learner_ptr->_subtract_with_all_f =
        reinterpret_cast<details::add_subtract_with_all_fptr>(subtract_with_all_fn);
    return std::move(*this);
  }

  learner* build()
  {
    if (this->learner_ptr->_merge_f && this->learner_ptr->_merge_with_all_f)
    {
      THROW("cannot set both merge and merge_with_all");
    }
    return this->learner_ptr.release();
  }
};
VW_WARNING_STATE_POP

template <class DataT, class ExampleT, class BaseLearnerT>
reduction_learner_builder<DataT, ExampleT, BaseLearnerT> make_reduction_learner(std::unique_ptr<DataT>&& data,
    BaseLearnerT* base, void (*learn_fn)(DataT&, BaseLearnerT&, ExampleT&),
    void (*predict_fn)(DataT&, BaseLearnerT&, ExampleT&), const std::string& name)
{
  auto builder = reduction_learner_builder<DataT, ExampleT, BaseLearnerT>(std::move(data), base, name);
  builder.set_learn(learn_fn);
  builder.set_update(learn_fn);
  builder.set_predict(predict_fn);
  return builder;
}

template <class ExampleT, class BaseLearnerT>
reduction_no_data_learner_builder<ExampleT, BaseLearnerT> make_no_data_reduction_learner(BaseLearnerT* base,
    void (*learn_fn)(char&, BaseLearnerT&, ExampleT&), void (*predict_fn)(char&, BaseLearnerT&, ExampleT&),
    const std::string& name)
{
  auto builder = reduction_no_data_learner_builder<ExampleT, BaseLearnerT>(base, name);
  builder.set_learn(learn_fn);
  builder.set_update(learn_fn);
  builder.set_predict(predict_fn);
  return builder;
}

template <class DataT, class ExampleT>
base_learner_builder<DataT, ExampleT> make_base_learner(std::unique_ptr<DataT>&& data,
    void (*learn_fn)(DataT&, learner&, ExampleT&), void (*predict_fn)(DataT&, learner&, ExampleT&),
    const std::string& name, prediction_type_t out_pred_type, label_type_t in_label_type)
{
  auto builder = base_learner_builder<DataT, ExampleT>(std::move(data), name, out_pred_type, in_label_type);
  builder.set_learn(learn_fn);
  builder.set_update(learn_fn);
  builder.set_predict(predict_fn);
  return builder;
}

template <class ExampleT>
base_learner_builder<char, ExampleT> make_no_data_base_learner(void (*learn_fn)(char&, learner&, ExampleT&),
    void (*predict_fn)(char&, learner&, ExampleT&), const std::string& name, prediction_type_t out_pred_type,
    label_type_t in_label_type)
{
  return make_base_learner<char, ExampleT>(
      std::unique_ptr<char>(nullptr), learn_fn, predict_fn, name, out_pred_type, in_label_type);
}

}  // namespace LEARNER
}  // namespace VW