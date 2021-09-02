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
  cb_sim(int = 0);
  float get_cost(const std::map<std::string, std::string>&, const std::string&);
  std::vector<std::string> to_vw_example_format(
      const std::map<std::string, std::string>&, const std::string& = "", float = 0.f, float = 0.f);
  std::pair<int, float> sample_custom_pmf(std::vector<float>& pmf);
  std::pair<std::string, float> get_action(vw* vw, const std::map<std::string, std::string>&);
  const std::string& choose_user();
  const std::string& choose_time_of_day();
  std::vector<float> run_simulation(vw*, int, bool = true, int = 1);
};
}  // namespace simulator