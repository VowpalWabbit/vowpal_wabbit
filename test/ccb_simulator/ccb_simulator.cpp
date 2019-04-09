#include <iostream>

#include <vector>
#include <random>
#include <string>
#include <sstream>

#include "vw.h"
#include "rand48.h"
#include "conditional_contextual_bandit.h"

struct acp
{
  uint32_t action;
  float cost;
  float probability;
};

std::vector<std::string> build_example_string(std::string& user_feature, std::vector<std::string>& action_features,
    std::vector<std::string>& decision_features, std::vector<std::tuple<size_t, float, float>> labels = {})
{
  std::vector<std::string> ret_val;
  std::stringstream ss;
  ss << "ccb shared | " << user_feature;
  ret_val.push_back(ss.str());
  ss.str(std::string());

  for (auto action : action_features)
  {
    ss << "ccb action | " << action;
    ret_val.push_back(ss.str());
    ss.str(std::string());
  }

  for (size_t i = 0; i < decision_features.size(); i++)
  {
    ss << "ccb decision ";
    if (labels.size() > i)
    {
      ss << std::get<0>(labels[i]) << ":" << std::get<1>(labels[i]) << ":" << std::get<2>(labels[i]);
    }
    ss << " | " << decision_features[i];
    ret_val.push_back(ss.str());
    ss.str(std::string());
  }

  return ret_val;
}

void print_click_shows(size_t num_iter, std::vector<std::vector<std::vector<std::tuple<int, int>>>>& clicks_shows)
{
  std::cout << "num iterations: " << num_iter << "\n";
  std::cout << "user\taction\tdecsn\tclicks\tshown\tctr\n";
  for (auto user_index = 0; user_index < clicks_shows.size(); user_index++)
  {
    auto& actions = clicks_shows[user_index];
    for (auto action_index = 0; action_index < actions.size(); action_index++)
    {
      auto& decisions = actions[action_index];
      for (auto decision_index = 0; decision_index < decisions.size(); decision_index++)
      {
        auto& click_show = decisions[decision_index];
        std::cout << user_index << "\t" << action_index << "\t" << decision_index << "\t" << std::get<0>(click_show)
                  << "\t" << std::get<1>(click_show) << "\t" << (float)std::get<0>(click_show) / std::get<1>(click_show)
                  << "\n";
      }
    }
  }
  std::cout << std::endl;
}

int main()
{
  auto vw = VW::initialize("--ccb_explore_adf --epsilon 0.2");

  auto const NUM_USERS = 3;
  auto const NUM_ACTIONS = 4;
  auto const NUM_SLOTS = 2;

  std::vector<std::string> user_features = {"a", "b", "c"};
  std::vector<std::string> action_features = {"d", "e", "f", "g"};
  std::vector<std::string> slot_features = {"h", "i"};

  std::vector<std::vector<float>> user_1_actions_slots_probs = {{0.4f, 0.2f}, {0.3f, 0.2f}, {0.2f, 0.4f}, {0.1f, 0.3f}};

  std::vector<std::vector<float>> user_2_actions_slots_probs = {{0.2f, 0.1f}, {0.2f, 0.4f}, {0.1f, 0.2f}, {0.2f, 0.1f}};

  std::vector<std::vector<float>> user_3_actions_slots_probs = {{0.3f, 0.1f}, {0.2f, 0.3f}, {0.3f, 0.2f}, {0.1f, 0.1f}};

  std::vector<std::vector<std::vector<float>>> all_users = {
      user_1_actions_slots_probs, user_2_actions_slots_probs, user_3_actions_slots_probs};

  // click, show
  std::vector<std::vector<std::vector<std::tuple<int, int>>>> clicks_shows = {
      {{{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}},
  };

  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> user_distribution(0, NUM_USERS - 1);
  std::uniform_real_distribution<float> click_distribution(0.0f, 1.0f);

  for (int i = 0; i < 10000; i++)
  {
    auto chosen_user = user_distribution(eng);
    auto ex_str = build_example_string(user_features[chosen_user], action_features, slot_features);

    multi_ex ex_col;
    for (auto str : ex_str)
    {
      ex_col.push_back(VW::read_example(*vw, str));
    }

    vw->predict(ex_col);

    std::vector<std::tuple<size_t, float, float>> outcomes;
    auto decision_scores = ex_col[0]->pred.decision_scores;
    for (auto i = 0; i < decision_scores.size(); i++)
    {
      auto& decision = decision_scores[i];
      auto chosen_id = decision[0].action;
      auto prob = all_users[chosen_user][chosen_id][i];

      std::get<1>(clicks_shows[chosen_user][chosen_id][i])++;
      if (click_distribution(eng) < prob)
      {
        std::get<0>(clicks_shows[chosen_user][chosen_id][i])++;
        outcomes.push_back({chosen_id, -1.f, prob});
      }
      else
      {
        outcomes.push_back({chosen_id, 0.f, prob});
      }
    }

    auto learn_ex = build_example_string(user_features[chosen_user], action_features, slot_features, outcomes);
    multi_ex learn_ex_col;
    for (auto str : learn_ex)
    {
      learn_ex_col.push_back(VW::read_example(*vw, str));
    }
    vw->learn(learn_ex_col);

    if (i % 2000 == 0)
    {
      // Clear terminal
      std::cout << "\033[2J" << std::endl;
      print_click_shows(i, clicks_shows);
    }
  }
}
