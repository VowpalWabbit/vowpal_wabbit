// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/random.h"
#include "vw/core/multi_ex.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace VW
{
class workspace;
}

namespace simulator
{
class cb_sim;
// maps an int: # learned examples
// with a function to 'test' at that point in time in the simulator
using callback_map = typename std::map<size_t, std::function<bool(cb_sim&, VW::workspace&, VW::multi_ex&)>>;

class cb_sim
{
  const float USER_LIKED_ARTICLE = -1.f;
  const float USER_DISLIKED_ARTICLE = 0.f;
  const std::vector<std::string> users;
  const std::vector<std::string> times_of_day;
  const std::vector<std::string> actions;
  VW::rand_state random_state;
  float cost_sum = 0.f;
  std::vector<float> ctr;
  size_t callback_count;

public:
  std::string user_ns;
  std::string action_ns;

  cb_sim(uint64_t seed = 0, bool use_default_ns = false);
  float get_reaction(const std::map<std::string, std::string>& context, const std::string& action,
      bool add_noise = false, bool swap_reward = false, float scale_reward = 1.f);
  std::vector<std::string> to_vw_example_format(const std::map<std::string, std::string>& context,
      const std::string& chosen_action, float cost = 0.f, float prob = 0.f);
  std::pair<int, float> sample_custom_pmf(std::vector<float>& pmf);
  std::pair<std::string, float> get_action(VW::workspace* vw, const std::map<std::string, std::string>& context);
  const std::string& choose_user();
  const std::string& choose_time_of_day();
  std::vector<float> run_simulation(VW::workspace* vw, size_t num_iterations, bool do_learn = true, size_t shift = 1,
      const std::vector<uint64_t>& swap_after = std::vector<uint64_t>());
  std::vector<float> run_simulation_hook(VW::workspace* vw, size_t num_iterations, callback_map& callbacks,
      bool do_learn = true, size_t shift = 1, bool add_noise = false, uint64_t num_useless_features = 0,
      const std::vector<uint64_t>& swap_after = std::vector<uint64_t>(), float scale_reward = 1.f);

private:
  void call_if_exists(VW::workspace& vw, VW::multi_ex& ex, const callback_map& callbacks, const size_t event);
};

std::vector<float> _test_helper(const std::vector<std::string>& vw_arg, size_t num_iterations = 3000, int seed = 10);
std::vector<float> _test_helper_save_load(const std::vector<std::string>& vw_arg, size_t num_iterations = 3000,
    int seed = 10, const std::vector<uint64_t>& swap_after = std::vector<uint64_t>(), const size_t split = 1500);
std::vector<float> _test_helper_hook(const std::vector<std::string>& vw_arg, callback_map& hooks,
    size_t num_iterations = 3000, int seed = 10, const std::vector<uint64_t>& swap_after = std::vector<uint64_t>(),
    float scale_reward = 1.f);
}  // namespace simulator
