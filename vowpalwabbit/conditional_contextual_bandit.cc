#include "conditional_contextual_bandit.h"
#include "reductions.h"
#include "example.h"
#include "global_data.h"
#include "cache.h"
#include "vw.h"
#include "cb_adf.h"
#include "cb_algs.h"

#include <numeric>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

struct ccb
{
  example* shared;
  std::vector<example*> actions, decisions;
  uint32_t chosen_action_index;
  std::unordered_map<uint32_t, uint32_t> origin_index;
  CB::cb_class cb_label, default_cb_label;
  std::unordered_set<uint32_t> exclude_list, include_list;
  CCB::decision_scores_t decision_scores;
};

namespace CCB
{
static constexpr uint32_t SHARED_EX_INDEX = 0;
static constexpr uint32_t TOP_ACTION_INDEX = 0;

void clear_all(ccb& data)
{
  data.shared = nullptr;
  data.actions.clear();
  data.decisions.clear();
  data.chosen_action_index = 0;
  data.origin_index.clear();
  data.exclude_list.clear();
  data.include_list.clear();
  data.decision_scores.clear();
}

// split the decisions, the actions and the shared example from the multiline example
void split_multi_example(const multi_ex& examples, ccb& data)
{
  for (auto ex : examples)
  {
    switch (ex->l.conditional_contextual_bandit.type)
    {
      case CCB::example_type::shared:
        data.shared = ex;
        break;
      case CCB::example_type::action:
        data.actions.push_back(ex);
        break;
      case CCB::example_type::decision:
        data.decisions.push_back(ex);
        break;
      default:
        THROW("ccb_adf_explore: badly formatted example - invalid example type");
    }
  }
}

template <bool is_learn>
void sanity_checks(ccb& data)
{
  if (data.decisions.size() > data.actions.size())
    THROW("ccb_adf_explore: badly formatted example - number of actions "
        << data.actions.size() << " must be greater than the number of decisions " << data.decisions.size());

  if (is_learn)
  {
    for (auto decision : data.decisions)
    {
      if (decision->l.conditional_contextual_bandit.outcome != nullptr &&
          decision->l.conditional_contextual_bandit.outcome->probabilities.size() == 0)
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

void attach_label_to_first_action(conditional_contexual_bandit_outcome* outcome, ccb& data)
{
  // save the cb label
  data.cb_label.action = data.chosen_action_index;
  data.cb_label.probability = outcome->probabilities[0].score;
  data.cb_label.cost = outcome->cost;

  data.actions[0]->l.cb.costs.push_back(data.cb_label);
}

template <bool is_learn>
void save_action_scores(ccb& data)
{
  // save a copy
  auto copy = v_init<ACTION_SCORE::action_score>();
  copy_array(copy, data.shared->pred.a_s);
  data.decision_scores.push_back(copy);

  // correct indices: we want index relative to the original ccb multi-example, with no actions filtered
  for (auto& action_score : copy) action_score.action = data.origin_index[action_score.action];

  // exclude the chosen action from next decisions
  if (!is_learn)
    data.exclude_list.insert(copy[0].action);
  else
    data.exclude_list.insert(data.chosen_action_index);
}

void clear_pred_and_label(ccb& data)
{
  data.shared->pred.a_s.clear();
  data.actions[0]->l.cb.costs.clear();
}

// true if there exists at least 1 action in the cb multi-example
bool has_action(multi_ex& cb_ex) { return cb_ex.size() > 1; }

// build a cb example from the ccb example
template <bool is_learn>
void build_cb_example(multi_ex& cb_ex, example* decision, ccb& data)
{
  bool decision_has_label = decision->l.conditional_contextual_bandit.outcome != nullptr;

  // set the shared example in the cb multi-example
  //TODO merge decision features in shared feature + activate interactions
  cb_ex.push_back(data.shared);

  // retrieve the action index whitelist (if the list is empty, then all actions are white-listed)
  data.include_list.clear();
  for (uint32_t included_action_id : decision->l.conditional_contextual_bandit.explicit_included_actions)
    data.include_list.insert(included_action_id);

  // set the available actions in the cb multi-example
  uint32_t index = 0;
  data.origin_index.clear();
  for (size_t i = 0; i < data.actions.size(); i++)
  {
    // filter actions that are not explicitely included
    if (!data.include_list.empty() && data.include_list.find((uint32_t)i) == data.include_list.end())
      continue;

    // filter actions chosen by previous decisions
    if (data.exclude_list.find((uint32_t)i) != data.exclude_list.end())
      continue;

    // select the action
    cb_ex.push_back(data.actions[i]);

    // save the original index from the root multi-example
    data.origin_index[index++] = (uint32_t)i;

    // remember the index of the chosen action
    if (is_learn && decision_has_label &&
        i == decision->l.conditional_contextual_bandit.outcome->probabilities[0].action)
      data.chosen_action_index = (uint32_t)i;
  }

  if (is_learn && decision_has_label && has_action(cb_ex))
    attach_label_to_first_action(decision->l.conditional_contextual_bandit.outcome, data);
}

// iterate over decisions contained in the multi-example, and for each decision, build a cb example and perform a
// cb_explore_adf call.
template <bool is_learn>
void learn_or_predict(ccb& data, multi_learner& base, multi_ex& examples)
{
  clear_all(data);
  split_multi_example(examples, data);  // split shared, actions and decisions
  sanity_checks<is_learn>(data);
  create_cb_labels(data);

  // for each decision, re-build the cb example and call cb_explore_adf
  for (example* decision : data.decisions)
  {
    multi_ex cb_ex;
    build_cb_example<is_learn>(cb_ex, decision, data);

    if (has_action(cb_ex))
    {  // the cb example contains at least 1 action
      multiline_learn_or_predict<is_learn>(base, cb_ex, examples[0]->ft_offset);
      save_action_scores<is_learn>(data);
      clear_pred_and_label(data);
    }
    else
    {  // the cb example contains no action => cannot decide
      auto empty_action_scores = v_init<ACTION_SCORE::action_score>();
      data.decision_scores.push_back(empty_action_scores);
    }
  }

  delete_cb_labels(data);

  // save the predictions
  // TODO fix console print: this rewrite the polylabel and thus break the print stdout happening in cb.cc
  examples[0]->pred.decision_scores = data.decision_scores;
}

void print_decision_scores(int f, decision_scores_t& decision_scores)
{
  if (f >= 0)
  {
    std::stringstream ss;
    for (auto decision : decision_scores)
    {
      std::string delimiter = "";
      for (auto action_score : decision)
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

void print_update(vw& all, std::vector<example*>& decisions, decision_scores_t& decision_scores, size_t num_features)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    std::string label_str = "";
    std::string delim = "";
    int counter = 0;
    for (auto decision : decisions)
    {
      counter++;

      auto outcome = decision->l.conditional_contextual_bandit.outcome;
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
      if (counter > 1 && decisions.size() > 2)
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
    for (auto decision : decision_scores)
    {
      counter++;
      pred_str += delim;
      pred_str += std::to_string(decision[0].action);
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

  std::vector<example*> decisions;
  size_t num_features = 0;
  float loss = 0.;

  // Should this be done for shared, action and decision?
  for (auto ec : ec_seq)
  {
    num_features += ec->num_features;

    if (ec->l.conditional_contextual_bandit.type == CCB::example_type::decision)
    {
      decisions.push_back(ec);
    }
  }

  // What does it mean for not all of the decisions to be labeled? Does it become a non-labeled example at that point?
  // Is it hold out?
  bool labeled_example = true;
  auto preds = ec_seq[0]->pred.decision_scores;
  for (int i = 0; i < decisions.size(); i++)
  {
    auto outcome = decisions[i]->l.conditional_contextual_bandit.outcome;
    if (outcome != nullptr)
    {
      float l = CB_ALGS::get_unbiased_cost(
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

   CCB::print_update(all, decisions, preds, num_features);
}

void finish_multiline_example(vw& all, ccb& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, data, ec_seq);
    CB_ADF::global_print_newline(all);
  }
  VW::clear_seq_and_finish_examples(all, ec_seq);
}

void finish(ccb& data)
{
  data.actions.~vector<example*>();
  data.decisions.~vector<example*>();
  data.origin_index.~unordered_map<uint32_t, uint32_t>();
  data.exclude_list.~unordered_set<uint32_t>();
  data.include_list.~unordered_set<uint32_t>();
  data.cb_label.~cb_class();
  data.default_cb_label.~cb_class();
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
  data->decision_scores = v_init<ACTION_SCORE::action_scores>();
  data->default_cb_label = {FLT_MAX, 0, -1.f, 0.f};
  data->shared = nullptr;

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

  ld->type = static_cast<example_type>(read_object<uint8_t>(cache));
  read_count += sizeof(uint8_t);

  bool is_outcome_present = read_object<bool>(cache);
  read_count += sizeof(bool);

  if (is_outcome_present)
  {
    ld->outcome = new CCB::conditional_contexual_bandit_outcome();
    ld->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

    ld->outcome->cost = read_object<float>(cache);
    read_count += sizeof(float);
    auto size_probs = read_object<uint32_t>(cache);
    read_count += sizeof(uint32_t);

    for (uint32_t i = 0; i < size_probs; i++)
    {
      ld->outcome->probabilities.push_back(read_object<ACTION_SCORE::action_score>(cache));
      read_count += sizeof(ACTION_SCORE::action_score);
    }
  }

  auto size_includes = read_object<uint32_t>(cache);
  read_count += sizeof(uint32_t);

  for (uint32_t i = 0; i < size_includes; i++)
  {
    ld->explicit_included_actions.push_back(read_object<uint32_t>(cache));
    read_count += sizeof(uint32_t);
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
  c += sizeof(uint8_t);

  *(bool*)c = ld->outcome != nullptr;
  c += sizeof(bool);

  if (ld->outcome != nullptr)
  {
    *(float*)c = ld->outcome->cost;
    c += sizeof(float);

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
    c += sizeof(uint32_t);
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

  for (int i = 1; i < split_commas.size(); i++)
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
  else if (substring_equal(type, "decision"))
  {
    if (words.size() > 4)
      THROW("ccb decision label can only have a type cost and exclude list");
    ld->type = CCB::example_type::decision;

    // Skip the first two words "ccb <type>"
    for (auto i = 2; i < words.size(); i++)
    {
      auto is_outcome = std::find(words[i].begin, words[i].end, ':');
      if (is_outcome != words[i].end)
      {
        if (ld->outcome != nullptr)
        {
          THROW("There may be only 1 outcome associated with a decision.")
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
