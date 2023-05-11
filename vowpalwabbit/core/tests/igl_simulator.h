#include "simulator.h"

namespace igl_simulator
{
class igl_sim : public simulator::cb_sim
{
  float true_reward_sum = 0.f;
  std::vector<float> ctr;
  const std::map<std::string, std::string> ground_truth_enjoy = {{"Tom", "politics"}, {"Anna", "music"}};

  const std::map<std::string, std::string> ground_truth_hate = {{"Tom", "music"}, {"Anna", "sports"}};

  // assume two users communicate their satisfaction in the same way
  const std::map<std::string, float> enjoy_prob = {
      {"dislike", 0}, {"skip", 0}, {"click", 0.5}, {"like", 0.5}, {"none", 0}};
  const std::map<std::string, float> hate_prob = {
      {"dislike", 0.1}, {"skip", 0.9}, {"click", 0}, {"like", 0}, {"none", 0}};
  const std::map<std::string, float> neutral_prob = {
      {"dislike", 0}, {"skip", 0}, {"click", 0}, {"like", 0}, {"none", 1}};

public:
  igl_sim(uint64_t seed = 0);
  std::vector<float> run_simulation(VW::workspace* igl_vw, size_t shift, size_t num_iterations);
  std::string sample_feedback(const std::map<std::string, float>& probs);
  std::string get_feedback(const std::string& pref, const std::string& chosen_action);
  float true_reward(const std::string& user, const std::string& action);
  std::string to_dsjson_format(const std::map<std::string, std::string>& context, const std::string& chosen_action = "",
      float prob = 0.f, std::string feedback = "", VW::action_scores a_s = {});
};
}  // namespace igl_simulator
