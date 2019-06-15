#include "parser.h"
#include "vw.h"
#include "parse_regressor.h"
#include "parse_dispatch_loop.h"
using namespace std;

namespace prediction_type
{
#define CASE(type) \
  case type:       \
    return #type;

const char* to_string(prediction_type_t prediction_type)
{
  switch (prediction_type)
  {
    CASE(scalar)
    CASE(scalars)
    CASE(action_scores)
    CASE(action_probs)
    CASE(multiclass)
    CASE(multilabels)
    CASE(prob)
    CASE(multiclassprobs)
    default:
      return "<unsupported>";
  }
}
}  // namespace prediction_type

namespace LEARNER
{
  void learn_ex(example& ec, vw& all)
  {
    all.learn(ec);
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
    string final_regressor_name = all.final_regressor_name;

    if ((ec.tag).size() >= 6 && (ec.tag)[4] == '_')
      final_regressor_name = string(ec.tag.begin() + 5, (ec.tag).size() - 5);

    if (!all.quiet)
      all.trace_message << "saving regressor to " << final_regressor_name << endl;
    save_predictor(all, final_regressor_name, 0);

    VW::finish_example(all, ec);
  }
  
  class single_instance_context {
  public:
    single_instance_context(vw& all)
    : _all(all) {}
    
    vw& get_master() {return _all;}
    
    template <class T, void (*process_impl)(T&, vw&)>
    void process(T& ec) {
      process_impl(ec, _all);
    }
  private:
    vw& _all;
  };
  
  class multi_instance_context {
  public:
    multi_instance_context(const std::vector<vw*>& all)
    : _all(all) {}
    
    vw& get_master() {return *_all.front();}
    
    template <class T, void (*process_impl)(T&, vw&)>
    void process(T& ec) {
      // start with last as the first instance will free the example as it is the owner
      for (auto it = _all.rbegin(); it != _all.rend(); ++it) process_impl(ec, **it);
    }
  private:
    std::vector<vw*> _all;
  };
  
  /* example headers have the word "shared" */
  bool ec_is_example_header(example& ec)
  {
    v_array<CB::cb_class> costs = ec.l.cb.costs;
    if (costs.size() != 1)
      return false;
    if (costs[0].probability == -1.f)
      return true;
    return false;
  }
  
  bool inline is_save_cmd(example* ec)
  {
    return (ec->tag.size() >= 4) && (0 == strncmp((const char*)ec->tag.begin(), "save", 4));
  }
  
  /* is this just a newline */
  inline bool example_is_newline_not_header(example& ec) { return (example_is_newline(ec) && !ec_is_example_header(ec)); }

  template<typename Context>
  class single_example_handler {
  public:
    single_example_handler(Context context)
    : _context(context) {}
    
    void on_example(example* ec) {
      if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
        _context.template process<example, learn_ex>(*ec);
      else if (ec->end_pass)
        _context.template process<example, end_pass>(*ec);
      else if (is_save_cmd(ec))
        _context.template process<example, save>(*ec);
      else
        _context.template process<example, learn_ex>(*ec);
    }
  private:
    Context _context;
  };
  
  template<typename Context>
  class multi_example_handler {
  private:
    bool complete_multi_ex(example* ec)
    {
      auto& master = _context.get_master();
      const bool is_test_ec = master.p->lp.test_label(&ec->l);
      const bool is_newline = (example_is_newline_not_header(*ec) && is_test_ec);
      if (!is_newline) {
        ec_seq.push_back(ec);
      } else {
        VW::finish_example(master, *ec);
      }
      return is_newline;
    }
    
    bool try_complete_multi_ex(example* ec) {
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
    multi_example_handler(Context context)
    : _context(context) {}
    
    void on_example(example* ec) {
      if (try_complete_multi_ex(ec)) {
        _context.template process<multi_ex, learn_multi_ex>(ec_seq);
        ec_seq.clear();
      }
    }
    
  private:
    Context _context;
    multi_ex ec_seq;
  };
  
  class ready_examples {
  public:
    ready_examples(vw& master)
    : _master(master) {}
    
    example* get() {
      return !_master.early_terminate ? VW::get_example(_master.p) : nullptr;
    }
  private:
    vw& _master;
  };
  
  class not_ready_examples {
  public:
    not_ready_examples(v_array<example*> examples)
    : _examples(examples) {}
    
    example* get() {
      return _index < _examples.size() ? _examples[_index++] : nullptr;
    }
  private:
    v_array<example*> _examples;
    size_t _index {0};
  };
  
  template <class Source, class Handler>
  void new_driver(Source& example_source, Handler& handler)
  {
    example* ex;
    
    while ((ex = example_source.get()) != nullptr)
      handler.on_example(ex);
  }
  
  void drain_examples_if_needed(vw& all) {
    if (all.early_terminate) { // drain any extra examples from parser.
      example* ec = nullptr;
      while ((ec = VW::get_example(all.p)) != nullptr) VW::finish_example(all, *ec);
    }
    all.l->end_examples();
  }
  
  void generic_driver(vw& all)
  {
    single_instance_context context(all);
    ready_examples examples_source(all);
    if (all.l->is_multiline) {
      using handler_type = multi_example_handler<single_instance_context>;
      handler_type handler(context);
      new_driver<ready_examples, handler_type>(examples_source, handler);
    } else {
      using handler_type = single_example_handler<single_instance_context>;
      handler_type handler(context);
      new_driver<ready_examples, handler_type>(examples_source, handler);
    }
    drain_examples_if_needed(all);
  }
  
  void generic_driver(vw& master, std::vector<vw*> all)
  {
    multi_instance_context context(all);
    ready_examples examples_source(context.get_master());
    if (master.l->is_multiline) {
      using handler_type = multi_example_handler<multi_instance_context>;
      handler_type handler(context);
      new_driver<ready_examples, handler_type>(examples_source, handler);
    } else {
      using handler_type = single_example_handler<multi_instance_context>;
      handler_type handler(context);
      new_driver<ready_examples, handler_type>(examples_source, handler);
    }
    drain_examples_if_needed(context.get_master());
  }
  
  template<typename handler_type>
  void generic_driver_onethread(vw& all) {
    single_instance_context context(all);
    handler_type handler(context);
    auto multi_ex_fptr = [&handler](vw& all, v_array<example*> examples) {
      all.p->end_parsed_examples += examples.size();  // divergence: lock & signal
      not_ready_examples examples_source(examples);
      new_driver<not_ready_examples, handler_type>(examples_source, handler);
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
