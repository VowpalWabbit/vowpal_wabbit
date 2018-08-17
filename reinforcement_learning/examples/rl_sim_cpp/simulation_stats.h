#pragma once
#include <map>
class simulation_stats {
public:
  simulation_stats();
  ~simulation_stats();
  void record(const std::string& id, size_t chosen_action, const float outcome);
  std::string get_stats(const std::string& id, size_t chosen_action);
  int count() const;

private:
  std::map<std::pair<std::string, int>, std::pair<int, int>> _action_stats;
  std::map<std::string, int> _person_stats;
  int _total_events = 0;
};

