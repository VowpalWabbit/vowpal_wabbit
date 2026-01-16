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
      {"dislike", 0.0f}, {"skip", 0.0f}, {"click", 0.5f}, {"like", 0.5f}, {"none", 0.0f}};
  const std::map<std::string, float> hate_prob = {
      {"dislike", 0.1f}, {"skip", 0.9f}, {"click", 0.0f}, {"like", 0.0f}, {"none", 0.0f}};
  const std::map<std::string, float> neutral_prob = {
      {"dislike", 0.0f}, {"skip", 0.0f}, {"click", 0.0f}, {"like", 0.0f}, {"none", 1.0f}};

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
