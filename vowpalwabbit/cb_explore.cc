#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "explore.h"
#include <memory>

using namespace LEARNER;
using namespace ACTION_SCORE;
using namespace GEN_CS;
using namespace CB_ALGS;
using namespace exploration;
using namespace VW::config;
// All exploration algorithms return a vector of probabilities, to be used by GenericExplorer downstream

namespace CB_EXPLORE
{
struct cb_explore
{
  std::shared_ptr<rand_state> _random_state;
  cb_to_cs cbcs;
  v_array<uint32_t> preds;
  v_array<float> cover_probs;

  CB::label cb_label;
  COST_SENSITIVE::label cs_label;
  COST_SENSITIVE::label second_cs_label;

  learner<cb_explore, example>* cs;

  size_t tau;
  float epsilon;
  size_t bag_size;
  size_t cover_size;
  float psi;

  size_t counter;

  ~cb_explore()
  {
    preds.delete_v();
    cover_probs.delete_v();
    COST_SENSITIVE::cs_label.delete_label(&cbcs.pred_scores);
    COST_SENSITIVE::cs_label.delete_label(&cs_label);
    COST_SENSITIVE::cs_label.delete_label(&second_cs_label);
  }
};

template <bool is_learn>
void predict_or_learn_first(cb_explore& data, single_learner& base, example& ec)
{
  // Explore tau times, then act according to optimal.
  action_scores probs = ec.pred.a_s;

  if (is_learn && ec.l.cb.costs[0].probability < 1)
    base.learn(ec);
  else
    base.predict(ec);

  probs.clear();
  if (data.tau > 0)
  {
    float prob = 1.f / (float)data.cbcs.num_actions;
    for (uint32_t i = 0; i < data.cbcs.num_actions; i++) probs.push_back({i, prob});
    data.tau--;
  }
  else
  {
    uint32_t chosen = ec.pred.multiclass - 1;
    for (uint32_t i = 0; i < data.cbcs.num_actions; i++) probs.push_back({i, 0.});
    probs[chosen].score = 1.0;
  }

  ec.pred.a_s = probs;
}

template <bool is_learn>
void predict_or_learn_greedy(cb_explore& data, single_learner& base, example& ec)
{
  // Explore uniform random an epsilon fraction of the time.
  // TODO: pointers are copied here. What happens if base.learn/base.predict re-allocs?
  // ec.pred.a_s = probs; will restore the than free'd memory
  action_scores probs = ec.pred.a_s;
  probs.clear();

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  // pre-allocate pdf
  probs.resize(data.cbcs.num_actions);
  for (uint32_t i = 0; i < data.cbcs.num_actions; i++) probs.push_back({i, 0});
  generate_epsilon_greedy(data.epsilon, ec.pred.multiclass - 1, begin_scores(probs), end_scores(probs));

  ec.pred.a_s = probs;
}

template <bool is_learn>
void predict_or_learn_bag(cb_explore& data, single_learner& base, example& ec)
{
  // Randomize over predictions from a base set of predictors
  action_scores probs = ec.pred.a_s;
  probs.clear();

  for (uint32_t i = 0; i < data.cbcs.num_actions; i++) probs.push_back({i, 0.});
  float prob = 1.f / (float)data.bag_size;
  for (size_t i = 0; i < data.bag_size; i++)
  {
    uint32_t count = BS::weight_gen(data._random_state);
    if (is_learn && count > 0)
      base.learn(ec, i);
    else
      base.predict(ec, i);
    uint32_t chosen = ec.pred.multiclass - 1;
    probs[chosen].score += prob;
    if (is_learn)
      for (uint32_t j = 1; j < count; j++) base.learn(ec, i);
  }

  ec.pred.a_s = probs;
}

void get_cover_probabilities(cb_explore& data, single_learner& /* base */, example& ec, v_array<action_score>& probs)
{
  float additive_probability = 1.f / (float)data.cover_size;
  data.preds.clear();

  for (uint32_t i = 0; i < data.cbcs.num_actions; i++) probs.push_back({i, 0.});

  for (size_t i = 0; i < data.cover_size; i++)
  {
    // get predicted cost-sensitive predictions
    if (i == 0)
      data.cs->predict(ec, i);
    else
      data.cs->predict(ec, i + 1);
    uint32_t pred = ec.pred.multiclass;
    probs[pred - 1].score += additive_probability;
    data.preds.push_back((uint32_t)pred);
  }
  uint32_t num_actions = data.cbcs.num_actions;

  float min_prob = std::min(1.f / num_actions, 1.f / (float)std::sqrt(data.counter * num_actions));

  enforce_minimum_probability(min_prob * num_actions, false, begin_scores(probs), end_scores(probs));

  data.counter++;
}

template <bool is_learn>
void predict_or_learn_cover(cb_explore& data, single_learner& base, example& ec)
{
  // Randomize over predictions from a base set of predictors
  // Use cost sensitive oracle to cover actions to form distribution.

  uint32_t num_actions = data.cbcs.num_actions;

  action_scores probs = ec.pred.a_s;
  probs.clear();
  data.cs_label.costs.clear();

  for (uint32_t j = 0; j < num_actions; j++) data.cs_label.costs.push_back({FLT_MAX, j + 1, 0., 0.});

  size_t cover_size = data.cover_size;
  size_t counter = data.counter;
  v_array<float>& probabilities = data.cover_probs;
  v_array<uint32_t>& predictions = data.preds;

  float additive_probability = 1.f / (float)cover_size;

  float min_prob = std::min(1.f / num_actions, 1.f / (float)std::sqrt(counter * num_actions));

  data.cb_label = ec.l.cb;

  ec.l.cs = data.cs_label;
  get_cover_probabilities(data, base, ec, probs);

  if (is_learn)
  {
    ec.l.cb = data.cb_label;
    base.learn(ec);

    // Now update oracles

    // 1. Compute loss vector
    data.cs_label.costs.clear();
    float norm = min_prob * num_actions;
    ec.l.cb = data.cb_label;
    data.cbcs.known_cost = get_observed_cost(data.cb_label);
    gen_cs_example<false>(data.cbcs, ec, data.cb_label, data.cs_label);
    for (uint32_t i = 0; i < num_actions; i++) probabilities[i] = 0;

    ec.l.cs = data.second_cs_label;
    // 2. Update functions
    for (size_t i = 0; i < cover_size; i++)
    {
      // Create costs of each action based on online cover
      for (uint32_t j = 0; j < num_actions; j++)
      {
        float pseudo_cost =
            data.cs_label.costs[j].x - data.psi * min_prob / (std::max(probabilities[j], min_prob) / norm) + 1;
        data.second_cs_label.costs[j].class_index = j + 1;
        data.second_cs_label.costs[j].x = pseudo_cost;
      }
      if (i != 0)
        data.cs->learn(ec, i + 1);
      if (probabilities[predictions[i] - 1] < min_prob)
        norm += std::max(0.f, additive_probability - (min_prob - probabilities[predictions[i] - 1]));
      else
        norm += additive_probability;
      probabilities[predictions[i] - 1] += additive_probability;
    }
  }

  ec.l.cb = data.cb_label;
  ec.pred.a_s = probs;
}

void print_update_cb_explore(vw& all, bool is_test, example& ec, std::stringstream& pred_string)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    std::stringstream label_string;
    if (is_test)
      label_string << " unknown";
    else
      label_string << ec.l.cb.costs[0].action;
    all.sd->print_update(all.holdout_set_off, all.current_pass, label_string.str(), pred_string.str(), ec.num_features,
        all.progress_add, all.progress_arg);
  }
}

void output_example(vw& all, cb_explore& data, example& ec, CB::label& ld)
{
  float loss = 0.;

  cb_to_cs& c = data.cbcs;

  if ((c.known_cost = get_observed_cost(ld)) != nullptr)
    for (uint32_t i = 0; i < ec.pred.a_s.size(); i++)
      loss += get_cost_estimate(c.known_cost, c.pred_scores, i + 1) * ec.pred.a_s[i].score;

  all.sd->update(ec.test_only, get_observed_cost(ld) != nullptr, loss, 1.f, ec.num_features);

  std::stringstream ss;
  float maxprob = 0.;
  uint32_t maxid = 0;
  for (uint32_t i = 0; i < ec.pred.a_s.size(); i++)
  {
    ss << std::fixed << ec.pred.a_s[i].score << " ";
    if (ec.pred.a_s[i].score > maxprob)
    {
      maxprob = ec.pred.a_s[i].score;
      maxid = i + 1;
    }
  }
  for (int sink : all.final_prediction_sink) all.print_text(sink, ss.str(), ec.tag);

  std::stringstream sso;
  sso << maxid << ":" << std::fixed << maxprob;
  print_update_cb_explore(all, CB::cb_label.test_label(&ld), ec, sso);
}

void finish_example(vw& all, cb_explore& c, example& ec)
{
  output_example(all, c, ec, ec.l.cb);
  VW::finish_example(all, ec);
}
}  // namespace CB_EXPLORE
using namespace CB_EXPLORE;

base_learner* cb_explore_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<cb_explore>();
  option_group_definition new_options("Contextual Bandit Exploration");
  new_options
      .add(make_option("cb_explore", data->cbcs.num_actions)
               .keep()
               .help("Online explore-exploit for a <k> action contextual bandit problem"))
      .add(make_option("first", data->tau).keep().help("tau-first exploration"))
      .add(make_option("epsilon", data->epsilon).keep().default_value(0.05f).help("epsilon-greedy exploration"))
      .add(make_option("bag", data->bag_size).keep().help("bagging-based exploration"))
      .add(make_option("cover", data->cover_size).keep().help("Online cover based exploration"))
      .add(make_option("psi", data->psi).keep().default_value(1.0f).help("disagreement parameter for cover"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("cb_explore"))
    return nullptr;

  data->_random_state = all.get_random_state();
  uint32_t num_actions = data->cbcs.num_actions;

  if (!options.was_supplied("cb"))
  {
    std::stringstream ss;
    ss << data->cbcs.num_actions;
    options.insert("cb", ss.str());
  }

  all.delete_prediction = delete_action_scores;
  data->cbcs.cb_type = CB_TYPE_DR;

  single_learner* base = as_singleline(setup_base(options, all));
  data->cbcs.scorer = all.scorer;

  learner<cb_explore, example>* l;
  if (options.was_supplied("cover"))
  {
    data->cs = (learner<cb_explore, example>*)(as_singleline(all.cost_sensitive));
    data->second_cs_label.costs.resize(num_actions);
    data->second_cs_label.costs.end() = data->second_cs_label.costs.begin() + num_actions;
    data->cover_probs = v_init<float>();
    data->cover_probs.resize(num_actions);
    data->preds = v_init<uint32_t>();
    data->preds.resize(data->cover_size);
    l = &init_learner(data, base, predict_or_learn_cover<true>, predict_or_learn_cover<false>, data->cover_size + 1,
        prediction_type::action_probs);
  }
  else if (options.was_supplied("bag"))
    l = &init_learner(data, base, predict_or_learn_bag<true>, predict_or_learn_bag<false>, data->bag_size,
        prediction_type::action_probs);
  else if (options.was_supplied("first"))
    l = &init_learner(
        data, base, predict_or_learn_first<true>, predict_or_learn_first<false>, 1, prediction_type::action_probs);
  else  // greedy
    l = &init_learner(
        data, base, predict_or_learn_greedy<true>, predict_or_learn_greedy<false>, 1, prediction_type::action_probs);

  l->set_finish_example(finish_example);
  return make_base(*l);
}
