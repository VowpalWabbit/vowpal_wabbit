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

std::vector<std::string> build_example_string_ccb(std::string& user_feature, std::vector<std::string>& action_features,
    std::vector<std::string>& slot_features, std::vector<std::tuple<size_t, float, float>> labels = {})
{
  std::vector<std::string> ret_val;
  std::stringstream ss;
  ss << "ccb shared |User " << user_feature;
  ret_val.push_back(ss.str());
  ss.str(std::string());

  for (auto action : action_features)
  {
    ss << "ccb action |Action " << action;
    ret_val.push_back(ss.str());
    ss.str(std::string());
  }

  for (size_t i = 0; i < slot_features.size(); i++)
  {
    ss << "ccb slot ";
    if (labels.size() > i)
    {
      ss << (std::get<0>(labels[i])) << ":" << std::get<1>(labels[i]) << ":" << std::get<2>(labels[i]);
    }
    ss << " |Slot " << slot_features[i];
    ret_val.push_back(ss.str());
    ss.str(std::string());
  }

  return ret_val;
}

//std::vector<std::string> build_example_string_cb(std::string& user_feature, std::vector<std::string>& action_features,
//    std::string& slot_features, std::tuple<size_t, float, float> outcome)
//{
//  std::vector<std::string> ret_val;
//  std::stringstream ss;
//  ss << "ccb shared |User " << user_feature;
//  ret_val.push_back(ss.str());
//  ss.str(std::string());
//
//  for (auto action : action_features)
//  {
//    ss << "ccb action |Action " << action;
//    ret_val.push_back(ss.str());
//    ss.str(std::string());
//  }
//
//  for (size_t i = 0; i < slot_features.size(); i++)
//  {
//    ss << "ccb slot ";
//    if (labels.size() > i)
//    {
//      ss << (std::get<0>(labels[i])) << ":" << std::get<1>(labels[i]) << ":" << std::get<2>(labels[i]);
//    }
//    ss << " |Slot " << slot_features[i];
//    ret_val.push_back(ss.str());
//    ss.str(std::string());
//  }
//
//  return ret_val;
//}

void print_click_shows(size_t num_iter, std::vector<std::vector<std::vector<std::tuple<int, int>>>>& clicks_shows)
{
  std::cout << "num iterations: " << num_iter << "\n";
  std::cout << "user\taction\tdecsn\tclicks\tshown\tctr\n";
  for (auto user_index = 0; user_index < clicks_shows.size(); user_index++)
  {
    auto& actions = clicks_shows[user_index];
    for (auto action_index = 0; action_index < actions.size(); action_index++)
    {
      auto& slots = actions[action_index];
      for (auto slot_index = 0; slot_index < slots.size(); slot_index++)
      {
        auto& click_show = slots[slot_index];
        std::cout << user_index << "\t" << action_index << "\t" << slot_index << "\t" << std::get<0>(click_show)
                  << "\t" << std::get<1>(click_show) << "\t" << (float)std::get<0>(click_show) / std::get<1>(click_show)
                  << "\n";
      }
    }
  }
  std::cout << std::endl;
}

int main()
{
  auto vw = VW::initialize("--ccb_explore_adf --epsilon 0.2 --cubic UAS -l 0.01 --ignore_linear UAS --quiet");

  auto const NUM_USERS = 3;
  auto const NUM_ACTIONS = 4;
  auto const NUM_SLOTS = 2;
  auto const NUM_ITER = 1000000;

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

  //std::random_device rd;
  std::default_random_engine rd{0};
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> user_distribution(0, NUM_USERS - 1);
  std::uniform_real_distribution<float> click_distribution(0.0f, 1.0f);

  for (int i = 0; i < NUM_ITER; i++)
  {
    auto chosen_user = user_distribution(eng);
    auto ex_str = build_example_string_ccb(user_features[chosen_user], action_features, slot_features);

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
      auto& slot = decision_scores[i];
      auto chosen_id = slot[0].action;
      auto prob_chosen = slot[0].score;
      auto prob_to_click = all_users[chosen_user][chosen_id][i];

      std::get<1>(clicks_shows[chosen_user][chosen_id][i])++;
      if (click_distribution(eng) < prob_to_click)
      {
        std::get<0>(clicks_shows[chosen_user][chosen_id][i])++;
        outcomes.push_back({chosen_id, -1.f, prob_chosen});
      }
      else
      {
        outcomes.push_back({chosen_id, 0.f, prob_chosen});
      }
    }
    as_multiline(vw->l)->finish_example(*vw, ex_col);

    auto learn_ex = build_example_string_ccb(user_features[chosen_user], action_features, slot_features, outcomes);
    multi_ex learn_ex_col;
    for (auto str : learn_ex)
    {
      learn_ex_col.push_back(VW::read_example(*vw, str));
    }
    vw->learn(learn_ex_col);
    as_multiline(vw->l)->finish_example(*vw, learn_ex_col);

    if (i % 10000 == 0)
    {
      // Clear terminal
      std::cout << "\033[2J" << std::endl;
      print_click_shows(i, clicks_shows);
    }
  }

  std::cout << "\033[2J" << std::endl;
  print_click_shows(NUM_ITER, clicks_shows);
}
