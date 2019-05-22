#include <iostream>

#include <vector>
#include <random>
#include <string>
#include <sstream>
#include <map>

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
    std::vector<std::string>& slot_features, std::vector<std::string>& tags, std::vector<std::tuple<size_t, float, float>> labels = {})
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
    ss << " " << tags[i] << "|Slot " << slot_features[i];
    ret_val.push_back(ss.str());
    ss.str(std::string());
  }

  return ret_val;
}

void print_click_shows(size_t num_iter, std::vector<std::map<std::vector<size_t>, std::tuple<std::vector<size_t>, size_t>>>& clicks_impressions)
{
  std::cout << "num iterations: " << num_iter << "\n";
  std::cout << "user\tactions\tclicks          shown\tctr\n";
  for (auto user_index = 0; user_index < clicks_impressions.size(); user_index++)
  {
    std::cout << "--\n";

    for (auto& kv : clicks_impressions[user_index])
    {
      std::cout << user_index << "\t";

      // actions
      for (auto num : kv.first)
      {
        std::cout << num;
      }
      std::cout << "\t";

      // clicks
      std::stringstream ss;
      for (auto num : std::get<0>(kv.second))
      {
        ss << num << ",";
      }

      std::cout << std::setw(16) << std::left << ss.str();

      // shown
      std::cout << std::get<1>(kv.second) << "\t";

      for (auto num : std::get<0>(kv.second))
      {
        std::cout <<std::setprecision(4) << (float)num / std::get<1>(kv.second) << ",";
      }
      std::cout << "\n";

    }
  }
  std::cout << std::endl;
}

int main()
{
  auto vw = VW::initialize("--ccb_explore_adf --epsilon 0.2 --learning_rate 0.001 --quiet --first_only");

  auto const NUM_USERS = 3;
  auto const NUM_ACTIONS = 4;
  auto const NUM_SLOTS = 2;
  auto const NUM_ITER = 1000000;

  std::vector<std::string> user_features = {"a", "b", "c"};
  std::vector<std::string> action_features = {"d", "e", "f", "g"};
  std::vector<std::string> slot_features = {"h", "i"};

  std::vector<std::map<std::vector<size_t>, std::tuple<std::vector<size_t>, size_t>>> clicks_impressions = {
    {
      {{0, 1}, {{0,0}, 0}},
      {{0, 2}, {{0,0}, 0}},
      {{0, 3}, {{0,0}, 0}},
      {{1, 0}, {{0,0}, 0}},
      {{1, 2}, {{0,0}, 0}},
      {{1, 3}, {{0,0}, 0}},
      {{2, 0}, {{0,0}, 0}},
      {{2, 1}, {{0,0}, 0}},
      {{2, 3}, {{0,0}, 0}},
      {{3, 0}, {{0,0}, 0}},
      {{3, 1}, {{0,0}, 0}},
      {{3, 2}, {{0,0}, 0}}
    },
    {
      {{0, 1}, {{0,0}, 0}},
      {{0, 2}, {{0,0}, 0}},
      {{0, 3}, {{0,0}, 0}},
      {{1, 0}, {{0,0}, 0}},
      {{1, 2}, {{0,0}, 0}},
      {{1, 3}, {{0,0}, 0}},
      {{2, 0}, {{0,0}, 0}},
      {{2, 1}, {{0,0}, 0}},
      {{2, 3}, {{0,0}, 0}},
      {{3, 0}, {{0,0}, 0}},
      {{3, 1}, {{0,0}, 0}},
      {{3, 2}, {{0,0}, 0}}
    },
    {
      {{0, 1}, {{0,0}, 0}},
      {{0, 2}, {{0,0}, 0}},
      {{0, 3}, {{0,0}, 0}},
      {{1, 0}, {{0,0}, 0}},
      {{1, 2}, {{0,0}, 0}},
      {{1, 3}, {{0,0}, 0}},
      {{2, 0}, {{0,0}, 0}},
      {{2, 1}, {{0,0}, 0}},
      {{2, 3}, {{0,0}, 0}},
      {{3, 0}, {{0,0}, 0}},
      {{3, 1}, {{0,0}, 0}},
      {{3, 2}, {{0,0}, 0}}
    }
  };


  std::vector<std::map<std::vector<size_t>,std::vector<float>>> user_slot_action_probabilities =
  {
    {
      {{0, 1}, {0.01f, 0.3f}},
      {{0, 2}, {0.01f, 0.5f}},
      {{0, 3}, {0.01f, 0.2f}},
      {{1, 0}, {0.2f, 0.01f}},
      {{1, 2}, {0.2f, 0.5f}},
      {{1, 3}, {0.2f, 0.2f}},
      {{2, 0}, {0.3f, 0.01f}},
      {{2, 1}, {0.3f, 0.3f}},
      {{2, 3}, {0.3f, 0.2f}},
      {{3, 0}, {0.1f, 0.01f}},
      {{3, 1}, {0.1f, 0.3f}},
      {{3, 2}, {0.1f, 0.5f}}
    },
    {
      {{0, 1}, {0.01f, 0.1f}},
      {{0, 2}, {0.01f, 0.3f}},
      {{0, 3}, {0.01f, 0.2f}},
      {{1, 0}, {0.2f, 0.01f}},
      {{1, 2}, {0.2f, 0.3f}},
      {{1, 3}, {0.2f, 0.2f}},
      {{2, 0}, {0.3f, 0.01f}},
      {{2, 1}, {0.3f, 0.1f}},
      {{2, 3}, {0.3f, 0.2f}},
      {{3, 0}, {0.1f, 0.01f}},
      {{3, 1}, {0.1f, 0.1f}},
      {{3, 2}, {0.1f, 0.3f}}
    },
    {
      {{0, 1}, {0.01f, 0.5f}},
      {{0, 2}, {0.01f, 0.1f}},
      {{0, 3}, {0.01f, 0.4f}},
      {{1, 0}, {0.2f, 0.01f}},
      {{1, 2}, {0.2f, 0.1f}},
      {{1, 3}, {0.2f, 0.4f}},
      {{2, 0}, {0.1f, 0.01f}},
      {{2, 1}, {0.1f, 0.5f}},
      {{2, 3}, {0.1f, 0.4f}},
      {{3, 0}, {0.1f, 0.01f}},
      {{3, 1}, {0.1f, 0.5f}},
      {{3, 2}, {0.1f, 0.1f}}
    }
  };

  std::default_random_engine rd{0};
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> user_distribution(0, NUM_USERS - 1);
  std::uniform_real_distribution<float> click_distribution(0.0f, 1.0f);
  uint64_t merand_seed = 0;



  for (int i = 1; i <= NUM_ITER; i++)
  {
    volatile auto chosen_user = user_distribution(eng);
    std::vector<std::string> tags;
    for (int t = 0; t < NUM_SLOTS; t++)
    {
      tags.push_back("seed="+std::to_string(merand48(merand_seed)));
    }
    auto ex_str = build_example_string_ccb(user_features[chosen_user], action_features, slot_features, tags);

    multi_ex ex_col;
    for (auto str : ex_str)
    {
      ex_col.push_back(VW::read_example(*vw, str));
    }

    vw->predict(ex_col);

    std::vector<std::tuple<size_t, float, float>> outcomes;
    auto decision_scores = ex_col[0]->pred.decision_scores;

    std::vector<size_t> actions_taken;
    for (auto s : decision_scores)
    {
      actions_taken.push_back(s[0].action);
    };

    std::get<1>(clicks_impressions[chosen_user][actions_taken])++;
    for (auto slot_id = 0; slot_id < decision_scores.size(); slot_id++)
    {
      auto& slot = decision_scores[slot_id];
      auto action_id = slot[0].action;
      auto prob_chosen = slot[0].score;
      auto prob_to_click = user_slot_action_probabilities[chosen_user][actions_taken][slot_id];

      if (click_distribution(eng) < prob_to_click)
      {
        std::get<0>(clicks_impressions[chosen_user][actions_taken])[slot_id]++;
        outcomes.push_back({action_id, -1.f, prob_chosen});
      }
      else
      {
        outcomes.push_back({action_id, 0.f, prob_chosen});
      }
    }
    as_multiline(vw->l)->finish_example(*vw, ex_col);

    auto learn_ex = build_example_string_ccb(user_features[chosen_user], action_features, slot_features, tags, outcomes);
    multi_ex learn_ex_col;
    for (auto str : learn_ex)
    {
      learn_ex_col.push_back(VW::read_example(*vw, str));
    }
    vw->learn(learn_ex_col);
    as_multiline(vw->l)->finish_example(*vw, learn_ex_col);

    if (i % 5000 == 0)
    {
      // Clear terminal
      std::cout << "\033[2J" << std::endl;
      print_click_shows(i, clicks_impressions);
    }
  }

  std::cout << "\033[2J" << std::endl;
  print_click_shows(NUM_ITER, clicks_impressions);
}
