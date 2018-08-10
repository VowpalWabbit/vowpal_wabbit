#include "simulation_stats.h"
#include "str_util.h"

simulation_stats::simulation_stats() = default;

simulation_stats::~simulation_stats() = default;

void simulation_stats::record(const std::string& id, size_t chosen_action, const float outcome) {
  auto& action_stats = _action_stats[std::make_pair(id, chosen_action)];
  if ( outcome > 0.00001f )
    ++action_stats.first;
  ++action_stats.second;
  auto& person_count = _person_stats[id];
  ++person_count;
  ++_total_events;
}

namespace u = reinforcement_learning::utility;
std::string simulation_stats::get_stats(const std::string& id, size_t chosen_action) {
  auto& action_stats = _action_stats[std::make_pair(id, chosen_action)];
  auto& person_count = _person_stats[id];

  return u::concat("wins, ", action_stats.first, ", out_of,", action_stats.second, ", total,", person_count);
}

int simulation_stats::count() const {
  return _total_events;
}
