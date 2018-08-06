#pragma once
#include <map>
class simulation_stats {
public:
  simulation_stats();
  ~simulation_stats();
  void record(const std::string& id, size_t choosen_action, const float reward);
  std::string get_stats(const std::string& id, size_t choosen_action);
  int count() const;

private:
  std::map<std::pair<std::string, int>, std::pair<int, int>> _action_stats;
  std::map<std::string, int> _person_stats;
  int _total_events = 0;
};

