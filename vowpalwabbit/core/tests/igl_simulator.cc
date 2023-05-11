
#include "igl_simulator.h"

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <fmt/format.h>

namespace igl_simulator
{
std::string igl_sim::to_dsjson_format(const std::map<std::string, std::string>& context,
    const std::string& chosen_action, float prob, std::string feedback, VW::action_scores a_s)
{
  std::ostringstream ex;
  std::ostringstream multi;
  std::ostringstream a;
  std::ostringstream p;
  size_t chosen_index = 0;
  bool definitely_bad = feedback == "dislike";

  for (size_t i = 0; i < a_s.size(); i++)
  {
    a << a_s[i].action;
    p << a_s[i].score;

    if (i != a_s.size() - 1)
    {
      a << ",";
      p << ",";
    }
  }

  for (size_t i = 0; i < actions.size(); i++)
  {
    if (std::strcmp(actions[i].c_str(), chosen_action.c_str()) == 0) { chosen_index = i; }
    multi << fmt::format("{{\"Action\": {{\"action={}\": 1}}}}", actions[i]);

    if (i != actions.size() - 1) { multi << ","; }
  }

  if (!chosen_action.empty())
  {
    // learn example
    ex << fmt::format(
        "{{\"_label_cost\": 0,  \"_label_probability\": {}, \"_label_Action\": {}, \"_labelIndex\": {}, \"o\": "
        "[{{\"v\": {{\"v={}\": 1}}, \"_definitely_bad\": {}}}], \"a\": [{}], \"c\": {{\"User\": {{\"user={}\": 1, "
        "\"time={}\" : 1}}, \"_multi\": [{}]}}, \"p\":[{}]}}",
        prob, chosen_index + 1, chosen_index, feedback, definitely_bad, a.str(), context.at("user"),
        context.at("time_of_day"), multi.str(), p.str());
  }
  else
  {
    // predict example
    ex << fmt::format("{{ \"c\": {{\"User\": {{\"user={}\": 1, \"time={}\" : 1}}, \"_multi\": [{}]}}}}",
        context.at("user"), context.at("time_of_day"), multi.str());
  }

  return ex.str();
}

std::string igl_sim::sample_feedback(const std::map<std::string, float>& probs)
{
  float draw = random_state.get_and_update_random();
  float sum_prob = 0.f;
  for (auto& feedback_prob : probs)
  {
    sum_prob += feedback_prob.second;
    if (draw < sum_prob) { return feedback_prob.first; }
  }
  THROW("Error: No feedback selected");
}

std::string igl_sim::get_feedback(const std::string& user, const std::string& chosen_action)
{
  if (ground_truth_enjoy.at(user) == chosen_action) { return sample_feedback(enjoy_prob); }
  else if (ground_truth_hate.at(user) == chosen_action) { return sample_feedback(hate_prob); }

  return sample_feedback(neutral_prob);
}

igl_sim::igl_sim(uint64_t seed)
    : cb_sim(seed, false, {"politics", "sports", "music", "food", "finance", "health", "camping"})
{
}

float igl_sim::true_reward(const std::string& user, const std::string& action)
{
  return (ground_truth_enjoy.at(user) == action) - (ground_truth_hate.at(user) == action);
}

std::vector<float> igl_sim::run_simulation(VW::workspace* igl_vw, size_t shift, size_t num_iterations)
{
  for (size_t i = shift; i < shift + num_iterations; i++)
  {
    // 1. In each simulation choose a user
    std::string user = choose_user();

    // 2. Choose time of day for a given user
    std::string time_of_day = choose_time_of_day();

    // 3. Pass context to vw to get an action
    std::map<std::string, std::string> context{{"user", user}, {"time_of_day", time_of_day}};
    std::string igl_pred_ex = to_dsjson_format(context);
    auto examples = vwtest::parse_dsjson(*igl_vw, igl_pred_ex);
    VW::setup_examples(*igl_vw, examples);

    auto a_s = get_action_scores(igl_vw, examples);
    auto action_prob = get_action(a_s);
    auto chosen_action = action_prob.first;
    auto prob = action_prob.second;

    // 4. Simulate feedback for chosen action
    std::string feedback = get_feedback(user, chosen_action);
    true_reward_sum += true_reward(user, chosen_action);

    // 5 - IGL learn
    std::string igl_ex_str = to_dsjson_format(context, chosen_action, prob, feedback, a_s);
    examples = vwtest::parse_dsjson(*igl_vw, igl_ex_str);
    VW::setup_examples(*igl_vw, examples);

    igl_vw->learn(examples);
    igl_vw->finish_example(examples);

    // 6 - calculate ctr
    ctr.push_back(true_reward_sum / static_cast<float>(i));
  }

  return ctr;
}
}  // namespace igl_simulator