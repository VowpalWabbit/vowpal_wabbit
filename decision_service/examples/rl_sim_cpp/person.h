#pragma once
#include <string>
#include <unordered_map>

class person {
  public:
    using topic_prob = std::unordered_map<std::string, float>;
    
    person(std::string id, std::string major,
               std::string hobby, std::string fav_char, 
               topic_prob& p);
    ~person();
    std::string get_features();
    float get_reward(const std::string& topic);
    std::string id();
  private:
    const std::string _id;
    const std::string _major;
    const std::string _hobby;
    const std::string _favorite_character;
    topic_prob _topic_click_probability;
};

