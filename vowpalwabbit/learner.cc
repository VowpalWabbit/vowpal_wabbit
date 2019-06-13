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

  template <class T, void (*process_impl)(T&, vw&)>
  void process(T& ec, vw& context)
  {
    process_impl(ec, context);
  }
  
  template <class T, void (*process_impl)(T&, vw&)>
  void process(T& ec, vector<vw*>& context)
  {
    // start with last as the first instance will free the example as it is the owner
    for (auto it = context.rbegin(); it != context.rend(); ++it) process<T, process_impl>(ec, **it);
  }
  
  template <class Ex, class Ctx, void (*dispatch_impl)(Ex&, vw&)>
  void dispatch(Ex& ec, Ctx& context)
  {
    process<Ex, dispatch_impl>(ec, context);                   // call learn or predict
  }
  
  bool inline is_save_cmd(example* ec)
  {
    return (ec->tag.size() >= 4) && (0 == strncmp((const char*)ec->tag.begin(), "save", 4));
  }

  template <class Ctx>
  void dispatch_single_line(example*& ec, Ctx& context)
  {
    if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
      dispatch<example, Ctx, learn_ex>(*ec, context);
    else if (ec->end_pass)
      dispatch<example, Ctx, end_pass>(*ec, context);
    else if (is_save_cmd(ec))
      dispatch<example, Ctx, save>(*ec, context);
    else
      dispatch<example, Ctx, learn_ex>(*ec, context);
  }

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

  /* is this just a newline */
  inline bool example_is_newline_not_header(example& ec) { return (example_is_newline(ec) && !ec_is_example_header(ec)); }

  /* Adds an example to multiline collection
   * Returns: true if complete and false if incomplete example */
  bool complete_multi_ex(example* ec, multi_ex& ec_seq, vw& all)
  {
    const bool is_test_ec = all.p->lp.test_label(&ec->l);
    const bool is_newline = (example_is_newline_not_header(*ec) && is_test_ec);
    if (!is_newline) {
      ec_seq.push_back(ec);
    } else {
      VW::finish_example(all, *ec);
    }
    return is_newline;
  }

  template<class Ctx>
  bool try_complete_multi_ex(example* ec, multi_ex& ec_seq, vw& master, Ctx& context)
  {
    if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
      return complete_multi_ex(ec, ec_seq, master);
    else if (ec->end_pass)
      dispatch<example, Ctx, end_pass>(*ec, context);
    else if (is_save_cmd(ec))
      dispatch<example, Ctx, save>(*ec, context);
    else
      return complete_multi_ex(ec, ec_seq, master);
    return false;
  }

/*template <void (*f)(vw&, multi_ex&)>
void multi_ex_generic_driver(vw& all)
{
  multi_ex ec_seq;
  example* ec = nullptr;
  while (all.early_terminate == false)
  {
    ec = VW::get_example(all.p);
    on_new_partial_ex<f>(ec, ec_seq, all);
    if (ec == nullptr)
      break;
  }

  if (all.early_terminate)  // drain any extra examples from parser.
    while ((ec = VW::get_example(all.p)) != nullptr) VW::finish_example(all, *ec);

  all.l->end_examples();
}

template <class T, void (*f)(T, example*)>
void generic_driver(vw& all, T context)
{
  example* ec = nullptr;

  while (all.early_terminate == false)
    if ((ec = VW::get_example(all.p)) != nullptr)
      f(context, ec);
    else
      break;
  if (all.early_terminate)  // drain any extra examples from parser.
    while ((ec = VW::get_example(all.p)) != nullptr) VW::finish_example(all, *ec);
  all.l->end_examples();
}

void process_multiple(vector<vw*> alls, example* ec)
{
  // start with last as the first instance will free the example as it is the owner
  for (auto it = alls.rbegin(); it != alls.rend(); ++it) process_example(**it, ec);
}

void generic_driver(vector<vw*> alls)
{
  generic_driver<vector<vw*>, process_multiple>(**alls.begin(), alls);

  // skip first as it already called end_examples()
  auto it = alls.begin();
  for (it++; it != alls.end(); it++) (*it)->l->end_examples();
}*/

  template<class Ctx>
  bool produce_single_line_example(vw& master, example*& ec, Ctx&) {
    if (master.early_terminate == true) return false;
    ec = VW::get_example(master.p);
    return ec != nullptr;
  }
  
  template<class Ctx>
  bool produce_multi_line_example(vw& master, multi_ex& ec_seq, Ctx& context) {
    example* ec = nullptr;
    ec_seq.clear();
    while (produce_single_line_example(master, ec, context)) {
      if (try_complete_multi_ex(ec, ec_seq, master, context)) {
        break;
      }
    }
    return !ec_seq.empty();
  }
  
  template <class Ex, class Ctx, bool (*produce)(vw&, Ex&, Ctx&), void (*consume)(Ex&, Ctx&)>
  void new_driver(vw& master, Ctx& context)
  {
    Ex ex;
    
    while (produce(master, ex, context))
      consume(ex, context);
    
    if (master.early_terminate) { // drain any extra examples from parser.
      example* ec = nullptr;
      while ((ec = VW::get_example(master.p)) != nullptr) VW::finish_example(master, *ec);
    }
    master.l->end_examples();
  }
  
  void generic_driver(vw& all)
  {
    if (all.l->is_multiline)
      new_driver<multi_ex, vw, produce_multi_line_example, dispatch<multi_ex, vw, learn_multi_ex>>(all, all);
    else
      new_driver<example*, vw, produce_single_line_example, dispatch_single_line<vw>>(all, all);
  }
  
  void generic_driver(vw& master, std::vector<vw*> all)
  {
    if (master.l->is_multiline)
      new_driver<multi_ex, std::vector<vw*>, produce_multi_line_example, dispatch<multi_ex, std::vector<vw*>, learn_multi_ex>>(master, all);
    else
      new_driver<example*, std::vector<vw*>, produce_single_line_example, dispatch_single_line<std::vector<vw*>>>(master, all);
  }

/*void generic_driver_onethread(vw& all)
{
  if (all.l->is_multiline)
  {
    multi_ex ctxt;
    auto multi_ex_fptr = [&ctxt](vw& all, v_array<example*> examples) {
      all.p->end_parsed_examples += examples.size();  // divergence: lock & signal
      for (size_t i = 0; i < examples.size(); ++i) on_new_partial_ex<process_multi_ex>(examples[i], ctxt, all);
    };

    parse_dispatch(all, multi_ex_fptr);

    // flush accumulated examples if there is no newline
    // at the end of the file
    on_new_partial_ex<process_multi_ex>(nullptr, ctxt, all);
  }
  else
    parse_dispatch(all, dispatch);

  all.l->end_examples();
}*/

float recur_sensitivity(void*, base_learner& base, example& ec) { return base.sensitivity(ec); }

}  // namespace LEARNER
