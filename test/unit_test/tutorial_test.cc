#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "vw.h"
#include "test_common.h"
#include <numeric>
#include <fmt/format.h>

class Simulator
{
  float USER_LIKED_ARTICLE = -1.f;
  float USER_DISLIKED_ARTICLE = 0.f;
  std::vector<std::string> users{"Tom", "Anna"};
  std::vector<std::string> times_of_day{"morning", "afternoon"};
  std::vector<std::string> actions{"politics", "sports", "music", "food", "finance", "health", "camping"};
  std::string debug_logfile;
  int seed;
  float cost_sum = 0.f;
  std::vector<float> ctr;

public:
  Simulator(std::string debug_logfile = "", int seed = 0) : debug_logfile(debug_logfile), seed(seed) { srand(seed); }

  float get_cost(std::map<std::string, std::string> context, std::string action)
  {
    if (context["user"] == "Tom")
    {
      if (context["time_of_day"] == "morning" && action == "politics") { return USER_LIKED_ARTICLE; }
      else if (context["time_of_day"] == "afternoon" && action == "music")
      {
        return USER_LIKED_ARTICLE;
      }
    }
    else if (context["user"] == "Anna")
    {
      if (context["time_of_day"] == "morning" && action == "sports") { return USER_LIKED_ARTICLE; }
      else if (context["time_of_day"] == "afternoon" && action == "politics")
      {
        return USER_LIKED_ARTICLE;
      }
    }
    return USER_DISLIKED_ARTICLE;
  }

  std::vector<std::string> to_vw_example_format(
      std::map<std::string, std::string> context, std::string chosen_action = "", float cost = 0.f, float prob = 0.f)
  {
    std::vector<std::string> multi_ex_str;
    multi_ex_str.push_back(fmt::format("shared |User user={} time_of_day={}", context["user"], context["time_of_day"]));
    for (const auto& action : actions)
    {
      std::ostringstream ex;
      if (action == chosen_action) { ex << fmt::format("0:{}:{} ", cost, prob); }
      ex << fmt::format("|Action article={}", action);
      multi_ex_str.push_back(ex.str());
    }
    return multi_ex_str;
  }

  std::pair<int, float> sample_custom_pmf(std::vector<float> pmf)
  {
    float total = std::accumulate(pmf.begin(), pmf.end(), 0.f);
    float scale = 1.f / total;
    for (float& val : pmf) { val *= scale; }
    float draw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float sum_prob = 0.f;
    for (int index = 0; index < pmf.size(); ++index)
    {
      sum_prob += pmf[index];
      if (sum_prob > draw) { return std::pair<int, float>(index, pmf[index]); }
    }
    THROW("Error: No prob selected");
  }

  std::pair<std::string, float> get_action(vw* vw, std::map<std::string, std::string> context)
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

    std::pair<int, float> pmf_sample = sample_custom_pmf(pmf);
    return std::pair<std::string, float>(actions[pmf_sample.first], pmf_sample.second);
  }

  std::string choose_user()
  {
    int rand_ind = rand() % users.size();
    return users[rand_ind];
  }

  std::string choose_time_of_day()
  {
    int rand_ind = rand() % times_of_day.size();
    return times_of_day[rand_ind];
  }

  std::vector<float> run_simulation(vw* vw, int num_iterations, bool do_learn = true, int shift = 1)
  {
    for (int i = shift; i < shift + num_iterations; ++i)
    {
      // 1. In each simulation choose a user
      auto user = choose_user();

      // 2. Choose time of day for a given user
      auto time_of_day = choose_time_of_day();

      // 3. Pass context to vw to get an action
      std::map<std::string, std::string> context{{"user", user}, {"time_of_day", time_of_day}};
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
};

std::vector<float> _test_helper(std::string vw_arg, int num_iterations = 3000, int seed = 10)
{
  auto vw = VW::initialize(vw_arg);
  Simulator sim("", seed);
  VW::finish(*vw);
  return sim.run_simulation(vw, num_iterations);
}

std::vector<float> _test_helper_save_load(std::string vw_arg, int num_iterations = 3000, int seed = 10)
{
  int split = 1500;
  int before_save = num_iterations - split;

  auto first_vw = VW::initialize(vw_arg);
  Simulator sim("", seed);
  // first chunk
  auto ctr = sim.run_simulation(first_vw, before_save);
  // save
  std::string model_file = "test_save_load.vw";
  VW::save_predictor(*first_vw, model_file);
  VW::finish(*first_vw);
  // reload in another instance
  auto other_vw = VW::initialize("--quiet -i test_save_load.vw");
  // continue
  ctr = sim.run_simulation(other_vw, split, true, before_save + 1);
  VW::finish(*other_vw);
  return ctr;
}

BOOST_AUTO_TEST_CASE(cpp_simulator_without_interaction)
{
  auto ctr = _test_helper("--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  BOOST_CHECK(ctr.back() <= 0.49f && ctr.back() >= 0.38f);
}

BOOST_AUTO_TEST_CASE(cpp_simulator_with_interaction)
{
  auto ctr = _test_helper("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5");
  float without_save = ctr.back();
  BOOST_CHECK(without_save >= 0.7f);

  ctr = _test_helper_save_load("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5 --save_resume");
  float with_save = ctr.back();
  BOOST_CHECK(with_save >= 0.7f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}