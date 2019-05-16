#include "conditional_contextual_bandit.h"
#include "reductions.h"
#include "example.h"
#include "global_data.h"
#include "cache.h"
#include "vw.h"
#include "interactions.h"
#include "label_dictionary.h"
#include "cb_adf.h"
#include "cb_algs.h"

#include <numeric>
#include <algorithm>
#include <unordered_set>

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

struct ccb
{
  example* shared;
  std::vector<example*> actions, slots;
  uint32_t chosen_action_index;
  std::vector<uint32_t> origin_index;
  CB::cb_class cb_label, default_cb_label;
  std::vector<bool> exclude_list /*, include_list*/;
  std::vector<std::string> generated_interactions;
  std::vector<std::string>* original_interactions;
  std::vector<CCB::label> stored_labels;
};

namespace CCB
{
static constexpr uint32_t SHARED_EX_INDEX = 0;
static constexpr uint32_t TOP_ACTION_INDEX = 0;

void clear_all(ccb& data)
{
  data.shared = nullptr;
  data.actions.clear();
  data.slots.clear();
  data.chosen_action_index = 0;
  // data.exclude_list.clear();
  // data.include_list.clear();
}

// split the slots, the actions and the shared example from the multiline example
void split_multi_example(const multi_ex& examples, ccb& data)
{
  for (auto ex : examples)
  {
    switch (ex->l.conditional_contextual_bandit.type)
    {
      case example_type::shared:
        data.shared = ex;
        break;
      case example_type::action:
        data.actions.push_back(ex);
        break;
      case example_type::slot:
        data.slots.push_back(ex);
        break;
      default:
        THROW("ccb_adf_explore: badly formatted example - invalid example type");
    }
  }
}

template <bool is_learn>
void sanity_checks(ccb& data)
{
  if (data.slots.size() > data.actions.size())
    THROW("ccb_adf_explore: badly formatted example - number of actions "
        << data.actions.size() << " must be greater than the number of slots " << data.slots.size());

  if (is_learn)
  {
    for (auto slot : data.slots)
    {
      if (slot->l.conditional_contextual_bandit.outcome != nullptr &&
          slot->l.conditional_contextual_bandit.outcome->probabilities.size() == 0)
        THROW("ccb_adf_explore: badly formatted example - missing label probability");
    }
  }
}

// create empty/default cb labels
void create_cb_labels(ccb& data)
{
  data.shared->l.cb.costs = v_init<CB::cb_class>();
  data.shared->l.cb.costs.push_back(data.default_cb_label);
  for (example* action : data.actions) action->l.cb.costs = v_init<CB::cb_class>();
}

// the polylabel (union) must be manually cleaned up
void delete_cb_labels(ccb& data)
{
  data.shared->l.cb.costs.delete_v();
  for (example* action : data.actions) action->l.cb.costs.delete_v();
}

void attach_label_to_example(example* example, conditional_contexual_bandit_outcome* outcome, ccb& data)
{
  // save the cb label
  // Action is unused in cb
  data.cb_label.action = 0;
  data.cb_label.probability = outcome->probabilities[0].score;
  data.cb_label.cost = outcome->cost;

  example->l.cb.costs.push_back(data.cb_label);
}

template <bool is_learn>
void save_action_scores(ccb& data, decision_scores_t& decision_scores)
{
  // save a copy
  auto copy = v_init<ACTION_SCORE::action_score>();
  copy_array(copy, data.shared->pred.a_s);
  decision_scores.push_back(copy);

  // correct indices: we want index relative to the original ccb multi-example, with no actions filtered
  for (auto& action_score : copy) action_score.action = data.origin_index[action_score.action];

  // exclude the chosen action from next slots
  if (!is_learn)
    data.exclude_list[copy[0].action] = true;
  else
    data.exclude_list[data.chosen_action_index] = true;
}

void clear_pred_and_label(ccb& data)
{
  data.shared->pred.a_s.delete_v();
  data.actions[data.chosen_action_index]->l.cb.costs.delete_v();
}

// true if there exists at least 1 action in the cb multi-example
bool has_action(multi_ex& cb_ex) { return cb_ex.size() > 1; }

// This function intentionally does not handle increasing the num_features of the example because
// the output_example function has special logic to ensure the number of feaures is correctly calculated.
// Copy anything in default namespace for slot to ccb_slot_namespace in shared
// Copy other slot namespaces to shared
void inject_slot_features(example* shared, example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == constant_namespace)
    {
      continue;
    }
    else if (index == default_namespace)  // slot default namespace has a special namespace in shared
    {
      LabelDict::add_example_namespace(*shared, ccb_slot_namespace, slot->feature_space[32]);
    }
    else
    {
      LabelDict::add_example_namespace(*shared, index, slot->feature_space[index]);
    }
  }
}

// Flattens all features of the action into the history namespace in the shared example.
void inject_history_features(example* shared, example* action)
{
  for (auto index : action->indices)
  {
    LabelDict::add_example_namespace(*shared, ccb_history_namespace, action->feature_space[index]);
  }
}

void remove_slot_features(example* shared, example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == constant_namespace)
    {
      continue;
    }
    else if (index == default_namespace)  // slot default namespace has a special namespace in shared
    {
      LabelDict::del_example_namespace(*shared, ccb_slot_namespace, slot->feature_space[32]);
    }
    else
    {
      LabelDict::del_example_namespace(*shared, index, slot->feature_space[index]);
    }
  }
}

// Generates all combinations of 4th order interactions between namespaces in [shared,history,slot,action]
void calculate_and_insert_interactions(example* shared, example* slot, std::vector<example*> actions, std::vector<std::string>& vec)
{
  vec.reserve(shared->indices.size() * slot->indices.size() + vec.size());

  for (auto shared_index : shared->indices)
  {
    // History namespace is included in the shared example, it is added explicitly for each interaction.
    if (shared_index == ccb_history_namespace)
    {
      continue;
    }

    for (auto action : actions)
    {
      for (auto action_index : action->indices)
      {
        for (auto slot_index : slot->indices)
        {
          char slot_ns_index = (slot_index == default_namespace) ? ccb_slot_namespace : slot_index;
          vec.push_back({(char)shared_index, slot_ns_index, (char)ccb_history_namespace, (char)action_index});
        }
      }
    }
  }
}

// build a cb example from the ccb example
template <bool is_learn>
void build_cb_example(multi_ex& cb_ex, example* slot, ccb& data)
{
  bool slot_has_label = slot->l.conditional_contextual_bandit.outcome != nullptr;

  // Merge the slot features with the shared example and set it in the cb multi-example
  // TODO is it imporant for total_sum_feat_sq and num_features to be correct at this point?
  inject_slot_features(data.shared, slot);
  cb_ex.push_back(data.shared);

  // For V0, include list is not supported
  // retrieve the action index whitelist (if the list is empty, then all actions are white-listed)
  // data.include_list.clear();
  // for (uint32_t included_action_id : slot->l.conditional_contextual_bandit.explicit_included_actions)
  //   data.include_list.insert(included_action_id);

  // set the available actions in the cb multi-example
  uint32_t index = 0;
  // Ensure the origin_index vector is big enough for this example.
  data.origin_index.resize(data.actions.size());
  for (size_t i = 0; i < data.actions.size(); i++)
  {
    // For V0, include list is not supported
    // filter actions that are not explicitly included
    // if (!data.include_list.empty() && data.include_list.find((uint32_t)i) == data.include_list.end())
    //   continue;

    // filter actions chosen by previous slots
    if (data.exclude_list[i])
      continue;

    // select the action
    cb_ex.push_back(data.actions[i]);

    // save the original index from the root multi-example
    data.origin_index[index++] = (uint32_t)i;

    // remember the index of the chosen action
    if (is_learn && slot_has_label &&
        i == slot->l.conditional_contextual_bandit.outcome->probabilities[0].action)
    {
      // This is used to exclude this action from further slots and to remove the label after the call.
      data.chosen_action_index = (uint32_t)i;
      attach_label_to_example(data.actions[i], slot->l.conditional_contextual_bandit.outcome, data);
    }
  }

  // Must reset this in case the pooled example has stale data here.
  data.shared->pred.a_s = v_init<ACTION_SCORE::action_score>();
}

// iterate over slots contained in the multi-example, and for each slot, build a cb example and perform a
// cb_explore_adf call.
template <bool is_learn>
void learn_or_predict(ccb& data, multi_learner& base, multi_ex& examples)
{
  clear_all(data);
  split_multi_example(examples, data);  // split shared, actions and slots
  sanity_checks<is_learn>(data);

  // Stash the CCB labels before rewriting them.
  data.stored_labels.clear();
  for (auto ex : examples)
  {
    data.stored_labels.push_back({ex->l.conditional_contextual_bandit.type,
        ex->l.conditional_contextual_bandit.outcome, ex->l.conditional_contextual_bandit.explicit_included_actions});
  }

  // This will overwrite the labels with CB.
  create_cb_labels(data);

  // Reset exclusion list for this example.
  data.exclude_list.assign(data.actions.size(), false);

    auto decision_scores = v_init<ACTION_SCORE::action_scores>();

  // for each slot, re-build the cb example and call cb_explore_adf
  for (example* slot : data.slots)
  {
    multi_ex cb_ex;
    build_cb_example<is_learn>(cb_ex, slot, data);

    // Namespace crossing for slot features.
    // If the slot example only has the constant namespace, there will be no extra crossing and so skip that logic.
    if (!(slot->indices.size() == 1 && slot->indices[0] == constant_namespace))
    {
      data.generated_interactions.clear();
      // TODO be more efficient here
      std::copy(data.original_interactions->begin(), data.original_interactions->end(),
          std::back_inserter(data.generated_interactions));
      // TODO currently all action namespaces are used adding redundant interactions.
      calculate_and_insert_interactions(data.shared, slot, data.actions, data.generated_interactions);
      size_t removed_cnt;
      size_t sorted_cnt;
      INTERACTIONS::sort_and_filter_duplicate_interactions(data.generated_interactions, true, removed_cnt, sorted_cnt);
      data.shared->interactions = &data.generated_interactions;
    }

    if (has_action(cb_ex))
    {  // the cb example contains at least 1 action
      multiline_learn_or_predict<is_learn>(base, cb_ex, examples[0]->ft_offset);
      save_action_scores<is_learn>(data, decision_scores);
      //inject_history_features(data.shared, data.actions[data.chosen_action_index]);
      clear_pred_and_label(data);
    }
    else
    {  // the cb example contains no action => cannot decide
      auto empty_action_scores = v_init<ACTION_SCORE::action_score>();
      decision_scores.push_back(empty_action_scores);
    }

    data.shared->interactions = data.original_interactions;
    remove_slot_features(data.shared, slot);
  }

  delete_cb_labels(data);

  // Restore ccb labels to the example objects.
  for (size_t i = 0; i < examples.size(); i++)
  {
    examples[i]->l.conditional_contextual_bandit = {
        data.stored_labels[i].type, data.stored_labels[i].outcome, data.stored_labels[i].explicit_included_actions};
  }

  // Save the predictions
  examples[0]->pred.decision_scores = decision_scores;
}

void print_decision_scores(int f, decision_scores_t& decision_scores)
{
  if (f >= 0)
  {
    std::stringstream ss;
    for (auto slot : decision_scores)
    {
      std::string delimiter = "";
      for (auto action_score : slot)
      {
        ss << delimiter << action_score.action << ':' << action_score.score;
        delimiter = ",";
      }
      ss << '\n';
    }
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}

void print_update(vw& all, std::vector<example*>& slots, decision_scores_t& decision_scores, size_t num_features)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    std::string label_str = "";
    std::string delim = "";
    int counter = 0;
    for (auto slot : slots)
    {
      counter++;

      auto outcome = slot->l.conditional_contextual_bandit.outcome;
      if (outcome == nullptr)
      {
        label_str += delim;
        label_str += "?";
      }
      else
      {
        label_str += delim;
        label_str += std::to_string(outcome->probabilities[0].action);
        label_str += ":";
        label_str += std::to_string(outcome->cost);
      }

      delim = ",";

      // Stop after 2...
      if (counter > 1 && slots.size() > 2)
      {
        label_str += delim;
        label_str += "...";
        break;
      }
    }
    std::ostringstream label_buf;
    label_buf << std::setw(all.sd->col_current_label) << std::right << std::setfill(' ') << label_str;

    std::string pred_str = "";
    delim = "";
    counter = 0;
    for (auto slot : decision_scores)
    {
      counter++;
      pred_str += delim;
      pred_str += std::to_string(slot[0].action);
      delim = ",";

      // Stop after 3...
      if (counter > 2)
      {
        pred_str += delim;
        pred_str += "...";
        break;
      }
    }
    std::ostringstream pred_buf;
    pred_buf << std::setw(all.sd->col_current_predict) << std::right << std::setfill(' ') << pred_str;

    all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf.str(), pred_buf.str(), num_features,
        all.progress_add, all.progress_arg);
  }
}

void output_example(vw& all, ccb& /*c*/, multi_ex& ec_seq)
{
  if (ec_seq.size() <= 0)
    return;

  std::vector<example*> slots;
  size_t num_features = 0;
  float loss = 0.;

  // Should this be done for shared, action and slot?
  for (auto ec : ec_seq)
  {
    num_features += ec->num_features;

    if (ec->l.conditional_contextual_bandit.type == CCB::example_type::slot)
    {
      slots.push_back(ec);
    }
  }

  // What does it mean for not all of the slots to be labeled? Does it become a non-labeled example at that point?
  // Is it hold out?
  bool labeled_example = true;
  auto preds = ec_seq[0]->pred.decision_scores;
  for (size_t i = 0; i < slots.size(); i++)
  {
    auto outcome = slots[i]->l.conditional_contextual_bandit.outcome;
    if (outcome != nullptr)
    {
      float l = CB_ALGS::get_cost_estimate(
          outcome->probabilities[TOP_ACTION_INDEX], outcome->cost, preds[i][TOP_ACTION_INDEX].action);
      loss += l * preds[i][TOP_ACTION_INDEX].score;
    }
    else
    {
      labeled_example = false;
    }
  }

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) holdout_example &= ec_seq[i]->test_only;

  // TODO what does weight mean here?
  all.sd->update(holdout_example, labeled_example, loss, ec_seq[SHARED_EX_INDEX]->weight, num_features);

  for (auto sink : all.final_prediction_sink)
    print_decision_scores(sink, ec_seq[SHARED_EX_INDEX]->pred.decision_scores);

  CCB::print_update(all, slots, preds, num_features);
}

void finish_multiline_example(vw& all, ccb& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, data, ec_seq);
    CB_ADF::global_print_newline(all);
  }

  for (auto a_s : ec_seq[0]->pred.decision_scores)
  {
    a_s.delete_v();
  }
  ec_seq[0]->pred.decision_scores.delete_v();

  // Delete all of the labels originally allocated by the parser.
  for (auto ex : ec_seq)
  {
    all.p->lp.delete_label(&ex->l);
  }

  VW::clear_seq_and_finish_examples(all, ec_seq);
}

void finish(ccb& data)
{
  data.actions.~vector<example*>();
  data.slots.~vector<example*>();
  data.origin_index.~vector<uint32_t>();
  data.exclude_list.~vector<bool>();
  // data.include_list.~unordered_set<uint32_t>();
  data.cb_label.~cb_class();
  data.default_cb_label.~cb_class();
  data.generated_interactions.~vector<std::string>();
  data.stored_labels.~vector<CCB::label>();
}

base_learner* ccb_explore_adf_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<ccb>();
  bool ccb_explore_adf_option = false;
  option_group_definition new_options("Conditional Contextual Bandit Exploration with Action Dependent Features");
  new_options.add(make_option("ccb_explore_adf", ccb_explore_adf_option)
                      .keep()
                      .help("Do Conditional Contextual Bandit learning with multiline action dependent features."));
  options.add_and_parse(new_options);

  if (!ccb_explore_adf_option)
    return nullptr;

  if (!options.was_supplied("cb_explore_adf"))
  {
    options.insert("cb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  if (!options.was_supplied("cb_sample"))
  {
    options.insert("cb_sample", "");
    options.add_and_parse(new_options);
  }

  auto base = as_multiline(setup_base(options, all));
  all.p->lp = CCB::ccb_label_parser;
  all.label_type = label_type::ccb;

  // Extract from lower level reductions
  data->default_cb_label = {FLT_MAX, 0, -1.f, 0.f};
  data->shared = nullptr;
  data->original_interactions = &all.interactions;

  learner<ccb, multi_ex>& l =
      init_learner(data, base, learn_or_predict<true>, learn_or_predict<false>, 1, prediction_type::decision_probs);

  l.set_finish_example(finish_multiline_example);
  l.set_finish(CCB::finish);
  return make_base(l);
}

size_t read_cached_label(shared_data*, void* v, io_buf& cache)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  size_t read_count = 0;
  char* read_ptr;

  size_t next_read_size = sizeof(ld->type);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  ld->type = *(CCB::example_type*)read_ptr;
  read_count += sizeof(ld->type);

  bool is_outcome_present;
  next_read_size = sizeof(bool);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  is_outcome_present = *(bool*)read_ptr;
  read_count += sizeof(is_outcome_present);

  if (is_outcome_present)
  {
    ld->outcome = new CCB::conditional_contexual_bandit_outcome();
    ld->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

    next_read_size = sizeof(ld->outcome->cost);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
      return 0;
    ld->outcome->cost = *(float*)read_ptr;
    read_count += sizeof(ld->outcome->cost);

    uint32_t size_probs;
    next_read_size = sizeof(size_probs);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
      return 0;
    size_probs = *(uint32_t*)read_ptr;
    read_count += sizeof(size_probs);

    for (uint32_t i = 0; i < size_probs; i++)
    {
      ACTION_SCORE::action_score a_s;
      next_read_size = sizeof(a_s);
      if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
        return 0;
      a_s = *(ACTION_SCORE::action_score*)read_ptr;
      read_count += sizeof(a_s);

      ld->outcome->probabilities.push_back(a_s);
    }
  }

  uint32_t size_includes;
  next_read_size = sizeof(size_includes);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  size_includes = *(uint32_t*)read_ptr;
  read_count += sizeof(size_includes);

  for (uint32_t i = 0; i < size_includes; i++)
  {
    uint32_t include;
    next_read_size = sizeof(include);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
      return 0;
    include = *(uint32_t*)read_ptr;
    read_count += sizeof(include);
    ld->explicit_included_actions.push_back(include);
  }

  return read_count;
}

float ccb_weight(void*) { return 1.; }

void cache_label(void* v, io_buf& cache)
{
  char* c;
  CCB::label* ld = static_cast<CCB::label*>(v);
  size_t size = sizeof(uint8_t)  // type
      + sizeof(bool)             // outcome exists?
      + (ld->outcome == nullptr ? 0
                                : sizeof(ld->outcome->cost)                                    // cost
                    + sizeof(uint32_t)                                                         // probabilities size
                    + sizeof(ACTION_SCORE::action_score) * ld->outcome->probabilities.size())  // probabilities
      + sizeof(uint32_t)  // explicit_included_actions size
      + sizeof(uint32_t) * ld->explicit_included_actions.size();

  cache.buf_write(c, size);

  *(uint8_t*)c = static_cast<uint8_t>(ld->type);
  c += sizeof(ld->type);

  *(bool*)c = ld->outcome != nullptr;
  c += sizeof(bool);

  if (ld->outcome != nullptr)
  {
    *(float*)c = ld->outcome->cost;
    c += sizeof(ld->outcome->cost);

    *(uint32_t*)c = convert(ld->outcome->probabilities.size());
    c += sizeof(uint32_t);

    for (const auto& score : ld->outcome->probabilities)
    {
      *(ACTION_SCORE::action_score*)c = score;
      c += sizeof(ACTION_SCORE::action_score);
    }
  }

  *(uint32_t*)c = convert(ld->explicit_included_actions.size());
  c += sizeof(uint32_t);

  for (const auto& included_action : ld->explicit_included_actions)
  {
    *(uint32_t*)c = included_action;
    c += sizeof(included_action);
  }
}

void default_label(void* v)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  ld->outcome = nullptr;
  ld->explicit_included_actions = v_init<uint32_t>();
  ld->type = example_type::unset;
}

bool test_label(void* v)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  return ld->outcome == nullptr;
}

void delete_label(void* v)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  if (ld->outcome)
  {
    ld->outcome->probabilities.delete_v();
    delete ld->outcome;
    ld->outcome = nullptr;
  }
  ld->explicit_included_actions.delete_v();
}

void copy_label(void* dst, void* src)
{
  CCB::label* ldDst = static_cast<CCB::label*>(dst);
  CCB::label* ldSrc = static_cast<CCB::label*>(src);

  if (ldSrc->outcome)
  {
    ldDst->outcome = new CCB::conditional_contexual_bandit_outcome();
    ldDst->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

    ldDst->outcome->cost = ldSrc->outcome->cost;
    copy_array(ldDst->outcome->probabilities, ldSrc->outcome->probabilities);
  }

  copy_array(ldDst->explicit_included_actions, ldSrc->explicit_included_actions);
  ldDst->type = ldSrc->type;
}

ACTION_SCORE::action_score convert_to_score(const substring& action_id_str, const substring& probability_str)
{
  auto action_id = static_cast<uint32_t>(int_of_substring(action_id_str));
  auto probability = float_of_substring(probability_str);
  if (nanpattern(probability))
    THROW("error NaN probability: " << probability_str);

  if (probability > 1.0)
  {
    std::cerr << "invalid probability > 1 specified for an outcome, resetting to 1.\n";
    probability = 1.0;
  }
  if (probability < 0.0)
  {
    std::cerr << "invalid probability < 0 specified for an outcome, resetting to 0.\n";
    probability = .0;
  }

  return {action_id, probability};
}

//<action>:<cost>:<probability>,<action>:<probability>,<action>:<probability>,…
CCB::conditional_contexual_bandit_outcome* parse_outcome(substring& outcome)
{
  auto& ccb_outcome = *(new CCB::conditional_contexual_bandit_outcome());

  auto split_commas = v_init<substring>();
  tokenize(',', outcome, split_commas);

  auto split_colons = v_init<substring>();
  tokenize(':', split_commas[0], split_colons);

  if (split_colons.size() != 3)
    THROW("Malformed ccb label");

  ccb_outcome.probabilities = v_init<ACTION_SCORE::action_score>();
  ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[2]));

  ccb_outcome.cost = float_of_substring(split_colons[1]);
  if (nanpattern(ccb_outcome.cost))
    THROW("error NaN cost: " << split_colons[1]);

  split_colons.clear();

  for (size_t i = 1; i < split_commas.size(); i++)
  {
    tokenize(':', split_commas[i], split_colons);
    if (split_colons.size() != 2)
      THROW("Must be action probability pairs");
    ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[1]));
  }

  split_colons.delete_v();
  split_commas.delete_v();

  return &ccb_outcome;
}

void parse_explicit_inclusions(CCB::label* ld, v_array<substring>& split_inclusions)
{
  for (const auto& inclusion : split_inclusions)
  {
    ld->explicit_included_actions.push_back(int_of_substring(inclusion));
  }
}

void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
{
  CCB::label* ld = static_cast<CCB::label*>(v);

  if (words.size() < 2)
    THROW("ccb labels may not be empty");
  if (!substring_equal(words[0], "ccb"))
  {
    THROW("ccb labels require the first word to be ccb");
  }

  auto type = words[1];
  if (substring_equal(type, "shared"))
  {
    if (words.size() > 2)
      THROW("shared labels may not have a cost");
    ld->type = CCB::example_type::shared;
  }
  else if (substring_equal(type, "action"))
  {
    if (words.size() > 2)
      THROW("action labels may not have a cost");
    ld->type = CCB::example_type::action;
  }
  else if (substring_equal(type, "slot"))
  {
    if (words.size() > 4)
      THROW("ccb slot label can only have a type cost and exclude list");
    ld->type = CCB::example_type::slot;

    // Skip the first two words "ccb <type>"
    for (size_t i = 2; i < words.size(); i++)
    {
      auto is_outcome = std::find(words[i].begin, words[i].end, ':');
      if (is_outcome != words[i].end)
      {
        if (ld->outcome != nullptr)
        {
          THROW("There may be only 1 outcome associated with a slot.")
        }

        ld->outcome = parse_outcome(words[i]);
      }
      else
      {
        tokenize(',', words[i], p->parse_name);
        parse_explicit_inclusions(ld, p->parse_name);
      }
    }

    // If a full distribution has been given, check if it sums to 1, otherwise throw.
    if (ld->outcome && ld->outcome->probabilities.size() > 1)
    {
      float total_pred = std::accumulate(ld->outcome->probabilities.begin(), ld->outcome->probabilities.end(), 0.f,
          [](float result_so_far, ACTION_SCORE::action_score action_pred) {
            return result_so_far + action_pred.score;
          });

      // TODO do a proper comparison here.
      if (total_pred > 1.1f || total_pred < 0.9f)
      {
        THROW("When providing all predicition probabilties they must add up to 1.f");
      }
    }
  }
  else
  {
    THROW("unknown label type: " << type);
  }
}

// Export the definition of this label parser.
label_parser ccb_label_parser = {default_label, parse_label, cache_label, read_cached_label, delete_label, ccb_weight,
    copy_label, test_label, sizeof(CCB::label)};
}  // namespace CCB
