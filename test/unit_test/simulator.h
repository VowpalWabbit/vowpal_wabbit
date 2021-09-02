#pragma once

#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <functional>
#include <map>
#include <vector>
#include <string>

struct vw;

namespace simulator
{
// maps an int: # learned examples
// with a function to 'test' at that point in time in the simulator
using callback_map = typename std::map<int, std::function<bool(vw*)>>;

class cb_sim
{
  const float USER_LIKED_ARTICLE = -1.f;
  const float USER_DISLIKED_ARTICLE = 0.f;
  const std::vector<std::string> users;
  const std::vector<std::string> times_of_day;
  const std::vector<std::string> actions;
  int seed;
  float cost_sum = 0.f;
  std::vector<float> ctr;

public:
  cb_sim(int = 0);
  float get_cost(const std::map<std::string, std::string>&, const std::string&);
  std::vector<std::string> to_vw_example_format(
      const std::map<std::string, std::string>&, const std::string&, float = 0.f, float = 0.f);
  std::pair<int, float> sample_custom_pmf(std::vector<float>& pmf);
  std::pair<std::string, float> get_action(vw* vw, const std::map<std::string, std::string>&);
  const std::string& choose_user();
  const std::string& choose_time_of_day();
  std::vector<float> run_simulation(vw*, int, bool = true, int = 1);
  std::vector<float> run_simulation_hook(vw*, int, callback_map&, bool = true, int = 1);
};

std::vector<float> _test_helper(const std::string&, int = 3000, int = 10);
std::vector<float> _test_helper_save_load(const std::string&, int = 3000, int = 10);
std::vector<float> _test_helper_hook(const std::string&, callback_map&, int = 3000, int = 10);
}  // namespace simulator