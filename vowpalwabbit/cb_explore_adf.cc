#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"
#include "explore.h"
#include <vector>
#include <algorithm>

using namespace LEARNER;
using namespace ACTION_SCORE;
using namespace std;
using namespace CB_ALGS;
using namespace exploration;
using namespace VW::config;

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

// tau first
#define EXPLORE_FIRST 0
// epsilon greedy
#define EPS_GREEDY 1
// bagging explorer
#define BAG_EXPLORE 2
// softmax
#define SOFTMAX 3
// cover
#define COVER 4
// regcb
#define REGCB 5

#define B_SEARCH_MAX_ITER 20

namespace CB_EXPLORE_ADF
{
struct cb_explore_adf
{
  v_array<action_score> action_probs;
  vector<float> scores;
  vector<float> top_actions;

  size_t explore_type;

  size_t tau;
  float epsilon;
  size_t bag_size;
  size_t cover_size;
  float psi;
  bool nounif;
  float lambda;
  uint64_t offset;
  bool greedify;
  bool first_only;
  bool regcbopt;  // use optimistic variant of RegCB
  float c0;       // mellowness parameter for RegCB

  float min_cb_cost;
  float max_cb_cost;

  size_t counter;

  bool need_to_clear;
  vw* all;
  LEARNER::multi_learner* cs_ldf_learner;

  GEN_CS::cb_to_cs_adf gen_cs;
  COST_SENSITIVE::label cs_labels;
  v_array<CB::label> cb_labels;

  CB::label action_label;
  CB::label empty_label;

  COST_SENSITIVE::label cs_labels_2;

  v_array<COST_SENSITIVE::label> prepped_cs_labels;

  // for RegCB
  std::vector<float> min_costs;
  std::vector<float> max_costs;

  // for backing up cb example data when computing sensitivities
  std::vector<ACTION_SCORE::action_scores> ex_as;
  std::vector<v_array<CB::cb_class>> ex_costs;
};

template <class T>
void swap(T& ele1, T& ele2)
{
  T temp = ele2;
  ele2 = ele1;
  ele1 = temp;
}

// Validates a multiline example collection as a valid sequence for action dependent features format.
example* test_adf_sequence(multi_ex& ec_seq)
{
  if (ec_seq.size() == 0)
    THROW("cb_adf: At least one action must be provided for an example to be valid.");

  uint32_t count = 0;
  example* ret = nullptr;
  for (size_t k = 0; k < ec_seq.size(); k++)
  {
    example* ec = ec_seq[k];

    // Check if there is more than one cost for this example.
    if (ec->l.cb.costs.size() > 1)
      THROW("cb_adf: badly formatted example, only one cost can be known.");

    // Check whether the cost was initialized to a value.
    if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)
    {
      ret = ec;
      count += 1;
    }
  }

  if (count > 1)
    THROW("cb_adf: badly formatted example, only one line can have a cost");

  return ret;
}

// TODO: same as cs_active.cc, move to shared place
float binary_search(float fhat, float delta, float sens, float tol = 1e-6)
{
  const float maxw = min(fhat / sens, FLT_MAX);

  if (maxw * fhat * fhat <= delta)
    return maxw;

  float l = 0;
  float u = maxw;
  float w, v;

  for (int iter = 0; iter < B_SEARCH_MAX_ITER; iter++)
  {
    w = (u + l) / 2.f;
    v = w * (fhat * fhat - (fhat - sens * w) * (fhat - sens * w)) - delta;
    if (v > 0)
      u = w;
    else
      l = w;
    if (fabs(v) <= tol || u - l <= tol)
      break;
  }

  return l;
}

void get_cost_ranges(std::vector<float>& min_costs, std::vector<float>& max_costs, float delta, cb_explore_adf& data,
    multi_learner& base, multi_ex& examples, bool min_only)
{
  const size_t num_actions = examples[0]->pred.a_s.size();
  min_costs.resize(num_actions);
  max_costs.resize(num_actions);

  auto& ex_as = data.ex_as;
  auto& ex_costs = data.ex_costs;
  ex_as.clear();
  ex_costs.clear();

  // backup cb example data
  for (auto& ex : examples)
  {
    ex_as.push_back(ex->pred.a_s);
    ex_costs.push_back(ex->l.cb.costs);
  }

  // set regressor predictions
  for (const auto& as : ex_as[0])
  {
    examples[as.action]->pred.scalar = as.score;
  }

  const float cmin = data.min_cb_cost;
  const float cmax = data.max_cb_cost;

  for (size_t a = 0; a < num_actions; ++a)
  {
    example* ec = examples[a];
    ec->l.simple.label = cmin - 1;
    float sens = base.sensitivity(*ec);
    float w = 0;  // importance weight

    if (ec->pred.scalar < cmin || nanpattern(sens) || infpattern(sens))
      min_costs[a] = cmin;
    else
    {
      w = binary_search(ec->pred.scalar - cmin + 1, delta, sens);
      min_costs[a] = max(ec->pred.scalar - sens * w, cmin);
      if (min_costs[a] > cmax)
        min_costs[a] = cmax;
    }

    if (!min_only)
    {
      ec->l.simple.label = cmax + 1;
      sens = base.sensitivity(*ec);
      if (ec->pred.scalar > cmax || nanpattern(sens) || infpattern(sens))
      {
        max_costs[a] = cmax;
      }
      else
      {
        w = binary_search(cmax + 1 - ec->pred.scalar, delta, sens);
        max_costs[a] = min(ec->pred.scalar + sens * w, cmax);
        if (max_costs[a] < cmin)
          max_costs[a] = cmin;
      }
    }
  }

  // reset cb example data
  for (size_t i = 0; i < examples.size(); ++i)
  {
    examples[i]->pred.a_s = ex_as[i];
    examples[i]->l.cb.costs = ex_costs[i];
  }
}

size_t fill_tied(cb_explore_adf& /* data */, v_array<action_score>& preds)
{
  if (preds.size() == 0)
    return 0;
  size_t ret = 1;
  for (size_t i = 1; i < preds.size(); ++i)
    if (preds[i].score == preds[0].score)
      ++ret;
    else
      return ret;
  return ret;
}

template <bool is_learn>
void predict_or_learn_first(cb_explore_adf& data, multi_learner& base, multi_ex& examples)
{
  // Explore tau times, then act according to optimal.
  if (is_learn && data.gen_cs.known_cost.probability < 1 && test_adf_sequence(examples) != nullptr)
    multiline_learn_or_predict<true>(base, examples, data.offset);
  else
    multiline_learn_or_predict<true>(base, examples, data.offset);

  v_array<action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();

  if (data.tau)
  {
    float prob = 1.f / (float)num_actions;
    for (size_t i = 0; i < num_actions; i++) preds[i].score = prob;
    data.tau--;
  }
  else
  {
    for (size_t i = 1; i < num_actions; i++) preds[i].score = 0.;
    preds[0].score = 1.0;
  }

  enforce_minimum_probability(data.epsilon, true, begin_scores(preds), end_scores(preds));
}

template <bool is_learn>
void predict_or_learn_greedy(cb_explore_adf& data, multi_learner& base, multi_ex& examples)
{
  data.offset = examples[0]->ft_offset;
  // Explore uniform random an epsilon fraction of the time.

  if (is_learn && test_adf_sequence(examples) != nullptr)
    multiline_learn_or_predict<true>(base, examples, data.offset);
  else
    multiline_learn_or_predict<false>(base, examples, data.offset);
  action_scores& preds = examples[0]->pred.a_s;

  uint32_t num_actions = (uint32_t)preds.size();

  size_t tied_actions = fill_tied(data, preds);

  const float prob = data.epsilon / num_actions;
  for (size_t i = 0; i < num_actions; i++) preds[i].score = prob;
  if (!data.first_only)
  {
    for (size_t i = 0; i < tied_actions; ++i) preds[i].score += (1.f - data.epsilon) / tied_actions;
  }
  else
    preds[0].score += 1.f - data.epsilon;
}

template <bool is_learn>
void predict_or_learn_regcb(cb_explore_adf& data, multi_learner& base, multi_ex& examples)
{
  if (is_learn && test_adf_sequence(examples) != nullptr)
  {
    for (size_t i = 0; i < examples.size() - 1; ++i)
    {
      CB::label& ld = examples[i]->l.cb;
      if (ld.costs.size() == 1)
        ld.costs[0].probability = 1.f;  // no importance weighting
    }

    multiline_learn_or_predict<true>(base, examples, data.offset);
  }
  else
    multiline_learn_or_predict<false>(base, examples, data.offset);

  v_array<action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();
  ++data.counter;

  const float max_range = data.max_cb_cost - data.min_cb_cost;
  // threshold on empirical loss difference
  const float delta = data.c0 * log((float)(num_actions * data.counter)) * pow(max_range, 2);

  if (!is_learn)
  {
    get_cost_ranges(data.min_costs, data.max_costs, delta, data, base, examples,
        /*min_only=*/data.regcbopt);

    if (data.regcbopt)  // optimistic variant
    {
      float min_cost = FLT_MAX;
      size_t a_opt = 0;  // optimistic action
      for (size_t a = 0; a < num_actions; ++a)
      {
        if (data.min_costs[a] < min_cost)
        {
          min_cost = data.min_costs[a];
          a_opt = a;
        }
      }
      for (size_t i = 0; i < preds.size(); ++i)
      {
        if (preds[i].action == a_opt || (!data.first_only && data.min_costs[preds[i].action] == min_cost))
          preds[i].score = 1;
        else
          preds[i].score = 0;
      }
    }
    else  // elimination variant
    {
      float min_max_cost = FLT_MAX;
      for (size_t a = 0; a < num_actions; ++a)
        if (data.max_costs[a] < min_max_cost)
          min_max_cost = data.max_costs[a];
      for (size_t i = 0; i < preds.size(); ++i)
      {
        if (data.min_costs[preds[i].action] <= min_max_cost)
          preds[i].score = 1;
        else
          preds[i].score = 0;
        // explore uniformly on support
        enforce_minimum_probability(1.0, /*update_zero_elements=*/false, begin_scores(preds), end_scores(preds));
      }
    }
  }
}

void do_sort(cb_explore_adf& data)
{
  // We want to preserve the score order in the returned action_probs if possible.  To do this,
  // sort top_actions and data.action_probs by the order induced in data.scores.
  sort(data.action_probs.begin(), data.action_probs.end(), [data](action_score as1, action_score as2) {
    if (as1.score > as2.score)
      return true;
    else if (as1.score < as2.score)
      return false;
    // equal probabilities
    if (data.scores[as1.action] < data.scores[as2.action])
      return true;
    else if (data.scores[as1.action] > data.scores[as2.action])
      return false;
    // equal probabilities and equal cost estimates
    return as1.action < as2.action;
  });
}

template <bool is_learn>
void predict_or_learn_bag(cb_explore_adf& data, multi_learner& base, multi_ex& examples)
{
  // Randomize over predictions from a base set of predictors
  v_array<action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)examples.size();
  if (num_actions == 0)
  {
    preds.clear();
    return;
  }

  data.scores.clear();
  for (uint32_t i = 0; i < num_actions; i++) data.scores.push_back(0.f);
  vector<float>& top_actions = data.top_actions;
  top_actions.assign(num_actions, 0);
  bool test_sequence = test_adf_sequence(examples) == nullptr;
  for (uint32_t i = 0; i < data.bag_size; i++)
  {
    // avoid updates to the random num generator
    // for greedify, always update first policy once
    uint32_t count = is_learn ? ((data.greedify && i == 0) ? 1 : BS::weight_gen(*data.all)) : 0;

    if (is_learn && count > 0 && !test_sequence)
      multiline_learn_or_predict<true>(base, examples, data.offset, i);
    else
      multiline_learn_or_predict<false>(base, examples, data.offset, i);

    assert(preds.size() == num_actions);
    for (auto e : preds) data.scores[e.action] += e.score;

    if (!data.first_only)
    {
      size_t tied_actions = fill_tied(data, preds);
      for (size_t i = 0; i < tied_actions; ++i) top_actions[preds[i].action] += 1.f / tied_actions;
    }
    else
      top_actions[preds[0].action] += 1.f;
    if (is_learn && !test_sequence)
      for (uint32_t j = 1; j < count; j++) multiline_learn_or_predict<true>(base, examples, data.offset, i);
  }

  data.action_probs.clear();
  for (uint32_t i = 0; i < data.scores.size(); i++) data.action_probs.push_back({i, 0.});

  // generate distribution over actions
  generate_bag(
      begin(data.top_actions), end(data.top_actions), begin_scores(data.action_probs), end_scores(data.action_probs));

  enforce_minimum_probability(data.epsilon, true, begin_scores(data.action_probs), end_scores(data.action_probs));

  do_sort(data);

  for (size_t i = 0; i < num_actions; i++) preds[i] = data.action_probs[i];
}

template <bool is_learn>
void predict_or_learn_cover(cb_explore_adf& data, multi_learner& base, multi_ex& examples)
{
  // Randomize over predictions from a base set of predictors
  // Use cost sensitive oracle to cover actions to form distribution.
  const bool is_mtr = data.gen_cs.cb_type == CB_TYPE_MTR;
  if (is_learn)
  {
    if (is_mtr)  // use DR estimates for non-ERM policies in MTR
      GEN_CS::gen_cs_example_dr<true>(data.gen_cs, examples, data.cs_labels);
    else
      GEN_CS::gen_cs_example<false>(data.gen_cs, examples, data.cs_labels);
    multiline_learn_or_predict<true>(base, examples, data.offset);
  }
  else
  {
    GEN_CS::gen_cs_example_ips(examples, data.cs_labels);
    multiline_learn_or_predict<false>(base, examples, data.offset);
  }

  v_array<action_score>& preds = examples[0]->pred.a_s;
  const uint32_t num_actions = (uint32_t)preds.size();

  float additive_probability = 1.f / (float)data.cover_size;
  const float min_prob = min(1.f / num_actions, 1.f / (float)sqrt(data.counter * num_actions));
  v_array<action_score>& probs = data.action_probs;
  probs.clear();
  for (uint32_t i = 0; i < num_actions; i++) probs.push_back({i, 0.});
  data.scores.clear();
  for (uint32_t i = 0; i < num_actions; i++) data.scores.push_back(preds[i].score);

  if (!data.first_only)
  {
    size_t tied_actions = fill_tied(data, preds);
    for (size_t i = 0; i < tied_actions; ++i) probs[preds[i].action].score += additive_probability / tied_actions;
  }
  else
    probs[preds[0].action].score += additive_probability;

  float norm = min_prob * num_actions + (additive_probability - min_prob);
  for (size_t i = 1; i < data.cover_size; i++)
  {
    // Create costs of each action based on online cover
    if (is_learn)
    {
      data.cs_labels_2.costs.clear();
      for (uint32_t j = 0; j < num_actions; j++)
      {
        float pseudo_cost = data.cs_labels.costs[j].x - data.psi * min_prob / (max(probs[j].score, min_prob) / norm);
        data.cs_labels_2.costs.push_back({pseudo_cost, j, 0., 0.});
      }
      GEN_CS::call_cs_ldf<true>(*(data.cs_ldf_learner), examples, data.cb_labels, data.cs_labels_2,
          data.prepped_cs_labels, data.offset, i + 1);
    }
    else
      GEN_CS::call_cs_ldf<false>(
          *(data.cs_ldf_learner), examples, data.cb_labels, data.cs_labels, data.prepped_cs_labels, data.offset, i + 1);

    for (uint32_t i = 0; i < num_actions; i++) data.scores[i] += preds[i].score;
    if (!data.first_only)
    {
      size_t tied_actions = fill_tied(data, preds);
      const float add_prob = additive_probability / tied_actions;
      for (size_t i = 0; i < tied_actions; ++i)
      {
        if (probs[preds[i].action].score < min_prob)
          norm += max(0, add_prob - (min_prob - probs[preds[i].action].score));
        else
          norm += add_prob;
        probs[preds[i].action].score += add_prob;
      }
    }
    else
    {
      uint32_t action = preds[0].action;
      if (probs[action].score < min_prob)
        norm += max(0, additive_probability - (min_prob - probs[action].score));
      else
        norm += additive_probability;
      probs[action].score += additive_probability;
    }
  }

  enforce_minimum_probability(min_prob * num_actions, !data.nounif, begin_scores(probs), end_scores(probs));

  do_sort(data);
  for (size_t i = 0; i < num_actions; i++) preds[i] = probs[i];

  ++data.counter;
}

template <bool is_learn>
void predict_or_learn_softmax(cb_explore_adf& data, multi_learner& base, multi_ex& examples)
{
  if (is_learn && test_adf_sequence(examples) != nullptr)
    multiline_learn_or_predict<true>(base, examples, data.offset);
  else
    multiline_learn_or_predict<false>(base, examples, data.offset);

  v_array<action_score>& preds = examples[0]->pred.a_s;
  generate_softmax(-data.lambda, begin_scores(preds), end_scores(preds), begin_scores(preds), end_scores(preds));

  enforce_minimum_probability(data.epsilon, true, begin_scores(preds), end_scores(preds));
}

void finish(cb_explore_adf& data)
{
  data.top_actions.~vector<float>();
  data.scores.~vector<float>();
  data.min_costs.~vector<float>();
  data.max_costs.~vector<float>();
  data.ex_as.~vector<action_scores>();
  data.ex_costs.~vector<v_array<CB::cb_class>>();
  data.action_probs.delete_v();
  data.cs_labels.costs.delete_v();
  data.cs_labels_2.costs.delete_v();
  data.cb_labels.delete_v();
  for (size_t i = 0; i < data.prepped_cs_labels.size(); i++) data.prepped_cs_labels[i].costs.delete_v();
  data.prepped_cs_labels.delete_v();
  data.gen_cs.pred_scores.costs.delete_v();
  data.gen_cs.mtr_ec_seq.~multi_ex();
}

// Semantics: Currently we compute the IPS loss no matter what flags
// are specified. We print the first action and probability, based on
// ordering by scores in the final output.

void output_example(vw& all, cb_explore_adf& c, multi_ex& ec_seq)
{
  if (ec_seq.size() <= 0)
    return;

  size_t num_features = 0;

  float loss = 0.;

  auto& ec = *ec_seq[0];
  ACTION_SCORE::action_scores preds = ec.pred.a_s;

  for (const auto& example : ec_seq)
  {
    num_features += example->num_features;
  }

  bool labeled_example = true;
  if (c.gen_cs.known_cost.probability > 0)
  {
    for (uint32_t i = 0; i < preds.size(); i++)
    {
      float l = get_unbiased_cost(&c.gen_cs.known_cost, preds[i].action);
      loss += l * preds[i].score;
    }
  }
  else
    labeled_example = false;

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) holdout_example &= ec_seq[i]->test_only;

  all.sd->update(holdout_example, labeled_example, loss, ec.weight, num_features);

  for (int sink : all.final_prediction_sink) print_action_score(sink, ec.pred.a_s, ec.tag);

  if (all.raw_prediction > 0)
  {
    string outputString;
    stringstream outputStringStream(outputString);
    v_array<CB::cb_class> costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, !labeled_example, ec, &ec_seq, true);
}

void output_example_seq(vw& all, cb_explore_adf& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, data, ec_seq);
    if (all.raw_prediction > 0)
      all.print_text(all.raw_prediction, "", ec_seq[0]->tag);
  }
}

void finish_multiline_example(vw& all, cb_explore_adf& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example_seq(all, data, ec_seq);
    CB_ADF::global_print_newline(all);
  }

  for (auto x : ec_seq)
  {
    x->l.cb.costs.clear();
  }

  VW::clear_seq_and_finish_examples(all, ec_seq);
}

template <bool is_learn>
void do_actual_learning(cb_explore_adf& data, multi_learner& base, multi_ex& ec_seq)
{
  example* label_example = test_adf_sequence(ec_seq);
  data.gen_cs.known_cost = CB_ADF::get_observed_cost(ec_seq);

  if (label_example == nullptr || !is_learn)
  {
    if (label_example != nullptr)  // extract label
    {
      data.action_label = label_example->l.cb;
      label_example->l.cb = data.empty_label;
    }
    switch (data.explore_type)
    {
      case EXPLORE_FIRST:
        predict_or_learn_first<false>(data, base, ec_seq);
        break;
      case EPS_GREEDY:
        predict_or_learn_greedy<false>(data, base, ec_seq);
        break;
      case SOFTMAX:
        predict_or_learn_softmax<false>(data, base, ec_seq);
        break;
      case BAG_EXPLORE:
        predict_or_learn_bag<false>(data, base, ec_seq);
        break;
      case COVER:
        predict_or_learn_cover<false>(data, base, ec_seq);
        break;
      case REGCB:
        predict_or_learn_regcb<false>(data, base, ec_seq);
        break;
      default:
        THROW("Unknown explorer type specified for contextual bandit learning: " << data.explore_type);
    }
    if (label_example != nullptr)  // restore label
      label_example->l.cb = data.action_label;
  }
  else
  {
    /*	v_array<float> temp_probs;
    temp_probs = v_init<float>();
    do_actual_learning<false>(data,base);
    for (size_t i = 0; i < data.ec_seq[0]->pred.a_s.size(); i++)
    temp_probs.push_back(data.ec_seq[0]->pred.a_s[i].score);*/

    switch (data.explore_type)
    {
      case EXPLORE_FIRST:
        predict_or_learn_first<is_learn>(data, base, ec_seq);
        break;
      case EPS_GREEDY:
        predict_or_learn_greedy<is_learn>(data, base, ec_seq);
        break;
      case SOFTMAX:
        predict_or_learn_softmax<is_learn>(data, base, ec_seq);
        break;
      case BAG_EXPLORE:
        predict_or_learn_bag<is_learn>(data, base, ec_seq);
        break;
      case COVER:
        predict_or_learn_cover<is_learn>(data, base, ec_seq);
        break;
      case REGCB:
        predict_or_learn_regcb<is_learn>(data, base, ec_seq);
        break;
      default:
        THROW("Unknown explorer type specified for contextual bandit learning: " << data.explore_type);
    }
  }
}
}  // namespace CB_EXPLORE_ADF

using namespace CB_EXPLORE_ADF;

base_learner* cb_explore_adf_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<cb_explore_adf>();
  bool cb_explore_adf_option = false;
  bool softmax = false;
  bool regcb = false;
  std::string type_string = "mtr";
  option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("first", data->tau).keep().help("tau-first exploration"))
      .add(make_option("epsilon", data->epsilon).keep().help("epsilon-greedy exploration"))
      .add(make_option("bag", data->bag_size).keep().help("bagging-based exploration"))
      .add(make_option("cover", data->cover_size).keep().help("Online cover based exploration"))
      .add(make_option("psi", data->psi).keep().default_value(1.0f).help("disagreement parameter for cover"))
      .add(make_option("nounif", data->nounif)
               .keep()
               .help("do not explore uniformly on zero-probability actions in cover"))
      .add(make_option("softmax", softmax).keep().help("softmax exploration"))
      .add(make_option("regcb", regcb).keep().help("RegCB-elim exploration"))
      .add(make_option("regcbopt", data->regcbopt).keep().help("RegCB optimistic exploration"))
      .add(make_option("mellowness", data->c0)
               .keep()
               .default_value(0.1f)
               .help("RegCB mellowness parameter c_0. Default 0.1"))
      .add(make_option("greedify", data->greedify).keep().help("always update first policy once in bagging"))
      .add(make_option("cb_min_cost", data->min_cb_cost).keep().default_value(0.f).help("lower bound on cost"))
      .add(make_option("cb_max_cost", data->max_cb_cost).keep().default_value(1.f).help("upper bound on cost"))
      .add(make_option("first_only", data->first_only)
               .keep()
               .help("Only explore the first action in a tie-breaking event"))
      .add(make_option("lambda", data->lambda).keep().default_value(1.f).help("parameter for softmax"))
      .add(make_option("cb_type", type_string)
               .keep()
               .help("contextual bandit method to use in {ips,dr,mtr}. Default: mtr"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option)
    return nullptr;

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  data->all = &all;
  if (data->lambda < 0)  // Lambda should always be negative because we are using a cost basis.
    data->lambda = -data->lambda;

  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = delete_action_scores;

  size_t problem_multiplier = 1;

  if (options.was_supplied("cover"))
  {
    data->explore_type = COVER;
    problem_multiplier = data->cover_size + 1;
  }
  else if (options.was_supplied("bag"))
  {
    data->explore_type = BAG_EXPLORE;
    problem_multiplier = data->bag_size;
  }
  else if (options.was_supplied("first"))
    data->explore_type = EXPLORE_FIRST;
  else if (softmax)
    data->explore_type = SOFTMAX;
  else if (regcb || (options.was_supplied("regcbopt") && data->regcbopt))
    data->explore_type = REGCB;
  else
  {
    if (!options.was_supplied("epsilon"))
      data->epsilon = 0.05f;
    data->explore_type = EPS_GREEDY;
  }

  data->gen_cs.cb_type = CB_TYPE_IPS;
  if (options.was_supplied("cb_type"))
  {
    if (type_string.compare("dr") == 0)
      data->gen_cs.cb_type = CB_TYPE_DR;
    else if (type_string.compare("ips") == 0)
      data->gen_cs.cb_type = CB_TYPE_IPS;
    else if (type_string.compare("mtr") == 0)
    {
      if (options.was_supplied("cover"))
        all.trace_message << "warning: currently, mtr is only used for the first policy in cover, other policies use dr"
                          << endl;
      data->gen_cs.cb_type = CB_TYPE_MTR;
    }
    else
    {
      all.trace_message << "warning: cb_type must be in {'ips','dr','mtr'}; resetting to mtr." << std::endl;
      options.replace("cb_type", "mtr");
    }

    if (data->explore_type == REGCB && data->gen_cs.cb_type != CB_TYPE_MTR)
      {
        all.trace_message << "warning: bad cb_type, RegCB only supports mtr; resetting to mtr." << std::endl;
        options.replace("cb_type", "mtr");
      }
  }

  multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  // Extract from lower level reductions.
  data->gen_cs.scorer = all.scorer;
  data->cs_ldf_learner = as_multiline(all.cost_sensitive);

  learner<cb_explore_adf, multi_ex>& l = init_learner(data, base, CB_EXPLORE_ADF::do_actual_learning<true>,
      CB_EXPLORE_ADF::do_actual_learning<false>, problem_multiplier, prediction_type::action_probs);

  l.set_finish_example(CB_EXPLORE_ADF::finish_multiline_example);
  l.set_finish(CB_EXPLORE_ADF::finish);
  return make_base(l);
}
