#include "vw.h"
#include "test_common.h"
#include <numeric>
#include <fmt/format.h>

namespace simulator
{
class cb_sim
{
  float USER_LIKED_ARTICLE = -1.f;
  float USER_DISLIKED_ARTICLE = 0.f;
  std::vector<std::string> users{"Tom", "Anna"};
  std::vector<std::string> times_of_day{"morning", "afternoon"};
  std::vector<std::string> actions{"politics", "sports", "music", "food", "finance", "health", "camping"};
  int seed;
  float cost_sum = 0.f;
  std::vector<float> ctr;

public:
  cb_sim(int seed);
  float get_cost(std::map<std::string, std::string> context, std::string action);
  std::vector<std::string> to_vw_example_format(
      std::map<std::string, std::string> context, std::string chosen_action = "", float cost = 0.f, float prob = 0.f);
  std::pair<int, float> sample_custom_pmf(std::vector<float> pmf);
  std::pair<std::string, float> get_action(vw* vw, std::map<std::string, std::string> context);
  std::string choose_user();
  std::string choose_time_of_day();
  std::vector<float> run_simulation(vw* vw, int num_iterations, bool do_learn = true, int shift = 1);
};
}  // namespace simulator