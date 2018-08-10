#pragma once
#include <string>
#include <unordered_map>
/**
 * @brief Represents a person initiating an interaction
 */
class person {
  public:
    //! Collection type of outcome probability for given action
    using topic_prob = std::unordered_map<std::string, float>; 
    /**
     * @brief Construct a new person
     * 
     * @param id Unique id for a person
     * @param major Person feature (major)
     * @param hobby Person feature (hobby)
     * @param fav_char Person feature (fav_char)
     * @param topicprob Probability of outcome for a given topic
     **/
    person(std::string id, std::string major,
            std::string hobby, std::string fav_char, 
            topic_prob& topicprob);
    ~person();

    //! Get person features as a json string
    std::string get_features(); 
    //! Get the outcome for a topic.  Use probability to randomly assign a outcome
    float get_outcome(const std::string& topic); 
    //! Get the person's id
    std::string id(); 
  private:
    const std::string _id;
    const std::string _major;
    const std::string _hobby;
    const std::string _favorite_character;
    topic_prob _topic_click_probability;
};

