#include "parser.h"
#include "vw.h"
#include "parse_regressor.h"
#include "parse_dispatch_loop.h"
using namespace std;

void dispatch_example(vw& all, example& ec)
{
  all.learn(ec);
  as_singleline(all.l)->finish_example(all, ec);
}

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
void dispatch_end_pass(vw& all, example& ec)
{
  all.current_pass++;
  all.l->end_pass();
  VW::finish_example(all, ec);
}

void save(vw& all, example* ec)
{
  // save state command
  string final_regressor_name = all.final_regressor_name;

  if ((ec->tag).size() >= 6 && (ec->tag)[4] == '_')
    final_regressor_name = string(ec->tag.begin() + 5, (ec->tag).size() - 5);

  if (!all.quiet)
    all.trace_message << "saving regressor to " << final_regressor_name << endl;
  save_predictor(all, final_regressor_name, 0);

  VW::finish_example(all, *ec);
}

bool inline is_save_cmd(example* ec)
{
  return (ec->tag.size() >= 4) && (0 == strncmp((const char*)ec->tag.begin(), "save", 4));
}

void process_example(vw& all, example* ec)
{
  if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
    dispatch_example(all, *ec);
  else if (ec->end_pass)
    dispatch_end_pass(all, *ec);
  else if (is_save_cmd(ec))
    save(all, ec);
  else
    dispatch_example(all, *ec);
}

void process_multi_ex(vw& all, multi_ex& ec_seq)
{
  all.learn(ec_seq);
  as_multiline(all.l)->finish_example(all, ec_seq);
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

  if (example_is_newline_not_header(*ec) && is_test_ec)
  {
    VW::finish_example(all, *ec);
    if (ec_seq.size() == 0)
    {
      cout << "Something is wrong---an example with no choice.  Do you have all 0 features? Or multiple empty lines?"
           << endl;
      return false;
    }
    return true;  // example complete
  }
  ec_seq.push_back(ec);
  return false;
}

template <void (*f)(vw&, multi_ex&)>
void dispatch_multi_ex(vw& all, multi_ex& ec_seq)
{
  f(all, ec_seq);                   // call learn or predict
  VW::finish_example(all, ec_seq);  // clean up
}

template <void (*f)(vw&, multi_ex&)>
void dispatch_multi_ex(vw& all, example* ec, multi_ex& ec_seq)
{
  if (complete_multi_ex(ec, ec_seq, all))
  {
    dispatch_multi_ex<f>(all, ec_seq);
  }
}

template <void (*f)(vw&, multi_ex&)>
void on_new_partial_ex(example* ec, multi_ex& ec_seq, vw& all)
{
  if (ec != nullptr)
  {
    if (ec->indices.size() > 1)  // 1+ nonconstant feature. (most common case first)
      dispatch_multi_ex<f>(all, ec, ec_seq);
    else if (ec->end_pass)
      dispatch_end_pass(all, *ec);
    else if (is_save_cmd(ec))
      save(all, ec);
    else
      dispatch_multi_ex<f>(all, ec, ec_seq);
  }
  else
  {
    if (ec_seq.size() > 0)
      dispatch_multi_ex<f>(all, ec_seq);
  }
}

template <void (*f)(vw&, multi_ex&)>
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
}

void generic_driver(vw& all)
{
  if (all.l->is_multiline)
    multi_ex_generic_driver<process_multi_ex>(all);
  else
    generic_driver<vw&, process_example>(all, all);
}

void dispatch(vw& all, v_array<example*> examples)
{
  all.p->end_parsed_examples += examples.size();  // divergence: lock & signal
  for (size_t i = 0; i < examples.size(); ++i) process_example(all, examples[i]);
}

void generic_driver_onethread(vw& all)
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
}

float recur_sensitivity(void*, base_learner& base, example& ec) { return base.sensitivity(ec); }

}  // namespace LEARNER
