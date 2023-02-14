// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/learner.h"

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

#include "vw/core/parse_dispatch_loop.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"

namespace VW
{
namespace LEARNER
{
void learn_ex(example& ec, VW::workspace& all)
{
  all.learn(ec);
  require_singleline(all.l)->finish_example(all, ec);
}

void learn_multi_ex(multi_ex& ec_seq, VW::workspace& all)
{
  all.learn(ec_seq);
  require_multiline(all.l)->finish_example(all, ec_seq);
}

void end_pass(example& ec, VW::workspace& all)
{
  all.current_pass++;
  all.l->end_pass();

  VW::finish_example(all, ec);
}

void save(example& ec, VW::workspace& all)
{
  // save state command
  std::string final_regressor_name = all.final_regressor_name;

  if ((ec.tag).size() >= 6 && (ec.tag)[4] == '_')
  {
    final_regressor_name = std::string(ec.tag.begin() + 5, (ec.tag).size() - 5);
  }

  if (!all.quiet) { *(all.trace_message) << "saving regressor to " << final_regressor_name << std::endl; }
  VW::details::save_predictor(all, final_regressor_name, 0);

  VW::finish_example(all, ec);
}

/* is this just a newline */
inline bool example_is_newline_not_header(example& ec, VW::workspace& all)
{
  // If we are using CCB, test against CCB implementation otherwise fallback to previous behavior.
  const bool is_header = ec_is_example_header(ec, all.example_parser->lbl_parser.label_type);
  return example_is_newline(ec) && !is_header;
}

bool inline is_save_cmd(example* ec)
{
  return (ec->tag.size() >= 4) && (0 == strncmp((const char*)ec->tag.begin(), "save", 4));
}

void drain_examples(VW::workspace& all)
{
  if (all.early_terminate)
  {  // drain any extra examples from parser.
    example* ec = nullptr;
    while ((ec = VW::get_example(all.example_parser.get())) != nullptr) { VW::finish_example(all, *ec); }
  }
  all.l->end_examples();
}

// single_instance_context / multi_instance_context - classes incapsulating single/multiinstance example processing
// get_master - returns main vw instance for owner's example manipulations (i.e. finish)
// process<process_impl> - call process_impl for all vw instances
class single_instance_context
{
public:
  single_instance_context(VW::workspace& all) : _all(all) {}

  VW::workspace& get_master() const { return _all; }

  template <class T, void (*process_impl)(T&, VW::workspace&)>
  void process(T& ec)
  {
    process_impl(ec, _all);
  }

private:
  VW::workspace& _all;
};

class multi_instance_context
{
public:
  multi_instance_context(const std::vector<VW::workspace*>& all) : _all(all) {}

  VW::workspace& get_master() const { return *_all.front(); }

  template <class T, void (*process_impl)(T&, VW::workspace&)>
  void process(T& ec)
  {
    // start with last as the first instance will free the example as it is the owner
    for (auto it = _all.rbegin(); it != _all.rend(); ++it) { process_impl(ec, **it); }
  }

private:
  std::vector<VW::workspace*> _all;
};

// single_example_handler / multi_example_handler - consumer classes with on_example handle method, incapsulating
// creation of example / multi_ex and passing it to context.process
template <typename context_type>
class single_example_handler
{
public:
  single_example_handler(const context_type& context) : _context(context) {}

  void on_example(example* ec)
  {
    if (ec->indices.size() > 1)
    {  // 1+ nonconstant feature. (most common case first)
      _context.template process<example, learn_ex>(*ec);
    }
    else if (ec->end_pass) { _context.template process<example, end_pass>(*ec); }
    else if (is_save_cmd(ec)) { _context.template process<example, save>(*ec); }
    else { _context.template process<example, learn_ex>(*ec); }
  }

  void process_remaining() {}

private:
  context_type _context;
};

template <typename context_type>
class multi_example_handler
{
public:
  multi_example_handler(const context_type context) : _context(context) {}
  ~multi_example_handler() = default;

  void on_example(example* ec)
  {
    if (try_complete_multi_ex(ec))
    {
      _context.template process<multi_ex, learn_multi_ex>(_ec_seq);
      _ec_seq.clear();
    }
    // after we learn, cleanup is_newline or end_pass example
    if (ec->end_pass)
    {
      // Because the end_pass example is used to complete the in-flight multi_ex prior
      // to this call we should have no more in-flight multi_ex here.
      assert(_ec_seq.empty());
      _context.template process<example, end_pass>(*ec);
    }
    else if (ec->is_newline)
    {
      // Because the is_newline example is used to complete the in-flight multi_ex prior
      // to this call we should have no more in-flight multi_ex here.
      assert(_ec_seq.empty());
      VW::finish_example(_context.get_master(), *ec);
    }
  }

  void process_remaining()
  {
    if (!_ec_seq.empty())
    {
      _context.template process<multi_ex, learn_multi_ex>(_ec_seq);
      _ec_seq.clear();
    }
  }

private:
  bool complete_multi_ex(example* ec)
  {
    auto& master = _context.get_master();
    const bool is_test_ec = master.example_parser->lbl_parser.test_label(ec->l);
    const bool is_newline = (example_is_newline_not_header(*ec, master) && is_test_ec);

    if (!is_newline && !ec->end_pass) { _ec_seq.push_back(ec); }
    // A terminating example can occur when there have been no featureful examples
    // collected. In this case, do not trigger a learn.
    return (is_newline || ec->end_pass) && !_ec_seq.empty();
  }

  bool try_complete_multi_ex(example* ec)
  {
    if (ec->indices.size() > 1)
    {  // 1+ nonconstant feature. (most common case first)
      return complete_multi_ex(ec);
      // Explicitly do not process the end-of-pass examples here: It needs to be done
      // after learning on the collected multi_ex
    }
    else if (is_save_cmd(ec)) { _context.template process<example, save>(*ec); }
    else { return complete_multi_ex(ec); }
    return false;
  }

  context_type _context;
  multi_ex _ec_seq;
};

// ready_examples_queue / custom_examples_queue - adapters for connecting example handler to parser produce-consume loop
// for single- and multi-threaded scenarios
class ready_examples_queue
{
public:
  ready_examples_queue(VW::workspace& master) : _master(master) {}

  example* pop() { return !_master.early_terminate ? VW::get_example(_master.example_parser.get()) : nullptr; }

private:
  VW::workspace& _master;
};

class custom_examples_queue
{
public:
  void reset_examples(const VW::multi_ex* examples)
  {
    assert(examples != nullptr);
    _examples = examples;
    _index = 0;
  }

  example* pop()
  {
    assert(_examples != nullptr);
    return _index < _examples->size() ? (*_examples)[_index++] : nullptr;
  }

private:
  const VW::multi_ex* _examples;
  size_t _index{0};
};

template <typename queue_type, typename handler_type>
void process_examples(queue_type& examples, handler_type& handler)
{
  example* ec;
  while ((ec = examples.pop()) != nullptr) { handler.on_example(ec); }
}

template <typename context_type>
void generic_driver(ready_examples_queue& examples, context_type& context)
{
  if (context.get_master().l->is_multiline())
  {
    using handler_type = multi_example_handler<context_type>;
    handler_type handler(context);
    process_examples(examples, handler);
    handler.process_remaining();
  }
  else
  {
    using handler_type = single_example_handler<context_type>;
    handler_type handler(context);
    process_examples(examples, handler);
    handler.process_remaining();
  }
  drain_examples(context.get_master());
}

void generic_driver(VW::workspace& all)
{
  single_instance_context context(all);
  ready_examples_queue examples(all);
  generic_driver(examples, context);
}

void generic_driver(const std::vector<VW::workspace*>& all)
{
  multi_instance_context context(all);
  ready_examples_queue examples(context.get_master());
  generic_driver(examples, context);
}

template <typename handler_type>
void generic_driver_onethread(VW::workspace& all)
{
  single_instance_context context(all);
  handler_type handler(context);
  custom_examples_queue examples_queue;
  auto multi_ex_fptr = [&handler, &examples_queue](VW::workspace& /*all*/, const VW::multi_ex& examples)
  {
    examples_queue.reset_examples(&examples);
    process_examples(examples_queue, handler);
  };
  VW::details::parse_dispatch(all, multi_ex_fptr);
  handler.process_remaining();
  all.l->end_examples();
}

void generic_driver_onethread(VW::workspace& all)
{
  if (all.l->is_multiline()) { generic_driver_onethread<multi_example_handler<single_instance_context>>(all); }
  else { generic_driver_onethread<single_example_handler<single_instance_context>>(all); }
}

bool ec_is_example_header(const example& ec, label_type_t label_type)
{
  if (label_type == VW::label_type_t::CB) { return VW::ec_is_example_header_cb(ec); }
  else if (label_type == VW::label_type_t::CCB) { return reductions::ccb::ec_is_example_header(ec); }
  else if (label_type == VW::label_type_t::CS) { return VW::is_cs_example_header(ec); }
  return false;
}

learner* require_multiline(learner* l)
{
  if (l->is_multiline()) { return l; }
  auto message = fmt::format("Tried to use a singleline learner as a multiline learner Name: {}", l->get_name());
  THROW(message);
}

learner* require_singleline(learner* l)
{
  if (!l->is_multiline()) { return l; }
  auto message = fmt::format("Tried to use a multiline learner as a singleline learner. Name: {}", l->get_name());
  THROW(message);
}

std::shared_ptr<learner> require_multiline(std::shared_ptr<learner> l)
{
  if (l->is_multiline()) { return l; }
  auto message = fmt::format("Tried to use a singleline learner as a multiline learner Name: {}", l->get_name());
  THROW(message);
}

std::shared_ptr<learner> require_singleline(std::shared_ptr<learner> l)
{
  if (!l->is_multiline()) { return l; }
  auto message = fmt::format("Tried to use a multiline learner as a singleline learner. Name: {}", l->get_name());
  THROW(message);
}

void learner::debug_log_message(polymorphic_ex ex, const std::string& msg)
{
  if (ex.is_multiline())
  {
    VW_DBG(*static_cast<VW::multi_ex&>(ex)[0]) << "[" << _name << "." << msg << "]" << std::endl;
  }
  else { VW_DBG(static_cast<VW::example&>(ex)) << "[" << _name << "." << msg << "]" << std::endl; }
}

void learner::learn(polymorphic_ex ec, size_t i)
{
  assert(is_multiline() == ec.is_multiline());
  details::increment_offset(ec, increment, i);
  debug_log_message(ec, "learn");
  _learn_f(ec);
  details::decrement_offset(ec, increment, i);
}

void learner::predict(polymorphic_ex ec, size_t i)
{
  assert(is_multiline() == ec.is_multiline());
  details::increment_offset(ec, increment, i);
  debug_log_message(ec, "predict");
  _predict_f(ec);
  details::decrement_offset(ec, increment, i);
}

void learner::multipredict(polymorphic_ex ec, size_t lo, size_t count, polyprediction* pred, bool finalize_predictions)
{
  assert(is_multiline() == ec.is_multiline());
  if (_multipredict_f == nullptr)
  {
    details::increment_offset(ec, increment, lo);
    debug_log_message(ec, "multipredict");
    for (size_t c = 0; c < count; c++)
    {
      _predict_f(ec);
      if (finalize_predictions)
      {
        pred[c] = std::move(static_cast<VW::example&>(ec).pred);  // TODO: this breaks for complex labels because =
                                                                  // doesn't do deep copy! (XXX we "fix" this by moving)
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
    _multipredict_f(ec, count, increment, pred, finalize_predictions);
    details::decrement_offset(ec, increment, lo);
  }
}

void learner::update(polymorphic_ex ec, size_t i)
{
  assert(is_multiline() == ec.is_multiline());
  details::increment_offset(ec, increment, i);
  debug_log_message(ec, "update");
  _update_f(ec);
  details::decrement_offset(ec, increment, i);
}

float learner::sensitivity(example& ec, size_t i)
{
  details::increment_offset(ec, increment, i);
  debug_log_message(ec, "sensitivity");
  const float ret = _sensitivity_f(ec);
  details::decrement_offset(ec, increment, i);
  return ret;
}

void learner::save_load(io_buf& io, const bool read, const bool text)
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

void learner::pre_save_load(VW::workspace& all)
{
  if (_pre_save_load_f) { _pre_save_load_f(all); }
  if (_base_learner) { _base_learner->pre_save_load(all); }
}

void learner::persist_metrics(metric_sink& metrics)
{
  if (_persist_metrics_f) { _persist_metrics_f(metrics); }
  if (_base_learner) { _base_learner->persist_metrics(metrics); }
}

void learner::finish()
{
  // TODO: ensure that finish does not actually manage memory but just does driver finalization.
  // Then move the call to finish from the destructor of workspace to driver_finalize
  if (_finisher_f) { _finisher_f(); }
  if (_base_learner) { _base_learner->finish(); }
}

void learner::end_pass()
{
  if (_end_pass_f) { _end_pass_f(); }
  if (_base_learner) { _base_learner->end_pass(); }
}

void learner::end_examples()
{
  if (_end_examples_f) { _end_examples_f(); }
  if (_base_learner) { _base_learner->end_examples(); }
}

void learner::init_driver()
{
  if (_init_f) { _init_f(); }
}

void learner::finish_example(VW::workspace& all, polymorphic_ex ec)
{
  debug_log_message(ec, "finish_example");
  // If the current learner implements finish - that takes priority.
  // Else, we call the new style functions.
  // Else, we forward to the base learner if it exists.

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
    if (ec.is_multiline()) { VW::finish_example(all, static_cast<VW::multi_ex&>(ec)); }
    else { VW::finish_example(all, static_cast<VW::example&>(ec)); }
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

void learner::update_stats(
    const VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ec, VW::io::logger& logger)
{
  debug_log_message(ec, "update_stats");
  if (!has_update_stats()) { THROW("fatal: learner did not register update_stats fn: " + _name); }
  _update_stats_f(all, sd, ec, logger);
}
void learner::update_stats(VW::workspace& all, const polymorphic_ex ec) { update_stats(all, *all.sd, ec, all.logger); }

void learner::output_example_prediction(VW::workspace& all, const polymorphic_ex ec, VW::io::logger& logger)
{
  debug_log_message(ec, "output_example_prediction");
  if (!has_output_example_prediction()) { THROW("fatal: learner did not register output_example fn: " + _name); }
  _output_example_prediction_f(all, ec, logger);
}
void learner::output_example_prediction(VW::workspace& all, const polymorphic_ex ec)
{
  output_example_prediction(all, ec, all.logger);
}

void learner::print_update(VW::workspace& all, VW::shared_data& sd, const polymorphic_ex ec, VW::io::logger& logger)
{
  debug_log_message(ec, "print_update");
  if (!has_print_update()) { THROW("fatal: learner did not register print_update fn: " + _name); }
  _print_update_f(all, sd, ec, logger);
}
void learner::print_update(VW::workspace& all, const polymorphic_ex ec) { print_update(all, *all.sd, ec, all.logger); }

void learner::cleanup_example(polymorphic_ex ec)
{
  debug_log_message(ec, "cleanup_example");
  if (!has_cleanup_example()) { THROW("fatal: learner did not register cleanup_example fn: " + _name); }
  _cleanup_example_f(ec);
}

void learner::get_enabled_learners(std::vector<std::string>& enabled_learners) const
{
  if (_base_learner) { _base_learner->get_enabled_learners(enabled_learners); }
  enabled_learners.push_back(_name);
}

learner* learner::get_learner_by_name_prefix(const std::string& learner_name)
{
  if (_name.find(learner_name) != std::string::npos) { return this; }
  else
  {
    if (_base_learner) { return _base_learner->get_learner_by_name_prefix(learner_name); }
    else { THROW("fatal: could not find in learner chain: " << learner_name); }
  }
}

void learner::merge(const std::vector<float>& per_model_weighting,
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

void learner::add(const VW::workspace& base_ws, const VW::workspace& delta_ws, const learner* base_l,
    const learner* delta_l, VW::workspace& output_ws, learner* output_l)
{
  auto name = output_l->get_name();
  assert(name == base_l->get_name());
  assert(name == delta_l->get_name());
  if (_add_with_all_f)
  {
    _add_with_all_f(base_ws, base_l->_learner_data.get(), delta_ws, delta_l->_learner_data.get(), output_ws,
        output_l->_learner_data.get());
  }
  else if (_add_f) { _add_f(base_l->_learner_data.get(), delta_l->_learner_data.get(), output_l->_learner_data.get()); }
  else { THROW("learner " << name << " does not support adding a delta."); }
}

void learner::subtract(const VW::workspace& ws1, const VW::workspace& ws2, const learner* l1, const learner* l2,
    VW::workspace& output_ws, learner* output_l)
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

std::shared_ptr<learner> learner::create_learner_above_this()
{
  // Copy this learner and give the new learner ownership of this learner.
  std::shared_ptr<learner> l(new learner(*this));
  l->_base_learner = shared_from_this();

  // We explicitly overwrite the copy of the base learner's finish_example functions.
  // This is to allow us to determine if the current learner implements finish and in what way.
  l->_finish_example_f = nullptr;
  l->_update_stats_f = nullptr;
  l->_output_example_prediction_f = nullptr;
  l->_print_update_f = nullptr;
  l->_cleanup_example_f = nullptr;

  // Don't propagate these functions
  l->_multipredict_f = nullptr;
  l->_save_load_f = nullptr;
  l->_pre_save_load_f = nullptr;
  l->_end_pass_f = nullptr;
  l->_end_examples_f = nullptr;
  l->_persist_metrics_f = nullptr;
  l->_finisher_f = nullptr;

  // Don't propagate any of the merge functions
  l->_merge_f = nullptr;
  l->_merge_with_all_f = nullptr;
  l->_add_f = nullptr;
  l->_add_with_all_f = nullptr;
  l->_subtract_f = nullptr;
  l->_subtract_with_all_f = nullptr;

  return l;
}

}  // namespace LEARNER
}  // namespace VW

void VW::LEARNER::details::learner_build_diagnostic(VW::string_view this_name, VW::string_view base_name,
    prediction_type_t in_pred_type, prediction_type_t base_out_pred_type, label_type_t out_label_type,
    label_type_t base_in_label_type, details::merge_func merge_f, details::merge_with_all_func merge_with_all_f)
{
  if (in_pred_type != base_out_pred_type)
  {
    const auto message = fmt::format(
        "Input prediction type: {} of learner: {} does not match output prediction type: {} of base learner: "
        "{}.",
        to_string(in_pred_type), this_name, to_string(base_out_pred_type), base_name);
    THROW(message);
  }
  if (out_label_type != base_in_label_type)
  {
    const auto message =
        fmt::format("Output label type: {} of learner: {} does not match input label type: {} of base learner: {}.",
            to_string(out_label_type), this_name, to_string(base_in_label_type), base_name);
    THROW(message);
  }

  if (merge_f && merge_with_all_f) { THROW("cannot set both merge and merge_with_all"); }
}

void VW::LEARNER::details::debug_increment_depth(polymorphic_ex ex)
{
  if (ex.is_multiline())
  {
    if (vw_dbg::TRACK_STACK)
    {
      for (auto& ec : static_cast<VW::multi_ex&>(ex)) { ++ec->debug_current_reduction_depth; }
    }
  }
  else
  {
    if (vw_dbg::TRACK_STACK) { ++static_cast<VW::example&>(ex).debug_current_reduction_depth; }
  }
}

void VW::LEARNER::details::debug_decrement_depth(polymorphic_ex ex)
{
  if (ex.is_multiline())
  {
    if (vw_dbg::TRACK_STACK)
    {
      for (auto& ec : static_cast<VW::multi_ex&>(ex)) { --ec->debug_current_reduction_depth; }
    }
  }
  else
  {
    if (vw_dbg::TRACK_STACK) { --static_cast<VW::example&>(ex).debug_current_reduction_depth; }
  }
}

void VW::LEARNER::details::increment_offset(polymorphic_ex ex, const size_t increment, const size_t i)
{
  if (ex.is_multiline())
  {
    for (auto& ec : static_cast<VW::multi_ex&>(ex)) { ec->ft_offset += static_cast<uint32_t>(increment * i); }
  }
  else { static_cast<VW::example&>(ex).ft_offset += static_cast<uint32_t>(increment * i); }
  debug_increment_depth(ex);
}

void VW::LEARNER::details::decrement_offset(polymorphic_ex ex, const size_t increment, const size_t i)
{
  if (ex.is_multiline())
  {
    for (auto ec : static_cast<VW::multi_ex&>(ex))
    {
      assert(ec->ft_offset >= increment * i);
      ec->ft_offset -= static_cast<uint32_t>(increment * i);
    }
  }
  else
  {
    assert(static_cast<VW::example&>(ex).ft_offset >= increment * i);
    static_cast<VW::example&>(ex).ft_offset -= static_cast<uint32_t>(increment * i);
  }
  debug_decrement_depth(ex);
}
