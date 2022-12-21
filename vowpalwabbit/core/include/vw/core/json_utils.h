#pragma once

namespace VW
{
namespace details
{
// Decision Service JSON header information - required to construct final label
class decision_service_interaction
{
public:
  std::string event_id;
  std::string timestamp;
  std::vector<unsigned> actions;
  std::vector<float> probabilities;
  std::vector<unsigned> baseline_actions;
  float probability_of_drop = 0.f;
  float original_label_cost = 0.f;
  float original_label_cost_first_slot = 0.f;
  bool skip_learn{false};
};
}  // namespace details
}  // namespace VW