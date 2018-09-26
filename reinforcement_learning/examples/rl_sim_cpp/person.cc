#include "person.h"
#include <utility>
#include <sstream>

person::person(std::string id, std::string major,
               std::string hobby, std::string fav_char, 
               topic_prob& p) :
  _id(std::move(id)),_major{std::move(major)},_hobby{std::move(hobby)},
  _favorite_character{std::move(fav_char)},_topic_click_probability{p}
{}

person::~person() = default;

std::string person::get_features() const {
  std::ostringstream oss;
  oss << R"("GUser":{)";
  oss << R"("id":")" << _id << R"(",)";
  oss << R"("major":")" << _major << R"(",)";
  oss << R"("hobby":")" << _hobby << R"(",)";
  oss << R"("favorite_character":")" << _favorite_character;
  oss << R"("})";
  return oss.str();
}

float person::get_outcome(const std::string& chosen_action) {
  int const draw_uniform = rand() % 10000;
  float const norm_draw_val = static_cast<float>(draw_uniform) / 10000.0f;
  float const click_prob = _topic_click_probability[chosen_action];
  if ( norm_draw_val <= click_prob )
    return 1.0f;
  else
    return 0.0f;
}

std::string person::id() const {
  return _id;
}
