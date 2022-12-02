#include <sstream>
#include <string>
#include <vector>

inline std::string get_x_numerical_fts(int feature_size)
{
  std::stringstream ss;
  ss << "1:1:0.5 |";
  for (size_t i = 0; i < feature_size; i++) { ss << " " << std::to_string(i) + ":4.36352"; }
  std::string s = ss.str();
  return s;
};

inline std::string get_x_string_fts(int feature_size)
{
  std::stringstream ss;
  ss << "1:1:0.5 | ";
  for (size_t i = 0; i < feature_size; i++) { ss << "bigfeaturename" + std::to_string(i) + ":10 "; }
  std::string s = ss.str();
  return s;
};

inline std::string get_x_string_fts_no_label(int feature_size, size_t action_index = 0)
{
  std::stringstream ss;
  ss << " | ";
  for (size_t j = 0; j < feature_size; j++) { ss << std::to_string(action_index) + "_" + std::to_string(j) << +" "; }
  ss << std::endl;

  return ss.str();
};

inline std::string get_x_string_fts_multi_ex(
    int feature_size, size_t actions, bool shared, bool label, size_t start_index = 0)
{
  size_t action_start = 0;
  std::stringstream ss;
  if (shared) { ss << "shared | s_1 s_2 s_3 s_4" << std::endl; }
  if (label)
  {
    ss << "0:1.0:0.5 | ";
    for (size_t j = 0; j < feature_size; j++) { ss << "0_" + std::to_string(j) << +" "; }
    ss << std::endl;
    action_start++;
  }
  for (size_t i = action_start; i < actions; i++)
  {
    ss << " | ";
    for (size_t j = start_index; j < start_index + feature_size; j++)
    {
      ss << std::to_string(i) + "_" + std::to_string(j) << +" ";
    }
    ss << std::endl;
  }
  return ss.str();
};