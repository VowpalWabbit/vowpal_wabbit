// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "simulator.h"

#include "vw/config/options_cli.h"
#include "vw/core/vw.h"

#include <fmt/format.h>

#include <numeric>

namespace simulator
{
cb_sim_gf_filtering::cb_sim_gf_filtering(
    bool is_graph, uint64_t seed, bool use_default_ns, std::vector<std::string> actions)
    : cb_sim(seed, use_default_ns, actions), is_graph(is_graph)
{
}

float cb_sim_gf_filtering::get_reaction(
    const std::map<std::string, std::string>& context, const std::string& action, bool, bool, float)
{
  float reward = 0.f;
  if (action == "spam")
  {
    reward = MARKED_AS_SPAM;

    if (context.at("user") == "Tom")
    {
      // Tom gets a lot of spam in the evenings
      if (context.at("time_of_day") == "morning") { not_spam_classified_as_spam++; }
      if (context.at("time_of_day") == "afternoon") { spam_classified_as_spam++; }
    }
    else if (context.at("user") == "Anna")
    {
      // Anna gets a lot of spam in the mornings
      if (context.at("time_of_day") == "morning") { spam_classified_as_spam++; }
      if (context.at("time_of_day") == "afternoon") { not_spam_classified_as_spam++; }
    }
  }
  else
  {
    // action is not_spam

    if (context.at("user") == "Tom")
    {
      // Tom gets a lot of spam in the evenings
      if (context.at("time_of_day") == "morning")
      {
        reward = NOT_SPAM_CATEGORIZED_AS_NOT_SPAM;
        not_spam_classified_as_not_spam++;
      }
      else if (context.at("time_of_day") == "afternoon")
      {
        reward = SPAM_CATEGORIZED_AS_NOT_SPAM;
        spam_classified_as_not_spam++;
      }
    }
    else if (context.at("user") == "Anna")
    {
      // Anna gets a lot of spam in the mornings
      if (context.at("time_of_day") == "morning")
      {
        reward = SPAM_CATEGORIZED_AS_NOT_SPAM;
        spam_classified_as_not_spam++;
      }
      else if (context.at("time_of_day") == "afternoon")
      {
        reward = NOT_SPAM_CATEGORIZED_AS_NOT_SPAM;
        not_spam_classified_as_not_spam++;
      }
    }
  }

  return reward;
}

std::vector<std::string> cb_sim_gf_filtering::to_vw_example_format(
    const std::map<std::string, std::string>& context, const std::string& chosen_action, float cost, float prob)
{
  /**
   * ------ set the cost on both actions ------
   *
   * spam is action 0
   * not spam is action 1
   * if something is categorized as spam we never get here or we get here with an empty chosen action (i.e. predict and
   * we don't care about the label)
   *
   * so if we are setting the label the example has been categorized (correctly or not) as not_spam (i.e. action 1)
   *
   * in that case the cost of that action should be set as-is, and the cost of the opposite action
   * (i.e. action 0) should be set as an opposite cost. So if action 1 was categorized correctly as not spam that means
   * we want to learn that these features get a low cost for action 1 but the same features for the opposite action
   * (action 0) should get a high cost, and vice versa
   *
   *
   * all label (a:c:p) triplets should have the chosen action in the "a", their own cost in "c", and the chosen
   * probability at "p"
   */

  std::vector<std::string> multi_ex_str;
  multi_ex_str.push_back(fmt::format(
      "shared {} |{} user={} time_of_day={}", graph, user_ns, context.at("user"), context.at("time_of_day")));
  for (size_t action_index = 0; action_index < actions.size(); action_index++)
  {
    const auto& action = actions[action_index];
    std::ostringstream ex;
    if (!chosen_action.empty())
    {
      if (action == chosen_action) { ex << fmt::format("{}:{}:{} ", action_index, cost, prob); }
      else if (is_graph)
      {
        float cost_of_categorizing_as_spam = 0.f;
        if (cost == NOT_SPAM_CATEGORIZED_AS_NOT_SPAM)
        {
          // this not a spam message, if we had categorized it as spam (action 0) this would have been not great
          cost_of_categorizing_as_spam = NOT_SPAM_CATEGORIZED_AS_SPAM;
        }
        if (cost == SPAM_CATEGORIZED_AS_NOT_SPAM)
        {
          // this is a spam message, if we had categorized it as spam that would be great
          cost_of_categorizing_as_spam = SPAM_CATEGORIZED_AS_SPAM;
        }
        ex << fmt::format("{}:{}:{} ", action_index, cost_of_categorizing_as_spam, prob);
      }
    }
    ex << fmt::format("|{} article={}", action_ns, action);
    multi_ex_str.push_back(ex.str());
  }
  return multi_ex_str;
}

cb_sim::cb_sim(uint64_t seed, bool use_default_ns, std::vector<std::string> actions)
    : users({"Tom", "Anna"})
    , times_of_day({"morning", "afternoon"})
    // , actions({"politics", "sports", "music", "food", "finance", "health", "camping"})
    , actions(actions)
    , user_ns("User")
    , action_ns(use_default_ns ? "" : "Action")
{
  random_state.set_random_state(seed);
  callback_count = 0;
}

float cb_sim::get_reaction(const std::map<std::string, std::string>& context, const std::string& action, bool add_noise,
    bool swap_reward, float scale_reward)
{
  float like_reward = USER_LIKED_ARTICLE;
  float dislike_reward = USER_DISLIKED_ARTICLE;
  if (add_noise)
  {
    like_reward += random_state.get_and_update_random();
    dislike_reward += random_state.get_and_update_random();
  }

  float reward = dislike_reward;
  if (context.at("user") == "Tom")
  {
    if (context.at("time_of_day") == "morning" && action == "politics") { reward = like_reward; }
    else if (context.at("time_of_day") == "afternoon" && action == "music") { reward = like_reward; }
  }
  else if (context.at("user") == "Anna")
  {
    if (context.at("time_of_day") == "morning" && action == "sports") { reward = like_reward; }
    else if (context.at("time_of_day") == "afternoon" && action == "politics") { reward = like_reward; }
  }

  if (swap_reward) { return scale_reward * ((reward == like_reward) ? dislike_reward : like_reward); }
  return reward;
}

// todo: skip text format and create vw example directly
std::vector<std::string> cb_sim::to_vw_example_format(
    const std::map<std::string, std::string>& context, const std::string& chosen_action, float cost, float prob)
{
  std::vector<std::string> multi_ex_str;
  multi_ex_str.push_back(
      fmt::format("shared |{} user={} time_of_day={}", user_ns, context.at("user"), context.at("time_of_day")));
  for (const auto& action : actions)
  {
    std::ostringstream ex;
    if (action == chosen_action) { ex << fmt::format("0:{}:{} ", cost, prob); }
    ex << fmt::format("|{} article={}", action_ns, action);
    multi_ex_str.push_back(ex.str());
  }
  return multi_ex_str;
}

std::pair<int, float> cb_sim::sample_custom_pmf(std::vector<float>& pmf)
{
  float total = std::accumulate(pmf.begin(), pmf.end(), 0.f);
  float scale = 1.f / total;
  for (float& val : pmf) { val *= scale; }
  float draw = random_state.get_and_update_random();
  float sum_prob = 0.f;
  for (size_t index = 0; index < pmf.size(); ++index)
  {
    sum_prob += pmf[index];
    if (sum_prob > draw) { return std::make_pair(index, pmf[index]); }
  }
  THROW("Error: No prob selected");
}

VW::multi_ex cb_sim::build_vw_examples(VW::workspace* vw, std::map<std::string, std::string>& context)
{
  std::vector<std::string> multi_ex_str = to_vw_example_format(context, "");
  VW::multi_ex examples;
  for (const std::string& ex : multi_ex_str) { examples.push_back(VW::read_example(*vw, ex)); }

  return examples;
}

VW::action_scores cb_sim::get_action_scores(VW::workspace* vw, VW::multi_ex examples)
{
  vw->predict(examples);
  auto& scores = examples[0]->pred.a_s;
  vw->finish_example(examples);

  return scores;
}

std::pair<std::string, float> cb_sim::get_action(VW::action_scores scores)
{
  std::vector<float> ordered_scores(scores.size());
  for (auto const& action_score : scores) { ordered_scores[action_score.action] = action_score.score; }

  std::pair<int, float> pmf_sample = sample_custom_pmf(ordered_scores);
  return std::make_pair(actions[pmf_sample.first], pmf_sample.second);
}

const std::string& cb_sim::choose_user()
{
  int rand_ind = static_cast<int>(random_state.get_and_update_random() * users.size());
  return users[rand_ind];
}

const std::string& cb_sim::choose_time_of_day()
{
  int rand_ind = static_cast<int>(random_state.get_and_update_random() * times_of_day.size());
  return times_of_day[rand_ind];
}

void cb_sim::call_if_exists(VW::workspace& vw, VW::multi_ex& ex, const callback_map& callbacks, const size_t event)
{
  auto iter = callbacks.find(event);
  if (iter != callbacks.end())
  {
    callback_count++;
    if (!iter->second(*this, vw, ex)) { THROW("Simulator callback returned false"); }
  }
}

std::vector<float> cb_sim::run_simulation_hook(VW::workspace* vw, size_t num_iterations, callback_map& callbacks,
    bool do_learn, size_t shift, bool add_noise, uint64_t num_useless_features, const std::vector<uint64_t>& swap_after,
    float scale_reward)
{
  // check if there's a callback for the first possible element,
  // in this case most likely 0th event
  // i.e. right before sending any event to VW
  VW::multi_ex dummy;
  call_if_exists(*vw, dummy, callbacks, shift - 1);

  bool swap_reward = false;
  auto swap_after_iter = swap_after.begin();

  size_t update_count = shift;

  for (size_t i = shift; i < shift + num_iterations; ++i)
  {
    if (swap_after_iter != swap_after.end())
    {
      if (i > *swap_after_iter)
      {
        ++swap_after_iter;
        swap_reward = !swap_reward;
      }
    }
    // 1. In each simulation choose a user
    auto user = choose_user();

    // 2. Choose time of day for a given user
    auto time_of_day = choose_time_of_day();

    // 3. Pass context to vw to get an action
    std::map<std::string, std::string> context{{"user", user}, {"time_of_day", time_of_day}};
    // Add useless features if specified
    for (uint64_t j = 0; j < num_useless_features; ++j)
    {
      context.insert(std::pair<std::string, std::string>(std::to_string(j), std::to_string(j)));
    }
    auto examples = build_vw_examples(vw, context);
    auto a_s = get_action_scores(vw, examples);
    auto action_prob = get_action(a_s);
    auto chosen_action = action_prob.first;
    auto prob = action_prob.second;

    // 4. Get cost of the action we chose
    // Check for reward swap
    float cost = get_reaction(context, chosen_action, add_noise, swap_reward, scale_reward);

    // cost of FLT_MAX signals that we should skip anything to do with this example, like it does not exist (i.e. we
    // have no feedback for it)
    if (cost == FLT_MAX)
    {
      // keep the ctr up to date (no updates since we are skipping)
      ctr.push_back(-1 * cost_sum / static_cast<float>(update_count));
      continue;
    }

    update_count++;

    cost_sum += cost;

    if (do_learn)
    {
      // 5. Inform VW of what happened so we can learn from it
      std::vector<std::string> multi_ex_str = to_vw_example_format(context, chosen_action, cost, prob);
      VW::multi_ex examples;
      for (const std::string& ex : multi_ex_str) { examples.push_back(VW::read_example(*vw, ex)); }

      // 6. Learn
      vw->learn(examples);

      call_if_exists(*vw, examples, callbacks, i);

      // 7. Let VW know you're done with these objects
      vw->finish_example(examples);
    }

    // We negate this so that on the plot instead of minimizing cost, we are maximizing reward
    ctr.push_back(-1 * cost_sum / static_cast<float>(update_count));
  }

  // avoid silently failing: ensure that all callbacks
  // got called and then cleanup
  assert(callbacks.size() == callback_count);
  callbacks.clear();
  callback_count = 0;
  assert(callbacks.size() == callback_count);

  return ctr;
}

std::vector<float> cb_sim::run_simulation(
    VW::workspace* vw, size_t num_iterations, bool do_learn, size_t shift, const std::vector<uint64_t>& swap_after)
{
  callback_map callbacks;
  return cb_sim::run_simulation_hook(vw, num_iterations, callbacks, do_learn, shift, false, 0, swap_after);
}

std::vector<float> _test_helper(const std::vector<std::string>& vw_arg, size_t num_iterations, int seed)
{
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg));
  simulator::cb_sim sim(seed);
  auto ctr = sim.run_simulation(vw.get(), num_iterations);
  vw->finish();
  return ctr;
}

std::vector<float> _test_helper_save_load(const std::vector<std::string>& vw_arg, size_t num_iterations, int seed,
    const std::vector<uint64_t>& swap_after, const size_t split)
{
  assert(num_iterations > split);
  size_t before_save = num_iterations - split;

  auto first_vw = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg));
  simulator::cb_sim sim(seed);
  // first chunk
  auto ctr = sim.run_simulation(first_vw.get(), before_save, true, 1, swap_after);

  auto backing_vector = std::make_shared<std::vector<char>>();
  {
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*first_vw, io_writer);
    io_writer.flush();
  }

  first_vw->finish();
  first_vw.reset();

  // reload in another instance
  auto load_options = vw_arg;
  load_options.emplace_back("--quiet");
  auto other_vw = VW::initialize(VW::make_unique<VW::config::options_cli>(load_options),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  // continue
  ctr = sim.run_simulation(other_vw.get(), split, true, before_save + 1, swap_after);
  other_vw->finish();
  return ctr;
}

std::vector<float> _test_helper_hook(const std::vector<std::string>& vw_arg, callback_map& hooks, size_t num_iterations,
    int seed, const std::vector<uint64_t>& swap_after, float scale_reward)
{
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(vw_arg));
  simulator::cb_sim sim(seed);
  auto ctr = sim.run_simulation_hook(vw.get(), num_iterations, hooks, true, 1, false, 0, swap_after, scale_reward);
  vw->finish();
  return ctr;
}
}  // namespace simulator
