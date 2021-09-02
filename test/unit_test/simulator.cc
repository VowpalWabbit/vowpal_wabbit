#include "simulator.h"

namespace simulator
{
cb_sim::cb_sim(int seed) : seed(seed) { srand(seed); }

float cb_sim::get_cost(const std::map<std::string, std::string>& context, const std::string& action)
{
  if (context.at("user") == "Tom")
  {
    if (context.at("time_of_day") == "morning" && action == "politics") { return USER_LIKED_ARTICLE; }
    else if (context.at("time_of_day") == "afternoon" && action == "music")
    {
      return USER_LIKED_ARTICLE;
    }
  }
  else if (context.at("user") == "Anna")
  {
    if (context.at("time_of_day") == "morning" && action == "sports") { return USER_LIKED_ARTICLE; }
    else if (context.at("time_of_day") == "afternoon" && action == "politics")
    {
      return USER_LIKED_ARTICLE;
    }
  }
  return USER_DISLIKED_ARTICLE;
}
// todo: skip text format and create vw example directly
std::vector<std::string> cb_sim::to_vw_example_format(
    const std::map<std::string, std::string>& context, const std::string& chosen_action, float cost, float prob)
{
  std::vector<std::string> multi_ex_str;
  multi_ex_str.push_back(
      fmt::format("shared |User user={} time_of_day={}", context.at("user"), context.at("time_of_day")));
  for (const auto& action : actions)
  {
    std::ostringstream ex;
    if (action == chosen_action) { ex << fmt::format("0:{}:{} ", cost, prob); }
    ex << fmt::format("|Action article={}", action);
    multi_ex_str.push_back(ex.str());
  }
  return multi_ex_str;
}

std::pair<int, float> cb_sim::sample_custom_pmf(std::vector<float>& pmf)
{
  float total = std::accumulate(pmf.begin(), pmf.end(), 0.f);
  float scale = 1.f / total;
  for (float& val : pmf) { val *= scale; }
  float draw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float sum_prob = 0.f;
  for (int index = 0; index < pmf.size(); ++index)
  {
    sum_prob += pmf[index];
    if (sum_prob > draw) { return std::make_pair(index, pmf[index]); }
  }
  THROW("Error: No prob selected");
}

std::pair<std::string, float> cb_sim::get_action(vw* vw, const std::map<std::string, std::string>& context)
{
  std::vector<std::string> multi_ex_str = to_vw_example_format(context);
  multi_ex examples;
  for (const std::string& ex : multi_ex_str) { examples.push_back(VW::read_example(*vw, ex)); }
  vw->predict(examples);

  std::vector<float> pmf;
  auto const& scores = examples[0]->pred.a_s;
  std::vector<float> ordered_scores(scores.size());
  for (auto const& action_score : scores) { ordered_scores[action_score.action] = action_score.score; }
  for (auto action_score : ordered_scores) { pmf.push_back(action_score); }
  vw->finish_example(examples);

  std::pair<int, float> pmf_sample = sample_custom_pmf(pmf);
  return std::make_pair(actions[pmf_sample.first], pmf_sample.second);
}

const std::string& cb_sim::choose_user()
{
  int rand_ind = rand() % users.size();
  return users[rand_ind];
}

const std::string& cb_sim::choose_time_of_day()
{
  int rand_ind = rand() % times_of_day.size();
  return times_of_day[rand_ind];
}

std::vector<float> cb_sim::run_simulation(vw* vw, int num_iterations, bool do_learn, int shift)
{
  for (int i = shift; i < shift + num_iterations; ++i)
  {
    // 1. In each simulation choose a user
    auto user = choose_user();

    // 2. Choose time of day for a given user
    auto time_of_day = choose_time_of_day();

    // 3. Pass context to vw to get an action
    const std::map<std::string, std::string> context{{"user", user}, {"time_of_day", time_of_day}};
    auto action_prob = get_action(vw, context);
    auto chosen_action = action_prob.first;
    auto prob = action_prob.second;

    // 4. Get cost of the action we chose
    float cost = get_cost(context, chosen_action);
    cost_sum += cost;

    if (do_learn)
    {
      // 5. Inform VW of what happened so we can learn from it
      std::vector<std::string> multi_ex_str = to_vw_example_format(context, chosen_action, cost, prob);
      multi_ex examples;
      for (const std::string& ex : multi_ex_str) { examples.push_back(VW::read_example(*vw, ex)); }

      // 6. Learn
      vw->learn(examples);

      // 7. Let VW know you're done with these objects
      vw->finish_example(examples);
    }

    // We negate this so that on the plot instead of minimizing cost, we are maximizing reward
    ctr.push_back(-1 * cost_sum / static_cast<float>(i));
  }
  return ctr;
}
std::vector<float> cb_sim::run_simulation_hook(
    vw* vw, int num_iterations, callback_map& callbacks, bool do_learn, int shift)
{
  auto res = cb_sim::run_simulation(vw, num_iterations, do_learn, shift);

  if (!callbacks.empty())
  {
    BOOST_TEST_MESSAGE("executing callbacks...");
    for (auto it = callbacks.begin(); it != callbacks.end(); it++) { it->second(vw); }
  }

  return res;
}

std::vector<float> _test_helper(const std::string& vw_arg, int num_iterations, int seed)
{
  auto vw = VW::initialize(vw_arg);
  simulator::cb_sim sim(seed);
  auto ctr = sim.run_simulation(vw, num_iterations);
  VW::finish(*vw);
  return ctr;
}

std::vector<float> _test_helper_save_load(const std::string& vw_arg, int num_iterations, int seed)
{
  int split = 1500;
  int before_save = num_iterations - split;

  auto first_vw = VW::initialize(vw_arg);
  simulator::cb_sim sim(seed);
  // first chunk
  auto ctr = sim.run_simulation(first_vw, before_save);
  // save
  std::string model_file = "test_save_load.vw";
  VW::save_predictor(*first_vw, model_file);
  VW::finish(*first_vw);
  // reload in another instance
  auto other_vw = VW::initialize("--quiet -i " + model_file);
  // continue
  ctr = sim.run_simulation(other_vw, split, true, before_save + 1);
  VW::finish(*other_vw);
  return ctr;
}

std::vector<float> _test_helper_hook(const std::string& vw_arg, callback_map& hooks, int num_iterations, int seed)
{
  BOOST_CHECK(true);
  auto* vw = VW::initialize(vw_arg);
  simulator::cb_sim sim(seed);
  auto ctr = sim.run_simulation_hook(vw, num_iterations, hooks);
  VW::finish(*vw);
  return ctr;
}
}  // namespace simulator