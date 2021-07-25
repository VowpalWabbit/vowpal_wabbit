// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "learner.h"
#include "parser.h"
#include "vw.h"
#include "parse_regressor.h"
#include "parse_dispatch_loop.h"

#define CASE(type) \
  case type:       \
    return #type;

const char* to_string(prediction_type_t prediction_type)
{
  switch (prediction_type)
  {
    CASE(prediction_type_t::scalar)
    CASE(prediction_type_t::scalars)
    CASE(prediction_type_t::action_scores)
    CASE(prediction_type_t::action_probs)
    CASE(prediction_type_t::multiclass)
    CASE(prediction_type_t::multilabels)
    CASE(prediction_type_t::prob)
    CASE(prediction_type_t::multiclassprobs)
    default:
      return "<unsupported>";
  }
}

namespace VW
{
namespace LEARNER
{
void learn_ex(example& ec, vw& all)
{
  if(!(all.example_parser->no_learner)) all.learn(ec);
  as_singleline(all.l)->finish_example(all, ec);
}

void learn_multi_ex(multi_ex& ec_seq, vw& all)
{
  all.learn(ec_seq);
  as_multiline(all.l)->finish_example(all, ec_seq);
}

void end_pass(example& ec, vw& all)
{
  all.current_pass++;
  all.l->end_pass();

  VW::finish_example(all, ec);
}

void save(example& ec, vw& all)
{
  // save state command
  std::string final_regressor_name = all.final_regressor_name;

  if ((ec.tag).size() >= 6 && (ec.tag)[4] == '_')
    final_regressor_name = std::string(ec.tag.begin() + 5, (ec.tag).size() - 5);

  if (!all.logger.quiet) *(all.trace_message) << "saving regressor to " << final_regressor_name << std::endl;
  save_predictor(all, final_regressor_name, 0);

  VW::finish_example(all, ec);
}

/* is this just a newline */
inline bool example_is_newline_not_header(example& ec, vw& all)
{
  // If we are using CCB, test against CCB implementation otherwise fallback to previous behavior.
  bool is_header = false;
  if (all.example_parser->lbl_parser.label_type == label_type_t::ccb) { is_header = CCB::ec_is_example_header(ec); }
  else
  {
    is_header = CB::ec_is_example_header(ec);
  }

  return example_is_newline(ec) && !is_header;
}

bool inline is_save_cmd(example* ec)
{
  return (ec->tag.size() >= 4) && (0 == strncmp((const char*)ec->tag.begin(), "save", 4));
}

void drain_examples(vw& all)
{
  if (all.early_terminate)
  {  // drain any extra examples from parser.
    std::vector<example*>* ev = nullptr;
    while ((ev = VW::get_example(all.example_parser)) != nullptr){
      for (auto ex: *ev)
        VW::finish_example(all, *ex);
      VW::finish_example_vector(all, *ev);
    }
    
  }
  all.l->end_examples();
}

// single_instance_context / multi_instance_context - classes incapsulating single/multiinstance example processing
// get_master - returns main vw instance for owner's example manipulations (i.e. finish)
// process<process_impl> - call process_impl for all vw instances
class single_instance_context
{
public:
  single_instance_context(vw& all) : _all(all) {}

  vw& get_master() const { return _all; }

  template <class T, void (*process_impl)(T&, vw&)>
  void process(T& ec)
  {
    process_impl(ec, _all);
  }

private:
  vw& _all;
};

class multi_instance_context
{
public:
  multi_instance_context(const std::vector<vw*>& all) : _all(all) {}

  vw& get_master() const { return *_all.front(); }

  template <class T, void (*process_impl)(T&, vw&)>
  void process(T& ec)
  {
    // start with last as the first instance will free the example as it is the owner
    for (auto it = _all.rbegin(); it != _all.rend(); ++it) process_impl(ec, **it);
  }

private:
  std::vector<vw*> _all;
};

// single_example_handler / multi_example_handler - consumer classes with on_example handle method, incapsulating
// creation of example / multi_ex and passing it to context.process
template <typename context_type>
class single_example_handler
{
public:
  single_example_handler(const context_type& context) : _context(context) {}

  void on_example(std::vector<example*>* ev)
  {
    // ev is guaranteed to have exactly one element always.
    example* ec = (*ev)[0];

    work_on_example(_context.get_master(), ec);
    if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
      _context.template process<example, learn_ex>(*ec);
    else if (ec->end_pass)
      _context.template process<example, end_pass>(*ec);
    else if (is_save_cmd(ec))
      _context.template process<example, save>(*ec);
    else
      _context.template process<example, learn_ex>(*ec);
    
    VW::finish_example_vector(_context.get_master(), *ev);
  }

private:
  context_type _context;
};

template <typename context_type>
class multi_example_handler
{
private:
  bool complete_multi_ex(example* ec)
  {
    auto& master = _context.get_master();
    const bool is_test_ec = master.example_parser->lbl_parser.test_label(&ec->l);
    const bool is_newline = (example_is_newline_not_header(*ec, master) && is_test_ec);
    if (is_newline)
    {
      VW::finish_example(master, *ec);
    }
    return is_newline;
  }

  bool try_complete_multi_ex(std::vector<example*> ev)
  {
    example* ec = ev.back();
    if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
      return complete_multi_ex(ec);
    else if (ec->end_pass)
      _context.template process<example, end_pass>(*ec);
    else if (is_save_cmd(ec))
      _context.template process<example, save>(*ec);
    else
      return complete_multi_ex(ec);
    return false;
  }

public:
  multi_example_handler(const context_type context) : _context(context) {}

  void on_example(std::vector<example*>* ev)
  {
    for (example* ec : *ev) work_on_example(_context.get_master(), ec);
    if (try_complete_multi_ex(*ev))
    {
      _context.template process<multi_ex, learn_multi_ex>(*ev);
    }
    VW::finish_example_vector(_context.get_master(), *ev);
  }

private:
  context_type _context;
};

// ready_examples_queue / custom_examples_queue - adapters for connecting example handler to parser produce-consume loop
// for single- and multi-threaded scenarios
class ready_examples_queue
{
public:
  ready_examples_queue(vw& master) : _master(master){}

  std::vector<example*>* pop() { return !_master.early_terminate ? VW::get_example(_master.example_parser) : nullptr; }

private:
  vw& _master;
};

class custom_examples_queue
{
public:
  custom_examples_queue(std::vector<example*> examples) : _examples(examples) {}

  std::vector<example*>* pop() { return _examples.size() ? &_examples : nullptr; }

private:
  std::vector<example*> _examples;
  size_t _index{0};
};

template <typename queue_type, typename handler_type>
void process_examples(queue_type& examples, handler_type& handler)
{
  std::vector<example*>* ev;

  while ((ev = examples.pop()) != nullptr) handler.on_example(ev);
}

template <typename context_type>
void generic_driver(ready_examples_queue& examples, context_type& context)
{
  if (context.get_master().l->is_multiline)
  {
    using handler_type = multi_example_handler<context_type>;
    handler_type handler(context);
    process_examples(examples, handler);
  }
  else
  {
    using handler_type = single_example_handler<context_type>;
    handler_type handler(context);
    process_examples(examples, handler);
  }
  drain_examples(context.get_master());
}

void generic_driver(vw& all)
{
  single_instance_context context(all);
  ready_examples_queue examples(all);
  generic_driver(examples, context);
}

void generic_driver(const std::vector<vw*>& all)
{
  multi_instance_context context(all);
  ready_examples_queue examples(context.get_master());
  generic_driver(examples, context);
}

template <typename handler_type>
void generic_driver_onethread(vw& all)
{
  single_instance_context context(all);
  handler_type handler(context);
  auto multi_ex_fptr = [&handler](vw& all, std::vector<example*> examples) {
    all.example_parser->end_parsed_examples += examples.size();  // divergence: lock & signal
    custom_examples_queue examples_queue(examples);
    process_examples(examples_queue, handler);
  };
  parse_dispatch(all, multi_ex_fptr);
  all.l->end_examples();
}

void generic_driver_onethread(vw& all)
{
  if (all.l->is_multiline)
    generic_driver_onethread<multi_example_handler<single_instance_context>>(all);
  else
    generic_driver_onethread<single_example_handler<single_instance_context>>(all);
}

float recur_sensitivity(void*, base_learner& base, example& ec) { return base.sensitivity(ec); }

}  // namespace LEARNER
}  // namespace VW
